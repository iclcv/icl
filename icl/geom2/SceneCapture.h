// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/BVH.h>

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
      conventions consumed by PointCloudCreator). */
  class ICLGeom2_API SceneCapture {
  public:
    virtual ~SceneCapture() = default;

    /// Render \a scene through camera \a cameraIndex; returns RGB + depth.
    /** Returns an empty result on invalid camera index or missing capability. */
    virtual BVH::ImageResult capture(Scene2 &scene, int cameraIndex,
                                     BVH::DepthMode mode = BVH::DistToCamPlane) = 0;
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
  /** Must be invoked with a current GL context whose Renderer is the scene's. */
  class ICLGeom2_API GLSceneCapture : public SceneCapture {
  public:
    BVH::ImageResult capture(Scene2 &scene, int cameraIndex,
                             BVH::DepthMode mode = BVH::DistToCamPlane) override;
  };

} // namespace icl::geom2
