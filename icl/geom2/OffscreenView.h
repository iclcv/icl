// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/Img.h>
#include <memory>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::qt { class GLCallback; }

namespace icl::geom2 {

  class Scene2;
  class CyclesRenderer;

  /// Interactive on-screen Scene2 view + a switchable GL/Cycles OFFSCREEN render
  /// of a (possibly different) scene+camera — with the macOS-correct threading
  /// baked in.
  ///
  /// THE PROBLEM THIS SOLVES. An offscreen *GL* render of a Scene2
  /// (Scene2::renderToImage) MUST run on the GUI thread, in the on-screen
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
  ///   // ... in the worker run() loop:
  ///   if (view.poll()) { /* a NEW frame arrived */ }
  ///   const core::Img8u &cam = view.image();   // latest frame (cached)
  /// \endcode
  ///
  /// THREADING CONTRACT:
  ///   - callback() must be link()ed to the on-screen Canvas3D. Its draw() runs
  ///     on the GUI thread (widget context current) and performs the GL capture
  ///     there.
  ///   - poll() and setBackend() are called from your worker run() loop. poll()
  ///     drives the Cycles progressive render AND auto-requests a GL capture when
  ///     the view camera or backend changed, so call it EVERY frame; it returns
  ///     true only when a genuinely new frame is ready. image() is the cached
  ///     latest frame (so callers don't track their own "last frame").
  /// Also a utils::Configurable: it exposes the backend choice + Cycles tuning,
  /// and adds the captured scene as a child Configurable, so an app pulls the
  /// whole control set into its GUI with `gui << Prop(&view)` (no separate
  /// scene-props button needed). Properties:
  ///   "backend"                  GL (fast) / Cycles (photoreal)
  ///   "cycles.denoising"         OIDN on/off (off by default — it's slow)
  ///   "cycles.samples per step"  progressive granularity (1..16)
  ///   "scene.*"                  the capture scene's own props (enable lighting,
  ///                              background color, debug, …)
  class ICLGeom2_API OffscreenView : public utils::Configurable {
  public:
    enum class Backend { GL, Cycles };

    /// \a viewScene is rendered on screen through camera \a viewCam.
    explicit OffscreenView(Scene2 &viewScene, int viewCam = 0);
    ~OffscreenView();

    OffscreenView(const OffscreenView &) = delete;
    OffscreenView &operator=(const OffscreenView &) = delete;

    /// Render a different scene/camera offscreen (default: the view scene+cam).
    /// Call before the first poll()/capture (the Cycles backend binds to it).
    void setCaptureSource(Scene2 &capScene, int capCam = 0);

    /// When the capture scene differs from the view scene, copy the live view
    /// camera into the capture camera before each capture (default true — the
    /// "capture what I'm looking at" case). Turn off for a fixed capture camera.
    void setMirrorViewCamera(bool on);

    /// Select the offscreen backend (may be switched live). Cycles silently
    /// falls back to GL when ICL was built without Cycles.
    void setBackend(Backend b);
    Backend getBackend() const;

    /// Link target for gui["view"].link(...). Renders the view + GL capture.
    qt::GLCallback *callback();

    /// Explicitly request a GL capture on the next paint. Usually unnecessary —
    /// poll() auto-requests on camera/backend change and invalidate() on a scene
    /// edit — but available for app-specific triggers. No-op for Cycles.
    void requestCapture();

    /// Tell the view the capture scene's geometry/materials changed (e.g. a node
    /// was swapped): resyncs Cycles and requests a fresh GL capture.
    void invalidate();

    /// Call every frame (worker loop). Drives the Cycles progressive render and
    /// auto-requests a GL capture when the view camera or backend changed.
    /// Returns true when a NEW frame became available since the previous poll();
    /// read the pixels with image().
    bool poll();

    /// The latest captured frame (shallow copy; empty until the first capture).
    core::Img8u image() const;

    /// The Cycles backend's renderer (lazily created on first Cycles use), for
    /// tuning quality / scene scale / denoising. Returns nullptr if ICL was
    /// built without Cycles.
    CyclesRenderer *cycles();

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
  };

} // namespace icl::geom2
