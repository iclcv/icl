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

  bool update_render_tile(const Tile &tile) override {
    captureTile(tile);
    return true;
  }

  const core::Img8u &getImage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_image;
  }

private:
  void captureTile(const Tile &tile) {
    if (!(tile.size == tile.full_size))
      return;

    const int w = tile.size.x;
    const int h = tile.size.y;

    std::vector<float> pixels(w * h * 4);
    if (!tile.get_pass_pixels("combined", 4, pixels.data()))
      return;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_image.setSize(utils::Size(w, h));
    m_image.setChannels(3);

    icl8u *r = m_image.begin(0);
    icl8u *g = m_image.begin(1);
    icl8u *b = m_image.begin(2);

    // Y row is negated in camera matrix to match ICL's down-pointing up vector.
    // This makes Cycles output top-down already — no flip needed.
    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x) {
        const int srcIdx = (y * w + x) * 4;
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
  int maxBounces = 6;
  bool denoising = true;
  float exposure = 1.0f;
  bool paramsOverridden = false;

  // State
  bool initialized = false;
  bool sceneDirty = true;
  bool hasRendered = false;

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

  // Synchronize ICL scene → Cycles scene
  auto syncResult = m_impl->sync.synchronize(
      m_impl->iclScene, camIndex, m_impl->scene, m_impl->sceneScale);

  // Get camera resolution for buffer params
  int w = m_impl->scene->camera->get_full_width();
  int h = m_impl->scene->camera->get_full_height();
  if (w <= 0) w = 800;
  if (h <= 0) h = 600;

  using SR = SceneSynchronizer::SyncResult;
  bool needsRender = !m_impl->hasRendered
                  || syncResult != SR::NoChange;

  if (needsRender) {
    SessionParams sessionParams = m_impl->session->params;
    sessionParams.samples = m_impl->samples;

    BufferParams bufferParams;
    bufferParams.width = w;
    bufferParams.height = h;
    bufferParams.full_width = w;
    bufferParams.full_height = h;

    m_impl->session->reset(sessionParams, bufferParams);
    m_impl->session->start();
    m_impl->session->wait();
    m_impl->hasRendered = true;
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
