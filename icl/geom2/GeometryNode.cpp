// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/GeometryNode.h>
#include <icl/core/Img.h>
#include <icl/geom/Material.h>
#include <cmath>

namespace icl::geom2 {

  struct GeometryNode::Data {
    std::vector<Vec> vertices;
    std::vector<Vec> normals;
    std::vector<GeomColor> vertexColors;
    std::vector<utils::Point32f> texCoords;

    std::vector<LinePrimitive> lines;
    std::vector<TrianglePrimitive> triangles;
    std::vector<QuadPrimitive> quads;

    std::shared_ptr<geom::Material> material;

    int visibleMask = PrimAll;
    float pointSize = 3.0f;
    float lineWidth = 1.0f;
    bool smoothShading = true;
  };

  GeometryNode::GeometryNode() : m_data(new Data) {}
  GeometryNode::~GeometryNode() { delete m_data; }

  GeometryNode::GeometryNode(const GeometryNode &other) : SceneNode(other), m_data(new Data(*other.m_data)) {
    if (m_data->material) m_data->material = m_data->material->deepCopy();
  }

  GeometryNode &GeometryNode::operator=(const GeometryNode &other) {
    if (this != &other) {
      SceneNode::operator=(other);
      auto *old = m_data;
      m_data = new Data(*other.m_data);
      if (m_data->material) m_data->material = m_data->material->deepCopy();
      delete old;
    }
    return *this;
  }

  GeometryNode::GeometryNode(GeometryNode &&other) noexcept
      : SceneNode(std::move(other)), m_data(other.m_data) {
    other.m_data = nullptr;
  }

  GeometryNode &GeometryNode::operator=(GeometryNode &&other) noexcept {
    if (this != &other) {
      SceneNode::operator=(std::move(other));
      delete m_data;
      m_data = other.m_data;
      other.m_data = nullptr;
    }
    return *this;
  }

  // Read-only accessors
  const std::vector<Vec> &GeometryNode::getVertices() const { return m_data->vertices; }
  const std::vector<Vec> &GeometryNode::getNormals() const { return m_data->normals; }
  const std::vector<GeomColor> &GeometryNode::getVertexColors() const { return m_data->vertexColors; }
  const std::vector<utils::Point32f> &GeometryNode::getTexCoords() const { return m_data->texCoords; }
  const std::vector<LinePrimitive> &GeometryNode::getLines() const { return m_data->lines; }
  const std::vector<TrianglePrimitive> &GeometryNode::getTriangles() const { return m_data->triangles; }
  const std::vector<QuadPrimitive> &GeometryNode::getQuads() const { return m_data->quads; }

  // Protected mutable accessors
  std::vector<Vec> &GeometryNode::vertices() { return m_data->vertices; }
  std::vector<Vec> &GeometryNode::normals() { return m_data->normals; }
  std::vector<GeomColor> &GeometryNode::vertexColors() { return m_data->vertexColors; }
  std::vector<utils::Point32f> &GeometryNode::texCoords() { return m_data->texCoords; }
  std::vector<LinePrimitive> &GeometryNode::lines() { return m_data->lines; }
  std::vector<TrianglePrimitive> &GeometryNode::triangles() { return m_data->triangles; }
  std::vector<QuadPrimitive> &GeometryNode::quads() { return m_data->quads; }

  void GeometryNode::clearGeometryData() {
    m_data->vertices.clear();
    m_data->normals.clear();
    m_data->vertexColors.clear();
    m_data->texCoords.clear();
    m_data->lines.clear();
    m_data->triangles.clear();
    m_data->quads.clear();
  }

  // Material
  void GeometryNode::setMaterial(std::shared_ptr<geom::Material> mat) { m_data->material = std::move(mat); }
  std::shared_ptr<geom::Material> GeometryNode::getMaterial() const { return m_data->material; }

  // Visibility
  void GeometryNode::setPrimitiveVisible(int mask, bool visible) {
    if (visible) m_data->visibleMask |= mask;
    else m_data->visibleMask &= ~mask;
  }
  bool GeometryNode::isPrimitiveVisible(int type) const { return (m_data->visibleMask & type) != 0; }

  // Rendering hints
  void GeometryNode::setPointSize(float s) { m_data->pointSize = s; }
  float GeometryNode::getPointSize() const { return m_data->pointSize; }
  void GeometryNode::setLineWidth(float w) { m_data->lineWidth = w; }
  float GeometryNode::getLineWidth() const { return m_data->lineWidth; }
  void GeometryNode::setSmoothShading(bool on) { m_data->smoothShading = on; }
  bool GeometryNode::getSmoothShading() const { return m_data->smoothShading; }

  void GeometryNode::createAutoNormals(bool smooth) {
    auto &V = m_data->vertices;
    auto &N = m_data->normals;
    N.resize(V.size(), Vec(0, 0, 0, 0));

    auto addFaceNormal = [&](int a, int b, int c) {
      Vec e1 = V[b] - V[a], e2 = V[c] - V[a];
      Vec n(e1[1]*e2[2] - e1[2]*e2[1],
            e1[2]*e2[0] - e1[0]*e2[2],
            e1[0]*e2[1] - e1[1]*e2[0], 0);
      if (smooth) {
        N[a] = N[a] + n; N[b] = N[b] + n; N[c] = N[c] + n;
      } else {
        N[a] = n; N[b] = n; N[c] = n;
      }
    };

    for (const auto &tri : m_data->triangles) addFaceNormal(tri.v[0], tri.v[1], tri.v[2]);
    for (const auto &q : m_data->quads) {
      addFaceNormal(q.v[0], q.v[1], q.v[2]);
      addFaceNormal(q.v[0], q.v[2], q.v[3]);
    }

    if (smooth) {
      for (auto &n : N) {
        float len = std::sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
        if (len > 1e-8f) { n[0] /= len; n[1] /= len; n[2] /= len; }
      }
    }
  }

} // namespace icl::geom2
