// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "CyclesRenderer.h"
#include "SceneSynchronizer.h"

#include <ICLGeom/Scene.h>

// Qt defines 'emit' as a macro; ICL defines LOG_LEVEL as a macro.
// Both conflict with Cycles headers.
#undef emit
#undef LOG_LEVEL

#include "device/device.h"
#include "scene/camera.h"
#include "scene/film.h"
#include "scene/integrator.h"
#include "scene/pass.h"
#include "scene/scene.h"
#include "session/buffers.h"
#include "session/output_driver.h"
#include "session/session.h"
#include "util/log.h"
#include "util/path.h"
#include "util/unique_ptr.h"


#include <cmath>
#include <mutex>
#include <vector>

using namespace ccl;

namespace icl::rt {

// ---- ICLOutputDriver: captures Cycles render output into ICL Img8u ----

class ICLOutputDriver : public OutputDriver {
public:
  void write_render_tile(const Tile &tile) override {
    captureTile(tile);
  }

  // Don't capture intermediate tiles — only write_render_tile (above) fires
  // when the target sample count is reached, giving a clean image per step.

  const core::Img8u &getImage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_image;
  }

  int getUpdateCount() const { return m_updateCount; }
  void resetUpdateCount() { m_updateCount = 0; }

private:
  void captureTile(const Tile &tile) {
    if (!(tile.size == tile.full_size))
      return;

    const int w = tile.size.x;
    const int h = tile.size.y;

    std::vector<float> pixels(w * h * 4);
    if (!tile.get_pass_pixels("combined", 4, pixels.data()))
      return;

    m_updateCount++;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_image.setSize(utils::Size(w, h));
    m_image.setChannels(3);

    icl8u *r = m_image.begin(0);
    icl8u *g = m_image.begin(1);
    icl8u *b = m_image.begin(2);

    // Cycles outputs pixels bottom-up; convert to top-down for ICL image.
    for (int y = 0; y < h; ++y) {
      const int srcY = h - 1 - y;
      for (int x = 0; x < w; ++x) {
        const int srcIdx = (srcY * w + x) * 4;
        const int dstIdx = y * w + x;
        auto toSRGB = [](float v) -> icl8u {
          v = std::max(0.0f, std::min(1.0f, v));
          return static_cast<icl8u>(std::pow(v, 1.0f / 2.2f) * 255.0f + 0.5f);
        };
        r[dstIdx] = toSRGB(pixels[srcIdx + 0]);
        g[dstIdx] = toSRGB(pixels[srcIdx + 1]);
        b[dstIdx] = toSRGB(pixels[srcIdx + 2]);
      }
    }
  }

  mutable std::mutex m_mutex;
  core::Img8u m_image;
  std::atomic<int> m_updateCount{0};
};

// ---- CyclesRenderer::Impl ----

struct CyclesRenderer::Impl {
  geom::Scene &iclScene;
  RenderQuality quality;
  float sceneScale = 0.001f;  // mm → meters

  // Cycles objects
  unique_ptr<Session> session;
  Scene *scene = nullptr;  // owned by session
  ICLOutputDriver *outputDriver = nullptr;  // raw ptr, owned by session via unique_ptr

  // Scene synchronizer
  SceneSynchronizer sync;

  // Quality parameters (may be overridden)
  int samples = 64;
  int initialSamples = 1;
  int maxBounces = 6;
  bool denoising = true;
  float exposure = 1.0f;
  bool paramsOverridden = false;

  // State
  bool initialized = false;
  bool sceneDirty = true;
  bool hasRendered = false;
  bool renderInProgress = false;

  // Change detection
  float lastCamHash = 0;
  RenderQuality lastQuality = RenderQuality::Preview;

  // Progressive refinement state
  int currentSamples = 0;   // samples accumulated so far
  int targetSamples = 0;    // current target (doubles each step)

  Impl(geom::Scene &scene, RenderQuality q)
      : iclScene(scene), quality(q) {}

