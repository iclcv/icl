// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/Img.h>
#include <memory>

#ifndef ICLViz3d_API
#define ICLViz3d_API
#endif

namespace icl::qt { class GLCallback; }

namespace icl::viz3d {

  class Scene;
  class CyclesRenderer;

  /// Interactive on-screen Scene view + a switchable GL/Cycles OFFSCREEN render
  /// of a (possibly different) scene+camera — with the macOS-correct threading
  /// baked in.
  ///
  /// THE PROBLEM THIS SOLVES. An offscreen *GL* render of a Scene
  /// (Scene::renderToImage) MUST run on the GUI thread, in the on-screen
  /// widget's GL context. A second GL context driven from a worker thread
  /// serialises against the widget context on macOS and starves the GUI — the
  /// run() loop keeps ticking but the screen crawls. *Cycles* is GL-free and
  /// runs on the worker thread, where it must be polled every frame (it is a
  /// progressive renderer). Doing this split by hand is fiddly and easy to get
  /// wrong (it took several iterations in the checkerboard-detection lab);
  /// OffscreenView hides it behind a tiny pull API.
  ///
  /// USAGE — one interactive view + a captured image processed on the worker:
  /// \code
  ///   OffscreenView view(scene, 0);            // on-screen scene + view camera
  ///   view.setCaptureSource(capScene, 0);      // what to render offscreen
  ///   gui << Canvas3D({.handle="view"}) << ... << Show();
  ///   gui["view"].link(view.callback());       // wire the GUI-thread side
  ///   gui["view"].install(scene.getMouseHandler(0));
  ///   // ... in the worker run() loop (call EVERY frame — it drives Cycles):
  ///   auto f = view.next();        // f.image is always the latest capture
  ///   if (f.isNew) { /* a freshly arrived frame — process f.image */ }
  /// \endcode
  ///
  /// THREADING CONTRACT:
  ///   - callback() must be link()ed to the on-screen Canvas3D. Its draw() runs
  ///     on the GUI thread (widget context current) and performs the GL capture
  ///     there.
  ///   - next() and setBackend() are called from your worker run() loop. next()
  ///     drives the Cycles progressive render AND auto-requests a GL capture when
  ///     the view camera or backend changed, so call it EVERY frame; it always
  ///     returns the latest image, with isNew set only when one just arrived.
  /// Also a utils::Configurable: it exposes the backend choice + Cycles tuning,
  /// and adds the captured scene as a child Configurable, so an app pulls the
  /// whole control set into its GUI with `gui << Prop(&view)` (no separate
  /// scene-props button needed). Properties:
  ///   "backend"                  GL (fast) / Cycles (photoreal)
  ///   "distortion.k1" / ".k2"    forward radial lens distortion baked into the
  ///                              captured image (0,0 = ideal pinhole)
  ///   "distortion.reset"         button → zero k1,k2
  ///   "cycles.denoising"         OIDN on/off (off by default — it's slow)
  ///   "cycles.samples per step"  progressive granularity (1..16)
  ///   "scene.*"                  the capture scene's own props (enable lighting,
  ///                              background color, debug, …)
  class ICLViz3d_API OffscreenView : public utils::Configurable {
  public:
    enum class Backend { GL, Cycles };

    /// \a viewScene is rendered on screen through camera \a viewCam.
    explicit OffscreenView(Scene &viewScene, int viewCam = 0);
    ~OffscreenView();

    OffscreenView(const OffscreenView &) = delete;
    OffscreenView &operator=(const OffscreenView &) = delete;

    /// Render a different scene/camera offscreen (default: the view scene+cam).
    /// Call before the first poll()/capture (the Cycles backend binds to it).
    void setCaptureSource(Scene &capScene, int capCam = 0);

    /// When the capture scene differs from the view scene, copy the live view
    /// camera into the capture camera before each capture (default true — the
    /// "capture what I'm looking at" case). Turn off for a fixed capture camera.
    void setMirrorViewCamera(bool on);

    /// Select the offscreen backend (may be switched live). Cycles silently
    /// falls back to GL when ICL was built without Cycles.
    void setBackend(Backend b);
    Backend getBackend() const;

    /// The forward radial-distortion coefficients the captured image is warped
    /// with (the "distortion.k1/k2" properties). 0,0 = an ideal pinhole. A
    /// consumer that wants to *rectify* the frame (undistort) reads these to
    /// build the inverse map — the view applies only the forward (lens) warp.
    float distortionK1() const;
    float distortionK2() const;

    /// Link target for gui["view"].link(...). Renders the view + GL capture.
    qt::GLCallback *callback();

    /// Explicitly request a GL capture on the next paint. Usually unnecessary —
    /// poll() auto-requests on camera/backend change and invalidate() on a scene
    /// edit — but available for app-specific triggers. No-op for Cycles.
    void requestCapture();

    /// Tell the view the capture scene's geometry/materials changed (e.g. a node
    /// was swapped): resyncs Cycles and requests a fresh GL capture.
    void invalidate();

    /// A pulled frame: the latest captured image plus whether it's freshly arrived.
    struct Frame {
      core::Img8u image;   ///< latest frame, WITH forward lens distortion applied
                           ///< (== the clean render when k1==k2==0); shallow copy,
                           ///< always valid after the first capture, even when stale.
      bool isNew;          ///< true iff `image` differs from the previous next() call
    };

    /// Pull the current frame — call every frame from the worker loop.
    /** Drives the Cycles progressive render and auto-requests a GL capture when
        the view camera, backend, scene, or distortion changed, so it MUST be
        called every frame. The returned Frame ALWAYS carries the latest captured
        image (so a consumer that re-processes on a control change can use it
        directly — no need to cache the last frame); `isNew` flags whether that
        image just changed since the previous call. */
    Frame next();

    /// The Cycles backend's renderer (lazily created on first Cycles use), for
    /// tuning quality / scene scale / denoising. Returns nullptr if ICL was
    /// built without Cycles.
    CyclesRenderer *cycles();

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
  };

} // namespace icl::viz3d
