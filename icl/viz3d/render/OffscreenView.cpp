// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/render/OffscreenView.h>
#include <icl/viz3d/scene/Scene2.h>
#include <icl/viz3d/render/BVH.h>
#include <icl/cv3d/Camera.h>
#include <icl/qt/GLCallback.h>
#include <icl/qt/DrawWidget3D.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/filter/affine/ImageUndistortion.h>   // radial lens model + warp maps
#include <icl/filter/affine/WarpOp.h>               // efficient warp-map application

#ifdef ICL_HAVE_CYCLES
#include <icl/viz3d/render/CyclesRenderer.h>
#endif

#include <mutex>
#include <atomic>
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace icl::viz3d {

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

    // Change detection for auto-requesting GL captures (poll()).
    bool   haveCamSnap = false;
    cv3d::Vec snapPos, snapNorm, snapUp;
    int    reqBackend = -1;
    unsigned lastSceneVersion = 0;    // capScene->sceneVersion() last seen by poll()

    // Latest frames — written by whichever thread captured (GUI for GL, worker
    // for Cycles), read by poll()/image() on the worker. `rawLatest` is the clean
    // render; `latest` is rawLatest after the forward lens distortion. Keeping the
    // raw frame lets a k1/k2 change re-distort without a fresh capture.
    std::mutex            mtx;
    core::Img8u           rawLatest;       // clean render (pre-distortion)
    core::Img8u           latest;          // = distort(rawLatest)
    std::atomic<uint32_t> seq{0};
    uint32_t              lastPolled = 0;

    // Forward (lens) distortion — simulates a real camera. Params come from the
    // "distortion.k1/k2" Configurable props (mirrored into atomics by poll()).
    // The warp op + its cache key are touched only under mtx (in refreshOutput()).
    // Zero (black) border: pixels whose lens back-mapping lands outside the
    // rendered frame are set to black — the only sensible border. (Clamp would
    // smear the source edge — e.g. Cycles' sky — across the whole OOB region.)
    std::atomic<float>    k1{0.f}, k2{0.f};
    // auto-scale ("fill frame"): zoom the lens warp so the distorted frame stays
    // fully covered by source pixels — no black border at all (false until the
    // user opts in).
    std::atomic<bool>     autoScale{false};
    filter::WarpOp        distort{core::Img32f(), core::interpolateLIN, true,
                                  filter::WarpOp::BorderMode::Zero};
    float                 lk1 = 1e9f, lk2 = 1e9f;   // last-built warp-map key
    bool                  lautoScale = false;
    utils::Size           lsz{0, 0};

#ifdef ICL_HAVE_CYCLES
    std::unique_ptr<CyclesRenderer> cyc;
    int  lastCycUpdate = -1;
    bool lastDenoise = false;   // mirrors the Cycles defaults set on creation
    int  lastSPS = 1;
#endif

    Impl(Scene2 &vs, int vc)
      : viewScene(&vs), viewCam(vc), capScene(&vs), capCam(vc) {}

    // A fresh capture arrived: cache it raw, then (re)derive the distorted output.
    void publish(const core::Img8u &img) {
      if (!img.getDim()) return;
      std::scoped_lock l(mtx);
      rawLatest = img;
      refreshOutput();
    }

    // (Re)build `latest` = distort(rawLatest) and bump seq. Call under mtx after
    // a new capture (publish) OR a distortion-param change (poll). Passthrough
    // when k1==k2==0 (the "no lens" case — no warp cost).
    void refreshOutput() {
      const float a = k1.load(std::memory_order_relaxed);
      const float b = k2.load(std::memory_order_relaxed);
      if (a == 0.f && b == 0.f) {
        latest = rawLatest;
      } else {
        const utils::Size sz = rawLatest.getSize();
        const bool as = autoScale.load(std::memory_order_relaxed);
        if (a != lk1 || b != lk2 || as != lautoScale || sz != lsz) {   // rebuild on change
          const double f = std::max(sz.width, sz.height) / 2.0;
          const double cx = sz.width / 2.0, cy = sz.height / 2.0;
          // Coefficients are NEGATED so distortion.k1/k2 follow the Matlab/calibration
          // convention (k1<0 ⇒ barrel): createWarpMap resamples output→input, so baking
          // it with +k warps the image by the INVERSE of a +k lens — a rendered board
          // would then calibrate to −k. Feeding −k makes the baked distortion the one a
          // calibrator recovers as k1 (verified against cv::IntrinsicCalibrator).
          filter::ImageUndistortion ud("MatlabModel5Params",
              {f, f, cx, cy, 0, -(double)a, -(double)b, 0, 0, 0}, sz);
          // Use the EXACT forward map (createWarpMap), NOT createInverseWarpMap: the
          // latter inverts the model by additive fixed-point iteration, which DIVERGES
          // for strong distortion (|k1|≳0.2) and collapses to a near-zero warp — so
          // extreme sliders would barely distort. The forward map is closed-form and
          // grows monotonically. The lab's optional rectify pass inverts the SAME
          // (negated) model. autoScale zooms the map to keep the frame border-free.
          distort.setWarpMap(ud.createWarpMap(as));
          lk1 = a; lk2 = b; lautoScale = as; lsz = sz;
        }
        latest = distort.apply(core::Image(rawLatest)).as<icl8u>();
      }
      seq.fetch_add(1, std::memory_order_relaxed);
    }

    // Mirror the distortion props into the atomics; re-distort the cached frame on
    // change (no recapture needed). Returns true if a param changed.
    bool syncDistortion(float a, float b, bool as) {
      if (a == k1.load() && b == k2.load() && as == autoScale.load()) return false;
      k1.store(a); k2.store(b); autoScale.store(as);
      std::scoped_lock l(mtx);
      if (rawLatest.getDim()) refreshOutput();
      return true;
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
    : m_impl(std::make_unique<Impl>(viewScene, viewCam)) {
    addProperty("backend", utils::prop::Menu{"GL (fast)", "Cycles (photoreal)"}, "GL (fast)");
    // Forward radial lens distortion baked into the captured image (simulate a
    // real camera). k1==k2==0 → an ideal pinhole (no warp). reset → zero both.
    addProperty("distortion.k1", utils::prop::Range{.min=-0.4f, .max=0.4f}, 0.f);
    addProperty("distortion.k2", utils::prop::Range{.min=-0.3f, .max=0.3f}, 0.f);
    // "fill frame": auto-zoom the lens warp so the distorted frame has no
    // out-of-bounds (black/clamped) border — fully covered by source pixels.
    addProperty("distortion.fill frame", utils::prop::Flag{}, false);
    addProperty("distortion.reset", utils::prop::Command{});
    addProperty("cycles.denoising", utils::prop::Flag{}, false);
    addProperty("cycles.samples per step", utils::prop::Range{.min=1, .max=16}, 1);
    // Expose the captured scene's own properties (enable lighting, background,
    // debug, …) under "scene." — the lighting toggle "comes from the scene".
    addChildConfigurable(m_impl->capScene, "scene");

    registerCallback([this](const utils::Configurable::Property &p){
      if (p.name == "distortion.reset") {
        setPropertyValue("distortion.k1", 0.f);
        setPropertyValue("distortion.k2", 0.f);
      }
    });
  }

  OffscreenView::~OffscreenView() = default;

  void OffscreenView::setCaptureSource(Scene2 &capScene, int capCam) {
    if (m_impl->capScene != &capScene) {
      removeChildConfigurable(m_impl->capScene);   // re-point the "scene." child
      addChildConfigurable(&capScene, "scene");
    }
    m_impl->capScene = &capScene;
    m_impl->capCam   = capCam;
    m_impl->lastSceneVersion = 0;   // force a resync of the new capture scene
  }

  void OffscreenView::setMirrorViewCamera(bool on) { m_impl->mirror.store(on); }

  void OffscreenView::setBackend(Backend b) {
    if (b == Backend::Cycles && !kCyclesAvailable) b = Backend::GL;
    m_impl->backend.store(b);
    setPropertyValue("backend", b == Backend::Cycles ? "Cycles (photoreal)" : "GL (fast)");
  }

  OffscreenView::Backend OffscreenView::getBackend() const {
    return m_impl->backend.load();
  }

  float OffscreenView::distortionK1() const { return m_impl->k1.load(); }
  float OffscreenView::distortionK2() const { return m_impl->k2.load(); }

  qt::GLCallback *OffscreenView::callback() { return &m_impl->cb; }

  void OffscreenView::requestCapture() {
    if (m_impl->backend.load() == Backend::GL) m_impl->glPending.store(true);
    // Cycles self-detects scene/camera changes in poll() → nothing to do.
  }

  void OffscreenView::invalidate() {
    m_impl->glPending.store(true);   // GL: recapture the (changed) scene
#ifdef ICL_HAVE_CYCLES
    if (m_impl->cyc) m_impl->cyc->invalidateAll();
#endif
  }

  OffscreenView::Frame OffscreenView::next() {
    auto &d = *m_impl;

    // The Configurable properties are the UI-facing source of truth — sync them.
    Backend nb;
    {
      const std::string b = prop("backend").value;
      nb = (!b.empty() && b[0] == 'C') ? Backend::Cycles : Backend::GL;
      if (!kCyclesAvailable) nb = Backend::GL;
      d.backend.store(nb);
    }

    // Lens distortion params (camera lens): on change, re-distort the cached raw
    // frame in place — no recapture needed (it's a pure image post-process).
    d.syncDistortion(prop("distortion.k1").value, prop("distortion.k2").value,
                     prop("distortion.fill frame").value);

    // Auto-pick up scene edits: a node mutator (via Node::ScopedEdit) or
    // add/removeNode bumps the capture scene's version. On a change, drop Cycles'
    // cache and re-request a GL capture — this replaces the manual invalidate()
    // the app used to call after a board edit.
    const unsigned sv = d.capScene->sceneVersion();
    if (sv != d.lastSceneVersion) {
      d.lastSceneVersion = sv;
      d.glPending.store(true);
#ifdef ICL_HAVE_CYCLES
      if (d.cyc) d.cyc->invalidateAll();
#endif
    }

    // Auto-request a GL capture when the rendered scene's appearance would change:
    // the view camera moved or the backend switched (e.g. Cycles→GL). So callers
    // don't track camMoved / call requestCapture themselves.
    const bool backendChanged = ((int)nb != d.reqBackend);
    d.reqBackend = (int)nb;
    if (nb == Backend::GL) {
      const cv3d::Camera &cam = d.viewScene->getCamera(d.viewCam);
      auto same = [](const cv3d::Vec &a, const cv3d::Vec &b){
        return a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && a[3]==b[3]; };
      const bool camChanged = !d.haveCamSnap
          || !same(cam.getPosition(), d.snapPos) || !same(cam.getNorm(), d.snapNorm)
          || !same(cam.getUp(), d.snapUp);
      if (camChanged || backendChanged) d.glPending.store(true);
      d.haveCamSnap = true;
      d.snapPos = cam.getPosition(); d.snapNorm = cam.getNorm(); d.snapUp = cam.getUp();
    }
#ifdef ICL_HAVE_CYCLES
    if (d.backend.load() == Backend::Cycles) {
      if (!d.cyc) {
        d.cyc = std::make_unique<CyclesRenderer>(*d.capScene, RenderQuality::Interactive);
        d.cyc->setSceneScale(1.0f);   // viz3d is in mm — see CyclesRenderer.h
        d.cyc->setSamplesPerStep(1);  // finest progressive step
        d.cyc->setDenoising(false);   // OIDN ~500ms/frame; toggle via the property
      }
      // Apply Cycles tuning only on change (mutating every frame can stop it
      // converging).
      const bool den = prop("cycles.denoising").value;
      const int  sps = prop("cycles.samples per step").value;
      if (den != d.lastDenoise) { d.cyc->setDenoising(den); d.lastDenoise = den; }
      if (sps != d.lastSPS)     { d.cyc->setSamplesPerStep(sps); d.lastSPS = sps; }

      d.syncCaptureCamera();
      d.cyc->render(d.capCam);        // progressive — call every frame (see header)
      const int uc = d.cyc->getUpdateCount();
      if (uc != d.lastCycUpdate) { d.lastCycUpdate = uc; d.publish(d.cyc->getImage()); }
    }
#endif
    const uint32_t s = d.seq.load(std::memory_order_relaxed);
    const bool isNew = (s != d.lastPolled);
    d.lastPolled = s;
    std::scoped_lock l(d.mtx);
    return Frame{ d.latest, isNew };   // latest is a shallow copy under the lock
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

} // namespace icl::viz3d