  void ensureInitialized() {
    if (initialized) return;

    ccl::log_init(nullptr);
    ccl::path_init(CYCLES_INSTALL_DIR);

    // Select device
    vector<DeviceInfo> devices = Device::available_devices();
    DeviceInfo deviceInfo = devices[0]; // CPU fallback
    for (const auto &d : devices) {
      if (d.type == DEVICE_METAL) {
        deviceInfo = d;
        break;
      }
    }

    // Session params
    SessionParams sessionParams;
    sessionParams.device = deviceInfo;
    sessionParams.background = (quality == RenderQuality::Final);
    applyQualityToParams(sessionParams);

    SceneParams sceneParams;
    sceneParams.shadingsystem = SHADINGSYSTEM_SVM;

    session = make_unique<Session>(sessionParams, sceneParams);
    scene = session->scene.get();

    // Output driver
    auto driver = ccl::make_unique<ICLOutputDriver>();
    outputDriver = driver.get();
    session->set_output_driver(std::move(driver));

    // Combined pass
    Pass *pass = scene->create_node<Pass>();
    pass->set_name(ustring("combined"));
    pass->set_type(PASS_COMBINED);

    // Film
    scene->film->set_exposure(exposure);

    initialized = true;
  }

  void applyQualityToParams(SessionParams &params) {
    if (paramsOverridden) {
      params.samples = samples;
      return;
    }
    switch (quality) {
      case RenderQuality::Preview:
        samples = 4;
        maxBounces = 2;
        denoising = false;
        break;
      case RenderQuality::Interactive:
        samples = 64;
        maxBounces = 6;
        denoising = true;
        break;
      case RenderQuality::Final:
        samples = 1024;
        maxBounces = 12;
        denoising = true;
        break;
    }
    params.samples = samples;
  }

  void applyQualityToScene() {
    if (!scene) return;

    Integrator *integrator = scene->integrator;
    integrator->set_max_bounce(maxBounces);

    switch (quality) {
      case RenderQuality::Preview:
        integrator->set_max_diffuse_bounce(1);
        integrator->set_max_glossy_bounce(1);
        integrator->set_max_transmission_bounce(1);
        integrator->set_use_denoise(false);
        break;
      case RenderQuality::Interactive:
        integrator->set_max_diffuse_bounce(3);
        integrator->set_max_glossy_bounce(3);
        integrator->set_max_transmission_bounce(4);
        integrator->set_use_denoise(denoising);
        break;
      case RenderQuality::Final:
        integrator->set_max_diffuse_bounce(6);
        integrator->set_max_glossy_bounce(6);
        integrator->set_max_transmission_bounce(8);
        integrator->set_use_denoise(denoising);
        break;
    }

    scene->film->set_exposure(exposure);
  }
};

// ---- CyclesRenderer public API ----

CyclesRenderer::CyclesRenderer(geom::Scene &scene, RenderQuality quality)
    : m_impl(std::make_unique<Impl>(scene, quality)) {}

CyclesRenderer::~CyclesRenderer() {
  if (m_impl && m_impl->session) {
    m_impl->session->cancel();
  }
}

CyclesRenderer::CyclesRenderer(CyclesRenderer &&) noexcept = default;
CyclesRenderer &CyclesRenderer::operator=(CyclesRenderer &&) noexcept = default;

