// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/CyclesRenderer.h>
#include <icl/geom2/SceneSynchronizer.h>

#include <icl/geom2/Scene2.h>

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
#include "util/time.h"
#include "util/unique_ptr.h"

#include <cmath>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>

using namespace ccl;

namespace icl::geom2 {

// ---- RaytracingOutputBuffer: double-buffered image capture from Cycles ----
//
// Inherits from Cycles' OutputDriver to receive render tiles, but from ICL's
// perspective it's a thread-safe double-buffered image output.
//
// Cycles calls write_render_tile() from its render thread; the application
// reads getImage() from the GUI thread.
//
// Two Img8u buffers alternate as write target. After capturing a tile, the
// write buffer is published as the "front" buffer. Before the next write,
// the driver checks whether the alternate buffer's channel data is exclusively
// owned (isIndependent()). If yes, it switches to that buffer. If no (a
// getImage() caller still holds a shallow copy), it detaches the current
// buffer to allocate fresh channel storage. In steady state (display running
// at 30fps, Cycles producing ~2-10 updates/sec), the alternate buffer is
// always free, so detach() never fires.

class RaytracingOutputBuffer : public OutputDriver {
public:
  void write_render_tile(const Tile &tile) override {
    // Select write target: always the non-front buffer so we never
    // overwrite what the GL thread is currently reading.
    int front;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      front = m_frontIdx;
    }
    m_writeIdx = 1 - front;

    // If the target buffer's channel data is still held by a previous
    // getImage() caller (rare: GL thread kept an Image across two frames),
    // detach to get fresh storage rather than corrupting their data.
    if (m_buf[m_writeIdx].getChannels() > 0 && !m_buf[m_writeIdx].isIndependent()) {
      m_buf[m_writeIdx].detach();
    }

    if (!captureTile(tile)) return;

    // Publish: make the write buffer the new front buffer.
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_frontIdx = m_writeIdx;
    }

    // Signal management thread that a batch produced output.
    if (m_batchDoneCb) m_batchDoneCb();

    // Fire user callback with the freshly published image.
    if (m_onImageReady) {
      m_onImageReady(m_buf[m_frontIdx]);
    }
  }

  /// Returns the latest rendered image. The returned reference is stable:
  /// it points to the front buffer, which is never written to directly.
  /// Callers that need to hold the data beyond the next getImage() call
  /// should take an Image shallow copy — the shared_ptr keeps the channel
  /// data alive even after the front buffer index swaps.
  const core::Img8u &getImage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_buf[m_frontIdx];
  }

  int getUpdateCount() const { return m_updateCount; }

  void setOnImageReady(std::function<void(const core::Img8u &)> cb) {
    m_onImageReady = std::move(cb);
  }

  /// Called by management thread to receive batch-done signals from Cycles.
  void setBatchDoneCallback(std::function<void()> cb) {
    m_batchDoneCb = std::move(cb);
  }

private:
  bool captureTile(const Tile &tile) {
    try {
      if (!(tile.size == tile.full_size)) return false;

      const int w = tile.size.x;
      const int h = tile.size.y;
      if (w <= 0 || h <= 0) return false;

      std::vector<float> pixels(w * h * 4);
      if (!tile.get_pass_pixels("combined", 4, pixels.data())) return false;

      auto &dst = m_buf[m_writeIdx];
      dst.setSize(utils::Size(w, h));
      dst.setChannels(3);

      icl8u *r = dst.begin(0);
      icl8u *g = dst.begin(1);
      icl8u *b = dst.begin(2);

      // Cycles outputs pixels bottom-up; convert to top-down for ICL.
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

      m_updateCount++;
      return true;
    } catch (...) {
      // Can happen during session reset with changed resolution.
      return false;
    }
  }

  core::Img8u m_buf[2];        // double buffer
  int m_writeIdx = 0;           // which buffer we write to next (Cycles thread only)
  int m_frontIdx = 0;           // which buffer holds the latest result (guarded by m_mutex)
  mutable std::mutex m_mutex;   // guards m_frontIdx
  std::atomic<int> m_updateCount{0};
  std::function<void(const core::Img8u &)> m_onImageReady;
  std::function<void()> m_batchDoneCb;
};

// ---- CyclesRenderer::Impl ----

