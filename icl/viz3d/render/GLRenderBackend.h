// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/render/RenderBackend.h>
#include <icl/viz3d/nodes/Node.h>
#include <memory>
#include <vector>

namespace icl::viz3d {

  /// GL 4.1 Core RenderBackend for viz3d scene graphs
  /** Traverses the node graph with dynamic_cast dispatch:
      - GroupNode → recurse into children
      - GeometryNode → build geometry cache, render with PBR shader
      Lines and points rendered with a separate unlit shader.
      No legacy fixed-function GL. This is the transitional backend being
      superseded by FilamentRenderBackend (see filament-plan.md). */
  class ICLViz3d_API GLRenderBackend : public RenderBackend {
  public:
    GLRenderBackend();
    ~GLRenderBackend() override;

    void render(const std::vector<std::shared_ptr<Node>> &nodes,
                const Mat &viewMatrix,
                const Mat &projectionMatrix) override;

    void setExposure(float exposure) override;
    void setAmbient(float ambient) override;
    void setOverlayAlpha(float alpha) override;
    void setSSREnabled(bool enabled) override;
    bool isSSREnabled() const override;
    void setShadowsEnabled(bool enabled) override;
    void setLightingEnabled(bool enabled) override;
    void setSkyEnabled(bool enabled) override;
    void setSkyUp(float x, float y, float z) override;
    void setDebugMode(int mode) override;
    void invalidateCache() override;
    void flushInvalidatedCache() override;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;

    void ensureShaderCompiled();
    void renderNode(Node *node, const Mat &viewMatrix, int pass = 0);
    void renderNodeShadow(Node *node);
  };

} // namespace icl::viz3d