void CyclesRenderer::render(int camIndex) {
  m_impl->ensureInitialized();
  m_impl->applyQualityToScene();

  // Detect what changed since last call
  float camHash = 0;
  if (camIndex < (int)m_impl->iclScene.getCameraCount()) {
    auto pos = m_impl->iclScene.getCamera(camIndex).getPosition();
    auto norm = m_impl->iclScene.getCamera(camIndex).getNorm();
    camHash = pos[0] + pos[1]*7 + pos[2]*13 + norm[0]*17 + norm[1]*23 + norm[2]*29;
  }

  bool needsReset = !m_impl->hasRendered
                 || (camHash != m_impl->lastCamHash)
                 || (m_impl->quality != m_impl->lastQuality)
                 || m_impl->sync.hasPendingChanges();

  m_impl->lastCamHash = camHash;
  m_impl->lastQuality = m_impl->quality;

  if (needsReset) {
    m_impl->sync.synchronize(
        m_impl->iclScene, camIndex, m_impl->scene, m_impl->sceneScale);

    int w = m_impl->scene->camera->get_full_width();
    int h = m_impl->scene->camera->get_full_height();
    if (w <= 0) w = 800;
    if (h <= 0) h = 600;

    int firstStep = std::min(m_impl->initialSamples, m_impl->samples);

    SessionParams sp = m_impl->session->params;
    sp.samples = firstStep;

    BufferParams bp;
    bp.width = w;  bp.height = h;
    bp.full_width = w;  bp.full_height = h;

    m_impl->session->reset(sp, bp);
    m_impl->session->start();
    m_impl->targetSamples = firstStep;
    m_impl->currentSamples = 0;
    m_impl->hasRendered = true;
    return;
  }

  // Already reached max — nothing more to do.
  if (m_impl->currentSamples >= m_impl->samples) return;

  // Check if current step has finished (non-blocking).
  double progress = m_impl->session->progress.get_progress();
  if (progress < 1.0) return;  // still rendering — GUI stays responsive

  // Current step done. Record it.
  m_impl->currentSamples = m_impl->targetSamples;

  // Advance to next step if below max.
  if (m_impl->currentSamples < m_impl->samples) {
    m_impl->targetSamples = std::min(
        m_impl->currentSamples + m_impl->initialSamples,
        m_impl->samples);
    // Extend the sample target — Cycles continues accumulating without
    // clearing the buffer.
    m_impl->session->set_samples(m_impl->targetSamples);
  }
}

const core::Img8u &CyclesRenderer::getImage() const {
  return m_impl->outputDriver->getImage();
}

void CyclesRenderer::setQuality(RenderQuality quality) {
  m_impl->quality = quality;
  m_impl->paramsOverridden = false;
}

RenderQuality CyclesRenderer::getQuality() const {
  return m_impl->quality;
}

void CyclesRenderer::setSamples(int samples) {
  m_impl->samples = samples;
  m_impl->paramsOverridden = true;
}

void CyclesRenderer::setInitialSamples(int n) {
  m_impl->initialSamples = std::max(1, n);
}

void CyclesRenderer::setMaxBounces(int bounces) {
  m_impl->maxBounces = bounces;
  m_impl->paramsOverridden = true;
}

void CyclesRenderer::setDenoising(bool enabled) {
  m_impl->denoising = enabled;
  m_impl->paramsOverridden = true;
}

void CyclesRenderer::setExposure(float exposure) {
  m_impl->exposure = exposure;
}

void CyclesRenderer::invalidateAll() {
  m_impl->sync.invalidateAll();
}

void CyclesRenderer::invalidateTransforms() {
  m_impl->sync.invalidateTransforms();
}

void CyclesRenderer::invalidateObject(geom::SceneObject *obj) {
  m_impl->sync.invalidateObject(obj);
}

float CyclesRenderer::getProgress() const {
  if (!m_impl->session) return 0.0f;
  return m_impl->session->progress.get_progress();
}

int CyclesRenderer::getUpdateCount() const {
  return m_impl->outputDriver ? m_impl->outputDriver->getUpdateCount() : 0;
}

bool CyclesRenderer::isRendering() const {
  if (!m_impl->session) return false;
  return m_impl->session->progress.get_progress() < 1.0f;
}

void CyclesRenderer::setDevice(const std::string &device) {
  // Must be called before first render
  // TODO: re-create session with new device if already initialized
}

void CyclesRenderer::setSceneScale(float scale) {
  m_impl->sceneScale = scale;
}

} // namespace icl::rt