struct CyclesRenderer::Impl {
  geom2::Scene2 &iclScene;
  RenderQuality quality;
  float sceneScale = 0.001f;  // mm → meters

  // Cycles objects
  unique_ptr<Session> session;
  Scene *scene = nullptr;  // owned by session
  RaytracingOutputBuffer *outputDriver = nullptr;  // raw ptr, owned by session via unique_ptr

  // Scene synchronizer
  SceneSynchronizer sync;

  // Quality parameters (may be overridden)
  int samples = 64;
  int samplesPerStep = 1;
  int maxBounces = 6;
  bool denoising = true;
  float exposure = 1.0f;
  float brightness = 1.0f;
  float resolutionScale = 1.0f;
  bool paramsOverridden = false;

  // Initialization
  bool initialized = false;
  bool sessionStarted = false;

  // --- Autonomous mode state (used by managementLoop) ---
  std::thread managementThread;
  std::atomic<bool> running{false};
  std::atomic<int> activeCamIndex{0};
  std::mutex cvMutex;              // guards dirty/batchReady flags + CV
  std::condition_variable cv;      // signaled on dirty, batchReady, or !running
  bool dirty = true;               // starts dirty → first render triggers immediately
  bool batchReady = false;         // set by write_render_tile callback

  // --- Poll mode state (used by render()) ---
  enum class State { IDLE, WAIT_FOR_START, RENDERING };
  State state = State::IDLE;
  int accumulated = 0;             // samples rendered so far in current accumulation
  int target = 0;                  // current Cycles sample target
  int updateCountAtReset = 0;
  std::recursive_mutex renderMutex;  // protects render() state machine

  // Change detection (for poll-mode render() only)
  float lastCamHash = 0;
  int lastSamples = -1;
  int lastInitialSamples = -1;
  float lastBrightness = -1;
  int lastBounces = -1;
  bool lastDenoising = false;
  float lastExposure = -1;
  float lastResScale = -1;
  RenderQuality lastQuality = RenderQuality::Preview;

  Impl(geom2::Scene2 &scene, RenderQuality q)
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
    auto driver = ccl::make_unique<RaytracingOutputBuffer>();
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
    integrator->set_max_diffuse_bounce(maxBounces);
    integrator->set_max_glossy_bounce(maxBounces);
    integrator->set_max_transmission_bounce(maxBounces);
    integrator->set_use_denoise(denoising);

