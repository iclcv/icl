// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Node.h>
#include <memory>
#include <vector>

namespace icl::geom2 {

  /// GL 4.1 Core renderer for geom2 scene graphs
  /** Traverses the node graph with dynamic_cast dispatch:
      - GroupNode → recurse into children
      - GeometryNode → build geometry cache, render with PBR shader
      Lines and points rendered with a separate unlit shader.
      No legacy fixed-function GL. */
  class ICLGeom2_API Renderer {
  public:
    Renderer();
    ~Renderer();

    /// Render a list of top-level nodes with given view and projection matrices
    void render(const std::vector<std::shared_ptr<Node>> &nodes,
                const Mat &viewMatrix,
                const Mat &projectionMatrix);

    /// Set exposure for tone mapping
    void setExposure(float exposure);

    /// Set ambient light level
    void setAmbient(float ambient);

    /// Set overlay alpha (0..1). When < 1, geometry is semi-transparent
    /// so a background image (e.g. Cycles render) shows through.
    void setOverlayAlpha(float alpha);

    /// Invalidate all geometry caches (call when scene structure changes)
    void invalidateCache();

  private:
    struct Data;
    std::unique_ptr<Data> m_data;

    void ensureShaderCompiled();
    void renderNode(Node *node, const Mat &viewMatrix);
  };

} // namespace icl::geom2
