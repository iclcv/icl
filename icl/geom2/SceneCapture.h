// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/BVH.h>
#include <memory>

namespace icl::geom2 {

  class Scene2;

  /// Renders a Scene2 through a camera into RGB color + metric depth.
  /** Abstract backend: two interchangeable implementations share one
      vocabulary (BVH::ImageResult = {Img8u image; Img32f depth;} and
      BVH::DepthMode), so a consumer (e.g. a scene-backed RGBD / point-cloud
      source) can pick a backend by *capability* rather than by string tag:

        - BVHSceneCapture — CPU raytrace, fully headless (no GL context).
          The portable default; works in servers / tests / this sandbox.
        - GLSceneCapture  — GL offscreen render (Scene2::renderToImage), full
          PBR shading and GPU-fast, but needs a current GL context whose
          Renderer is the scene's (a live widget context, or a dedicated
          offscreen context owned by the capturer).

      Depth is in millimetres; DistToCamPlane = Z-depth, DistToCamCenter =
      Euclidean distance to the camera centre (matches the two ICL depth-image
      conventions consumed by PointCloudCreator).

      For a *photoreal* (path-traced) colour render of a scene — full GI, soft
      shadows, PBR materials, but no depth — see geom2::CyclesRenderer (a
      Raytracer, GL-free). SceneCapture is the geometry/depth-oriented capturer;
      CyclesRenderer is the beauty-render path. */
  class ICLGeom2_API SceneCapture {
  public:
    virtual ~SceneCapture() = default;

    /// Render \a scene through camera \a cameraIndex; returns RGB + depth.
    /** Returns an empty result on invalid camera index or missing capability. */
    virtual BVH::ImageResult capture(Scene2 &scene, int cameraIndex,
                                     BVH::DepthMode mode = BVH::DistToCamPlane) = 0;

    /// Convenience: capture just the RGB color image (depth skipped).
    /** Thin wrapper over capture(..., NoDepth) returning only the color Image —
        the common case when you don't need the depth buffer. Empty on failure. */
    core::Img8u captureRGB(Scene2 &scene, int cameraIndex) {
      return capture(scene, cameraIndex, BVH::NoDepth).image;
    }
  };

  /// Headless CPU backend: builds a BVH and raytraces it. No GL context needed.
  class ICLGeom2_API BVHSceneCapture : public SceneCapture {
  public:
    BVHSceneCapture();
    ~BVHSceneCapture() override;

    BVH::ImageResult capture(Scene2 &scene, int cameraIndex,
                             BVH::DepthMode mode = BVH::DistToCamPlane) override;

    /// Pixel subsampling (>1 trades output resolution for speed). Default 1×1.
    void setSubsampling(int stepX, int stepY);

    /// Reuse the BVH across captures instead of rebuilding it every call.
    /** Rebuilding the BVH per frame dominates the cost for a static scene. With
        caching on, the BVH is built once (and on the next capture after
        invalidate()) and only the raycast runs each frame. The caller is
        responsible for calling invalidate() whenever the scene geometry changes
        — otherwise captures reflect the stale geometry. Default: off. */
    void setCaching(bool enabled);

    /// Force the cached BVH to be rebuilt on the next capture (see setCaching).
    void invalidate();

  private:
    int  m_stepX = 1, m_stepY = 1;
    bool m_caching = false;
    bool m_dirty = true;
    BVH  m_bvh;
  };

  /// GL backend: offscreen render via Scene2::renderToImage (full shading).
  /** Two modes, chosen at construction:

        - Borrowed context (default, ownContext=false) — capture() runs
          Scene2::renderToImage straight on the caller's current GL context.
          Use this when you already are on the GL thread with a live widget
          context current (e.g. composed inside an on-screen draw callback).
          There is NO GL context of its own; calling capture() without one
          current yields an empty result.

        - Owned offscreen context (ownContext=true) — the capturer owns a
          self-contained QOpenGLContext + QOffscreenSurface (shared lists with
          Qt's global share context for textures), makeCurrent()s it for the
          duration of the render, and doneCurrent()s after. This makes a scene
          renderable to an image FROM ANY THREAD — in particular a worker
          `run()` — without coupling to the on-screen paint loop. Ported from
          legacy geom::Scene::PBuffer.

      Qt thread-affinity caveat (owned mode): a QOpenGLContext is bound to the
      thread that first makeCurrent()s it. The context is lazily created on the
      first capture() call, so always issue the first (and every) capture() from
      the SAME thread — typically the dedicated worker thread that owns this
      capturer. Requires a running QApplication (for the global share context).*/
  class ICLGeom2_API GLSceneCapture : public SceneCapture {
  public:
    /// \a ownContext selects owned-offscreen mode (see class doc).
    explicit GLSceneCapture(bool ownContext = false);
    ~GLSceneCapture() override;

    BVH::ImageResult capture(Scene2 &scene, int cameraIndex,
                             BVH::DepthMode mode = BVH::DistToCamPlane) override;

  private:
    struct OffscreenContext;
    std::unique_ptr<OffscreenContext> m_ctx;   ///< null unless ownContext
  };

} // namespace icl::geom2