    scene->film->set_exposure(exposure);
  }

  /// Mark scene as dirty and wake the management thread (if running).
  /// Does NOT call session->cancel() — the management thread handles
  /// session lifecycle exclusively via reset() which cancels internally.
  void markDirty() {
    {
      std::lock_guard<std::mutex> lock(cvMutex);
      dirty = true;
    }
    cv.notify_one();
  }

  /// Compute a cheap hash of the camera pose for change detection.
  float computeCamHash(int camIndex) const {
    if (camIndex >= (int)iclScene.getCameraCount()) return 0;
    auto pos = iclScene.getCamera(camIndex).getPosition();
    auto norm = iclScene.getCamera(camIndex).getNorm();
    return pos[0] + pos[1]*7 + pos[2]*13 + norm[0]*17 + norm[1]*23 + norm[2]*29;
  }

  // --- Autonomous management loop (runs in managementThread) ---
  //
  // Outer loop: wait for dirty signal, sync scene, reset Cycles.
  // Inner loop: wait for batch completion via CV, extend samples, repeat.
  // Each batch finishes before dirty is checked, so intermediate images
  // are always captured and visible. Exits when running becomes false.

  void managementLoop() {
    ensureInitialized();

    // Wire the OutputDriver's batch-done signal into our CV.
    outputDriver->setBatchDoneCallback([this]() {
      {
        std::lock_guard<std::mutex> lock(cvMutex);
        batchReady = true;
      }
      cv.notify_one();
    });

    while (running) {
      // Wait for dirty signal or shutdown.
      {
        std::unique_lock<std::mutex> lock(cvMutex);
        cv.wait(lock, [&] { return dirty || !running.load(); });
        if (!running) break;
        dirty = false;
        batchReady = false;
      }

      // Sync ICL scene → Cycles scene.
      int camIndex = activeCamIndex;
      sync.setBackgroundStrength(brightness);
      sync.synchronize(iclScene, camIndex, scene, sceneScale);

      // Compute resolution.
      int wFull = scene->camera->get_full_width();
      int hFull = scene->camera->get_full_height();
      if (wFull <= 0) wFull = 800;
      if (hFull <= 0) hFull = 600;
      float s = std::max(0.1f, std::min(1.0f, resolutionScale));
      int w = std::max(1, (int)(wFull * s));
      int h = std::max(1, (int)(hFull * s));

      applyQualityToScene();
      scene->camera->set_full_width(w);
      scene->camera->set_full_height(h);
      scene->camera->compute_auto_viewplane();
      scene->camera->need_flags_update = true;

      int A = std::min(samplesPerStep, samples);

      SessionParams sp = session->params;
      sp.samples = A;
      BufferParams bp;
      bp.width = w;  bp.height = h;
      bp.full_width = w;  bp.full_height = h;

      // reset() waits for any active render to finish (producing a valid
      // last image), then reinitializes. No cancel needed — the current
      // batch is only 1 sample, so the wait is brief.
      session->reset(sp, bp);
      {
        std::lock_guard<std::mutex> lock(cvMutex);
        batchReady = false;
      }

      session->start();
      sessionStarted = true;
      accumulated = 0;
      target = A;

      // Progressive refinement: render N_step samples at a time.
      // Wait only for batch completion — NOT for dirty. This lets each
      // batch finish, producing a visible (noisy) intermediate image.
      // Dirty is checked AFTER each batch: if the scene changed during
      // rendering, we restart from scratch with the new camera. If not,
      // we extend samples and the image progressively refines.
      while (running) {
        {
          std::unique_lock<std::mutex> lock(cvMutex);
          cv.wait(lock, [&] { return batchReady || !running.load(); });
          if (!running) break;
          batchReady = false;
        }

        // Batch done — image captured and published by write_render_tile.
        accumulated = target;

        // Check if scene changed during this batch.
        {
          std::lock_guard<std::mutex> lock(cvMutex);
          if (dirty) break;  // outer loop picks up dirty immediately
        }

        if (accumulated >= samples) break;  // fully converged

        // Extend: render the next batch of samples (Cycles accumulates).
        target = std::min(accumulated + samplesPerStep, samples);
        session->set_samples(target);
        session->start();
      }
    }

    outputDriver->setBatchDoneCallback(nullptr);
  }
};

// ---- CyclesRenderer public API ----

CyclesRenderer::CyclesRenderer(geom2::Scene2 &scene, RenderQuality quality)
    : m_impl(std::make_unique<Impl>(scene, quality)) {}

CyclesRenderer::~CyclesRenderer() {
  stop();
  if (m_impl && m_impl->session) {
    m_impl->session->cancel();
  }
}

CyclesRenderer::CyclesRenderer(CyclesRenderer &&) noexcept = default;
CyclesRenderer &CyclesRenderer::operator=(CyclesRenderer &&) noexcept = default;

// ---- Autonomous mode: start/stop ----

void CyclesRenderer::start(int camIndex) {
  if (m_impl->running) return;
  m_impl->activeCamIndex = camIndex;
  m_impl->running = true;
  {
    std::lock_guard<std::mutex> lock(m_impl->cvMutex);
    m_impl->dirty = true;  // ensure first render fires
  }

  m_impl->managementThread = std::thread([this]() {
    m_impl->managementLoop();
  });
}

void CyclesRenderer::stop() {
  if (!m_impl->running) return;
  m_impl->running = false;
  m_impl->cv.notify_one();
  // Management thread's CV wait will see !running and exit.
  // Cycles' session thread may still be rendering; the management thread
  // will exit its loop, and ~CyclesRenderer calls session->cancel().
  if (m_impl->managementThread.joinable()) {
    m_impl->managementThread.join();
  }
}

// ---- Poll-driven mode: render() for scene-viewer use case ----
//
// Called once per frame from a run() loop. Non-blocking state machine:
// each call either advances state or returns immediately.

