// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/SceneCapture.h>
#include <icl/geom2/Scene2.h>
#include <algorithm>

namespace icl::geom2 {

  BVHSceneCapture::BVHSceneCapture() = default;
  BVHSceneCapture::~BVHSceneCapture() = default;

  void BVHSceneCapture::setSubsampling(int stepX, int stepY) {
    m_stepX = std::max(1, stepX);
    m_stepY = std::max(1, stepY);
  }

  void BVHSceneCapture::setCaching(bool enabled) {
    m_caching = enabled;
    m_dirty = true;   // force a (re)build on the next capture either way
  }

  void BVHSceneCapture::invalidate() { m_dirty = true; }

  BVH::ImageResult BVHSceneCapture::capture(Scene2 &scene, int cameraIndex,
                                            BVH::DepthMode mode) {
    if (cameraIndex < 0 || cameraIndex >= scene.getCameraCount()) return {};
    if (m_caching) {
      // Build once; reuse until the caller invalidate()s. Rebuilding the BVH per
      // frame is the dominant cost for a static scene — caching removes it.
      if (m_dirty) { m_bvh = scene.buildBVH(); m_dirty = false; }
      return m_bvh.raycastToImage(scene.getCamera(cameraIndex), mode, m_stepX, m_stepY);
    }
    BVH bvh = scene.buildBVH();
    return bvh.raycastToImage(scene.getCamera(cameraIndex), mode, m_stepX, m_stepY);
  }

  BVH::ImageResult GLSceneCapture::capture(Scene2 &scene, int cameraIndex,
                                           BVH::DepthMode mode) {
    return scene.renderToImage(cameraIndex, mode);
  }

} // namespace icl::geom2
