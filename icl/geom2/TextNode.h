// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/GeometryNode.h>
#include <string>
#include <memory>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::geom2 {

  /// Scene graph node that renders a text label as a textured quad
  /** Text is rendered to an RGBA image via QPainter, then displayed as a
      textured quad. In billboard mode (default), the quad always faces the
      camera.

      The quad's world-space height is set by billboardHeight. Width is
      computed from the text aspect ratio. Non-billboard mode places a
      fixed quad in local coordinates.

      Usage:
      \code
      auto label = TextNode::create("Hello", 30, GeomColor(255,255,255,255));
      label->translate(100, 50, 80);
      scene.addNode(label);
      \endcode */
  class ICLGeom2_API TextNode : public GeometryNode {
  public:
    /// Create a text node
    /** @param text     the text string to display
        @param height   world-space height of the text quad
        @param color    text color (background is transparent)
        @param fontSize font size in pixels for rendering quality */
    TextNode(const std::string &text, float height = 30.0f,
             const GeomColor &color = GeomColor(255, 255, 255, 255),
             int fontSize = 32);

    ~TextNode() override;
    TextNode(const TextNode &other);
    TextNode &operator=(const TextNode &other);
    TextNode(TextNode &&other) noexcept;
    TextNode &operator=(TextNode &&other) noexcept;

    Node *deepCopy() const override;

    /// Update the displayed text
    void setText(const std::string &text);
    const std::string &getText() const;

    /// Set billboard height (0 = non-billboard, fixed quad)
    void setBillboardHeight(float h);
    float getBillboardHeight() const;

    /// Set text color (re-renders the texture)
    void setTextColor(const GeomColor &color);

    /// Billboard mode: call in prepareForRendering with the inverse view rotation
    void prepareForRendering() override;

    /// Enable/disable billboard mode
    void setBillboard(bool enabled);
    bool isBillboard() const;

    /// Factory
    static std::shared_ptr<TextNode> create(
        const std::string &text, float height = 30.0f,
        const GeomColor &color = GeomColor(255, 255, 255, 255),
        int fontSize = 32);

  private:
    void regenerateTexture();
    void rebuildQuad();

    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::geom2
