// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/OffscreenView.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/BVH.h>
#include <icl/geom/Camera.h>
#include <icl/qt/GLCallback.h>
#include <icl/qt/DrawWidget3D.h>

#ifdef ICL_HAVE_CYCLES
#include <icl/geom2/CyclesRenderer.h>
#endif

#include <mutex>
#include <atomic>
#include <cstdint>

namespace icl::geom2 {

#ifdef ICL_HAVE_CYCLES
  static constexpr bool kCyclesAvailable = true;
#else
  static constexpr bool kCyclesAvailable = false;
#endif

  struct OffscreenView::Impl {
    Scene2 *viewScene;
    int     viewCam;
    Scene2 *capScene;                 // defaults to viewScene
    int     capCam;                   // defaults to viewCam

    std::atomic<Backend> backend{Backend::GL};
    std::atomic<bool>    mirror{true};
    std::atomic<bool>    glPending{false};

    // Latest captured frame — written by whichever thread captured (GUI for GL,
    // worker for Cycles), read by poll() on the worker.
    std::mutex            mtx;
    core::Img8u           latest;
    std::atomic<uint32_t> seq{0};
    uint32_t              lastPolled = 0;

#ifdef ICL_HAVE_CYCLES
    std::unique_ptr<CyclesRenderer> cyc;
    int lastCycUpdate = -1;
#endif

    Impl(Scene2 &vs, int vc)
      : viewScene(&vs), viewCam(vc), capScene(&vs), capCam(vc) {}

    void publish(const core::Img8u &img) {
      if (!img.getDim()) return;
      std::scoped_lock l(mtx);
      latest = img;
      seq.fetch_add(1, std::memory_order_relaxed);
    }

    void syncCaptureCamera() {
      if (capScene != viewScene && mirror.load()) {
        std::scoped_lock<Scene2> l(*capScene);   // Scene2 is BasicLockable
        capScene->getCamera(capCam) = viewScene->getCamera(viewCam);
      }
    }

    // GUI thread (widget context current): render the view, and — when a GL
    // capture was requested — render the capture scene through the borrowed
    // context (cheap, ~1ms; no cross-thread/cross-context stall).
    void onDraw() {
      viewScene->render(viewCam);
      if (backend.load() == Backend::GL && glPending.exchange(false)) {
        BVH::ImageResult r;
        {
          std::scoped_lock<Scene2> l(*capScene);
          if (capScene != viewScene && mirror.load())
            capScene->getCamera(capCam) = viewScene->getCamera(viewCam);
          r = capScene->renderToImage(capCam, BVH::NoDepth);
        }
        publish(r.image);
      }
    }

    // The GLCallback we hand to gui[...].link().
    struct CB : public qt::GLCallback {
      Impl *o;
      explicit CB(Impl *i) : o(i) {}
      void draw(qt::ICLDrawWidget3D *) override { o->onDraw(); }
    } cb{this};
  };

  OffscreenView::OffscreenView(Scene2 &viewScene, int viewCam)
    : m_impl(std::make_unique<Impl>(viewScene, viewCam)) {}

  OffscreenView::~OffscreenView() = default;

  void OffscreenView::setCaptureSource(Scene2 &capScene, int capCam) {
    m_impl->capScene = &capScene;
    m_impl->capCam   = capCam;
  }

  void OffscreenView::setMirrorViewCamera(bool on) { m_impl->mirror.store(on); }

  void OffscreenView::setBackend(Backend b) {
    if (b == Backend::Cycles && !kCyclesAvailable) b = Backend::GL;
    m_impl->backend.store(b);
  }

  OffscreenView::Backend OffscreenView::getBackend() const {
    return m_impl->backend.load();
  }

  qt::GLCallback *OffscreenView::callback() { return &m_impl->cb; }

  void OffscreenView::requestCapture() {
    if (m_impl->backend.load() == Backend::GL) m_impl->glPending.store(true);
    // Cycles self-detects scene/camera changes in poll() → nothing to do.
  }

  void OffscreenView::invalidate() {
#ifdef ICL_HAVE_CYCLES
    if (m_impl->cyc) m_impl->cyc->invalidateAll();
#endif
  }

  bool OffscreenView::poll(core::Img8u &out) {
    auto &d = *m_impl;
#ifdef ICL_HAVE_CYCLES
    if (d.backend.load() == Backend::Cycles) {
      if (!d.cyc) {
        d.cyc = std::make_unique<CyclesRenderer>(*d.capScene, RenderQuality::Interactive);
        d.cyc->setSceneScale(1.0f);   // geom2 is in mm — see CyclesRenderer.h
        d.cyc->setSamplesPerStep(1);  // finest progressive step
        d.cyc->setDenoising(false);   // OIDN ~500ms/frame; opt in via cycles() if wanted
      }
      d.syncCaptureCamera();
      d.cyc->render(d.capCam);        // progressive — call every frame (see header)
      const int uc = d.cyc->getUpdateCount();
      if (uc != d.lastCycUpdate) { d.lastCycUpdate = uc; d.publish(d.cyc->getImage()); }
    }
#endif
    const uint32_t s = d.seq.load(std::memory_order_relaxed);
    if (s == d.lastPolled) return false;
    d.lastPolled = s;
    std::scoped_lock l(d.mtx);
    out = d.latest;
    return out.getDim() > 0;
  }

  CyclesRenderer *OffscreenView::cycles() {
#ifdef ICL_HAVE_CYCLES
    if (!m_impl->cyc) {
      m_impl->cyc = std::make_unique<CyclesRenderer>(*m_impl->capScene, RenderQuality::Interactive);
      m_impl->cyc->setSceneScale(1.0f);
      m_impl->cyc->setSamplesPerStep(1);
      m_impl->cyc->setDenoising(false);
    }
    return m_impl->cyc.get();
#else
    return nullptr;
#endif
  }

} // namespace icl::geom2
