// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/TextNode.h>
#include <icl/core/Img.h>
#include <icl/geom/Material.h>

#ifdef ICL_HAVE_QT
#include <QImage>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#endif

namespace icl::geom2 {

  struct TextNode::Data {
    std::string text;
    float height = 30.0f;
    GeomColor color{255, 255, 255, 255};
    int fontSize = 32;
    bool billboard = true;
    float aspect = 1.0f;   // width/height of rendered text
  };

  TextNode::TextNode(const std::string &text, float height,
                     const GeomColor &color, int fontSize)
    : m_data(std::make_unique<Data>()) {
    m_data->text = text;
    m_data->height = height;
    m_data->color = color;
    m_data->fontSize = fontSize;
    setPrimitiveVisible(PrimVertex, false);
    regenerateTexture();
    rebuildQuad();
  }

  TextNode::~TextNode() = default;

  TextNode::TextNode(const TextNode &other)
    : GeometryNode(other), m_data(std::make_unique<Data>(*other.m_data)) {
  }

  TextNode &TextNode::operator=(const TextNode &other) {
    if (this != &other) {
      GeometryNode::operator=(other);
      m_data = std::make_unique<Data>(*other.m_data);
    }
    return *this;
  }

  TextNode::TextNode(TextNode &&other) noexcept = default;
  TextNode &TextNode::operator=(TextNode &&other) noexcept = default;

  Node *TextNode::deepCopy() const { return new TextNode(*this); }

  void TextNode::setText(const std::string &text) {
    if (m_data->text != text) {
      m_data->text = text;
      regenerateTexture();
      rebuildQuad();
    }
  }

  const std::string &TextNode::getText() const { return m_data->text; }

  void TextNode::setBillboardHeight(float h) {
    m_data->height = h;
    rebuildQuad();
  }

  float TextNode::getBillboardHeight() const { return m_data->height; }

  void TextNode::setTextColor(const GeomColor &color) {
    m_data->color = color;
    regenerateTexture();
  }

  void TextNode::setBillboard(bool enabled) { m_data->billboard = enabled; }
  bool TextNode::isBillboard() const { return m_data->billboard; }

  void TextNode::prepareForRendering() {
    // Billboard orientation is handled by the Renderer (it needs the view matrix)
  }

  std::shared_ptr<TextNode> TextNode::create(
      const std::string &text, float height,
      const GeomColor &color, int fontSize) {
    return std::make_shared<TextNode>(text, height, color, fontSize);
  }

  void TextNode::regenerateTexture() {
#ifdef ICL_HAVE_QT
    if (m_data->text.empty()) return;

    QFont font("Arial", m_data->fontSize);
    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(QString::fromStdString(m_data->text)) + 10;
    int textHeight = fm.height() + 4;

    // Render text to QImage
    QImage qimg(textWidth, textHeight, QImage::Format_ARGB32);
    qimg.fill(Qt::transparent);

    QPainter painter(&qimg);
    painter.setFont(font);
    QColor qcol(
      static_cast<int>(m_data->color[0] > 1.01f ? m_data->color[0] : m_data->color[0] * 255),
      static_cast<int>(m_data->color[1] > 1.01f ? m_data->color[1] : m_data->color[1] * 255),
      static_cast<int>(m_data->color[2] > 1.01f ? m_data->color[2] : m_data->color[2] * 255),
      static_cast<int>(m_data->color[3] > 1.01f ? m_data->color[3] : m_data->color[3] * 255));
    painter.setPen(qcol);
    painter.drawText(5, fm.ascent() + 2, QString::fromStdString(m_data->text));
    painter.end();

    m_data->aspect = static_cast<float>(textWidth) / textHeight;

    // Convert QImage to ICL Image (RGBA, 8u)
    int w = qimg.width(), h = qimg.height();
    auto img = core::Img8u(utils::Size(w, h), 4);
    for (int y = 0; y < h; y++) {
      const auto *scanline = reinterpret_cast<const uint32_t*>(qimg.scanLine(y));
      for (int x = 0; x < w; x++) {
        uint32_t px = scanline[x];
        img(x, y, 0) = (px >> 16) & 0xFF;  // R
        img(x, y, 1) = (px >> 8) & 0xFF;   // G
        img(x, y, 2) = px & 0xFF;           // B
        img(x, y, 3) = (px >> 24) & 0xFF;   // A
      }
    }

    // Set as material texture
    auto mat = getMaterial();
    if (!mat) {
      mat = geom::Material::fromColor(GeomColor(255, 255, 255, 255));
      setMaterial(mat);
    }
    if (!mat->textures) {
      mat->textures = std::make_shared<geom::Material::TextureMaps>();
    }
    mat->textures->baseColorMap = core::Image(img);
#endif
  }

  void TextNode::rebuildQuad() {
    clearGeometryData();

    float h = m_data->height * 0.5f;
    float w = h * m_data->aspect;

    // Quad centered at origin — vertices in "right/up" space.
    // For billboard, the renderer replaces the model rotation with
    // the inverse view rotation, mapping X→screen right, Y→screen up.
    vertices().push_back(Vec(-w, -h, 0, 1));  // bottom-left
    vertices().push_back(Vec( w, -h, 0, 1));  // bottom-right
    vertices().push_back(Vec( w,  h, 0, 1));  // top-right
    vertices().push_back(Vec(-w,  h, 0, 1));  // top-left

    normals().push_back(Vec(0, 0, 1, 0));
    normals().push_back(Vec(0, 0, 1, 0));
    normals().push_back(Vec(0, 0, 1, 0));
    normals().push_back(Vec(0, 0, 1, 0));

    texCoords().push_back(utils::Point32f(1, 1));  // bottom-left
    texCoords().push_back(utils::Point32f(0, 1));  // bottom-right
    texCoords().push_back(utils::Point32f(0, 0));  // top-right
    texCoords().push_back(utils::Point32f(1, 0));  // top-left

    // Two triangles for the quad
    triangles().push_back(TrianglePrimitive{
      {0, 1, 2}, {0, 1, 2}, {0, 1, 2}});
    triangles().push_back(TrianglePrimitive{
      {0, 2, 3}, {0, 2, 3}, {0, 2, 3}});
  }

} // namespace icl::geom2