void CyclesRenderer::render(int camIndex) {
  std::lock_guard<std::recursive_mutex> lock(m_impl->renderMutex);
  m_impl->ensureInitialized();

  // --- Detect changes → set dirty ---
  float camHash = m_impl->computeCamHash(camIndex);
  if (camHash != m_impl->lastCamHash
      || m_impl->samples != m_impl->lastSamples
      || m_impl->samplesPerStep != m_impl->lastInitialSamples
      || m_impl->maxBounces != m_impl->lastBounces
      || m_impl->denoising != m_impl->lastDenoising
      || m_impl->brightness != m_impl->lastBrightness
      || m_impl->exposure != m_impl->lastExposure
      || m_impl->resolutionScale != m_impl->lastResScale
      || m_impl->quality != m_impl->lastQuality
      || m_impl->sync.hasPendingChanges()) {
    m_impl->dirty = true;
  }
  m_impl->lastCamHash = camHash;
  m_impl->lastSamples = m_impl->samples;
  m_impl->lastInitialSamples = m_impl->samplesPerStep;
  m_impl->lastBounces = m_impl->maxBounces;
  m_impl->lastDenoising = m_impl->denoising;
  m_impl->lastBrightness = m_impl->brightness;
  m_impl->lastExposure = m_impl->exposure;
  m_impl->lastResScale = m_impl->resolutionScale;
  m_impl->lastQuality = m_impl->quality;

  // --- Helper: full reset ---
  auto fullReset = [&]() {
    int wFull = m_impl->scene->camera->get_full_width();
    int hFull = m_impl->scene->camera->get_full_height();
    if (wFull <= 0) wFull = 800;
    if (hFull <= 0) hFull = 600;
    float s = std::max(0.1f, std::min(1.0f, m_impl->resolutionScale));
    int w = std::max(1, (int)(wFull * s));
    int h = std::max(1, (int)(hFull * s));

    m_impl->applyQualityToScene();
    m_impl->scene->camera->set_full_width(w);
    m_impl->scene->camera->set_full_height(h);
    m_impl->scene->camera->compute_auto_viewplane();
    m_impl->scene->camera->need_flags_update = true;

    int A = std::min(m_impl->samplesPerStep, m_impl->samples);

    SessionParams sp = m_impl->session->params;
    sp.samples = A;
    BufferParams bp;
    bp.width = w;  bp.height = h;
    bp.full_width = w;  bp.full_height = h;

    m_impl->session->reset(sp, bp);
    m_impl->session->start();
    m_impl->sessionStarted = true;
    m_impl->accumulated = 0;
    m_impl->target = A;
    m_impl->updateCountAtReset = m_impl->outputDriver->getUpdateCount();
    m_impl->state = Impl::State::WAIT_FOR_START;
  };

  auto syncScene = [&]() {
    m_impl->dirty = false;
    m_impl->sync.setBackgroundStrength(m_impl->brightness);
    m_impl->sync.synchronize(
        m_impl->iclScene, camIndex, m_impl->scene, m_impl->sceneScale);
  };

  // --- State machine ---
  using S = Impl::State;

  if (m_impl->state == S::IDLE) {
    if (!m_impl->dirty) return;
    syncScene();
    fullReset();
    return;
  }

  if (m_impl->state == S::WAIT_FOR_START) {
    if (m_impl->outputDriver->getUpdateCount() <= m_impl->updateCountAtReset) return;
    m_impl->state = S::RENDERING;
  }

  double progress = m_impl->session->progress.get_progress();
  if (progress < 1.0) return;  // still rendering — don't block

  m_impl->accumulated = m_impl->target;

  if (m_impl->dirty) {
    syncScene();
    fullReset();
  } else if (m_impl->accumulated < m_impl->samples) {
    int A = m_impl->samplesPerStep;
    m_impl->target = std::min(m_impl->accumulated + A, m_impl->samples);
    m_impl->session->set_samples(m_impl->target);
    m_impl->session->start();
  } else {
    m_impl->state = S::IDLE;
  }
}

// ---- Blocking render (offline/headless) ----

