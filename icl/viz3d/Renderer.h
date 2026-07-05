// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/Node.h>
#include <memory>
#include <vector>

namespace icl::viz3d {

  /// GL 4.1 Core renderer for viz3d scene graphs
  /** Traverses the node graph with dynamic_cast dispatch:
      - GroupNode → recurse into children
      - GeometryNode → build geometry cache, render with PBR shader
      Lines and points rendered with a separate unlit shader.
      No legacy fixed-function GL. */
  class ICLViz3d_API Renderer {
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

    /// Enable/disable screen-space reflections (default: true)
    void setSSREnabled(bool enabled);

    /// Whether screen-space reflections are currently enabled
    /** Used by Scene2::renderToImage to force SSR off during an offscreen
        capture and restore the prior state afterwards. */
    bool isSSREnabled() const;

    /// Enable/disable shadow mapping (default: true)
    void setShadowsEnabled(bool enabled);

    /// Enable/disable lighting (default: true). When off, geometry renders as
    /// flat unlit base color (the "enable lighting" scene property).
    void setLightingEnabled(bool enabled);

    /// Enable/disable the procedural sky background (default: false)
    /** When enabled, a full-screen gradient (matching the sky model used for
        environment reflections) is drawn behind the scene, so the backdrop and
        reflections agree. Disabled → the flat clear color shows through. */
    void setSkyEnabled(bool enabled);

    /// Set the world "up" direction the sky gradient is oriented along
    /** Default (0,1,0). Physics scenes are Z-up → pass (0,0,1). */
    void setSkyUp(float x, float y, float z);

    /// Set debug visualization mode
    /** 0=shaded (default), 1=normals, 2=albedo, 3=UVs, 4=lighting only,
        5=NdotL, 6=SSR confidence, 7=depth buffer, 8=SSR only */
    void setDebugMode(int mode);

    /// Invalidate all geometry caches (call when scene structure changes)
    /** Safe to call from any thread — actual GL cleanup is deferred to
        the next render() call on the GL thread. */
    void invalidateCache();

    /// Flush invalidated caches (called automatically by render on GL thread)
    void flushInvalidatedCache();

  private:
    struct Data;
    std::unique_ptr<Data> m_data;

    void ensureShaderCompiled();
    void renderNode(Node *node, const Mat &viewMatrix, int pass = 0);
    void renderNodeShadow(Node *node);
  };

} // namespace icl::viz3d
