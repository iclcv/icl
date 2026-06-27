// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
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
  ///   view.setBackend(useCycles ? OffscreenView::Backend::Cycles
  ///                             : OffscreenView::Backend::GL);
  ///   if (inputsChanged) view.requestCapture(); // GL: capture next paint
  ///   core::Img8u img;
  ///   if (view.poll(img)) { /* a NEW frame arrived → process it */ }
  /// \endcode
  ///
  /// THREADING CONTRACT:
  ///   - callback() must be link()ed to the on-screen Canvas3D. Its draw() runs
  ///     on the GUI thread (widget context current) and performs the GL capture
  ///     there.
  ///   - poll(), requestCapture() and setBackend() are called from your worker
  ///     run() loop. poll() also drives the Cycles progressive render, so call it
  ///     EVERY frame; it returns true only when a genuinely new frame is ready.
  class ICLGeom2_API OffscreenView {
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

    /// (GL backend) request a capture on the next paint. No-op for Cycles, which
    /// self-detects scene/camera changes — harmless to call unconditionally.
    void requestCapture();

    /// Tell the backend the capture scene's geometry/materials changed (e.g. a
    /// node was swapped). Forces a full Cycles resync; no-op for GL (which always
    /// re-renders) and when no Cycles renderer exists yet.
    void invalidate();

    /// Latest captured frame. Returns true and fills \a out when a NEW frame has
    /// arrived since the previous poll(); false otherwise. Call every frame.
    bool poll(core::Img8u &out);

    /// The Cycles backend's renderer (lazily created on first Cycles use), for
    /// tuning quality / scene scale / denoising. Returns nullptr if ICL was
    /// built without Cycles.
    CyclesRenderer *cycles();

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
  };

} // namespace icl::geom2