void CyclesRenderer::renderBlocking(int camIndex) {
  m_impl->ensureInitialized();
  m_impl->sync.setBackgroundStrength(m_impl->brightness);
  m_impl->sync.synchronize(
      m_impl->iclScene, camIndex, m_impl->scene, m_impl->sceneScale);

  int w = m_impl->scene->camera->get_full_width();
  int h = m_impl->scene->camera->get_full_height();
  if (w <= 0) w = 800;
  if (h <= 0) h = 600;

  SessionParams sp = m_impl->session->params;
  sp.samples = m_impl->samples;
  BufferParams bp;
  bp.width = w;  bp.height = h;
  bp.full_width = w;  bp.full_height = h;

  m_impl->applyQualityToScene();
  m_impl->session->reset(sp, bp);
  m_impl->session->start();
  m_impl->sessionStarted = true;
  m_impl->session->wait();

  m_impl->accumulated = m_impl->samples;
  m_impl->state = Impl::State::IDLE;
  m_impl->dirty = false;
}

// ---- Image access ----

const core::Img8u &CyclesRenderer::getImage() const {
  static const core::Img8u empty;
  if (!m_impl->outputDriver) return empty;
  return m_impl->outputDriver->getImage();
}

void CyclesRenderer::setOnImageReady(std::function<void(const core::Img8u &)> cb) {
  if (!m_impl->outputDriver) return;  // will be set once initialized
  m_impl->outputDriver->setOnImageReady(std::move(cb));
}

// ---- Quality / parameter setters ----
//
// Each setter that changes rendering state calls markDirty() so the
// autonomous management thread restarts immediately.

void CyclesRenderer::setQuality(RenderQuality quality) {
  if (m_impl->quality == quality) return;
  m_impl->quality = quality;
  m_impl->paramsOverridden = false;
  m_impl->markDirty();
}

RenderQuality CyclesRenderer::getQuality() const {
  return m_impl->quality;
}

void CyclesRenderer::setSamples(int samples) {
  if (m_impl->samples == samples) return;
  m_impl->samples = samples;
  m_impl->paramsOverridden = true;
  m_impl->markDirty();
}

void CyclesRenderer::setSamplesPerStep(int n) {
  m_impl->samplesPerStep = std::max(1, n);
}

void CyclesRenderer::setMaxBounces(int bounces) {
  if (m_impl->maxBounces == bounces) return;
  m_impl->maxBounces = bounces;
  m_impl->paramsOverridden = true;
  m_impl->markDirty();
}

void CyclesRenderer::setDenoising(bool enabled) {
  if (m_impl->denoising == enabled) return;
  m_impl->denoising = enabled;
  m_impl->paramsOverridden = true;
  m_impl->markDirty();
}

void CyclesRenderer::setExposure(float exposure) {
  if (m_impl->exposure == exposure) return;
  m_impl->exposure = exposure;
  m_impl->markDirty();
}

void CyclesRenderer::setBrightness(float b) {
  b = std::max(0.0f, std::min(1.0f, b));
  if (m_impl->brightness == b) return;
  m_impl->brightness = b;
  m_impl->markDirty();
}

void CyclesRenderer::setResolutionScale(float scale) {
  scale = std::max(0.1f, std::min(1.0f, scale));
  if (m_impl->resolutionScale == scale) return;
  m_impl->resolutionScale = scale;
  m_impl->markDirty();
}

// ---- Scene invalidation ----

void CyclesRenderer::invalidateAll() {
  m_impl->sync.invalidateAll();
  m_impl->markDirty();
}

void CyclesRenderer::invalidateTransforms() {
  m_impl->sync.invalidateTransforms();
  m_impl->markDirty();
}

void CyclesRenderer::invalidateNode(geom2::Node *obj) {
  m_impl->sync.invalidateNode(obj);
  m_impl->markDirty();
}

// ---- Progress / state queries ----

float CyclesRenderer::getProgress() const {
  if (!m_impl->session) return 0.0f;
  return m_impl->session->progress.get_progress();
}

int CyclesRenderer::getUpdateCount() const {
  return m_impl->outputDriver ? m_impl->outputDriver->getUpdateCount() : 0;
}

bool CyclesRenderer::isRendering() const {
  if (!m_impl->session) return false;
  return m_impl->running || m_impl->session->progress.get_progress() < 1.0f;
}

// ---- Device / scale ----

void CyclesRenderer::setDevice(const std::string &device) {
  // Must be called before first render.
  // TODO: re-create session with new device if already initialized.
  (void)device;
}

void CyclesRenderer::setSceneScale(float scale) {
  m_impl->sceneScale = scale;
}

} // namespace icl::geom2
