// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/GeometryNode.h>
#include <icl/core/Img.h>
#include <icl/geom/Material.h>
#include <cmath>
#include <atomic>

namespace icl::viz3d {

  // Geometry versions are drawn from a single PROCESS-WIDE monotonic counter, not
  // a per-node one. The renderer caches VAOs in a map keyed by the raw node
  // pointer; when a node is destroyed and a new one is allocated at the same
  // address (very common for the plot's per-retic tick labels), a per-node
  // counter would restart and could match the stale cache entry's value —
  // serving the dead node's geometry. A global counter never repeats, so a
  // reused address always looks "changed" and rebuilds. Same rationale as
  // Material::TextureMaps::version.
  static uint64_t nextGeometryVersion() {
    static std::atomic<uint64_t> gen{1};
    return gen.fetch_add(1, std::memory_order_relaxed);
  }

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
    bool renderOnTop = false;

    // Set from the global counter on every geometry mutation; the renderer
    // compares it against its per-node cached value to re-upload only changed
    // nodes (globally unique, so pointer reuse can't alias a stale cache entry).
    uint64_t geometryVersion = nextGeometryVersion();
  };

  GeometryNode::GeometryNode() : m_data(std::make_unique<Data>()) {}
  GeometryNode::~GeometryNode() = default;

  GeometryNode::GeometryNode(const GeometryNode &other)
      : Node(other), m_data(std::make_unique<Data>(*other.m_data)) {
    if (m_data->material) m_data->material = m_data->material->deepCopy();
    m_data->geometryVersion = nextGeometryVersion();  // don't inherit source's stamp
  }

  GeometryNode &GeometryNode::operator=(const GeometryNode &other) {
    if (this != &other) {
      Node::operator=(other);
      m_data = std::make_unique<Data>(*other.m_data);
      if (m_data->material) m_data->material = m_data->material->deepCopy();
      m_data->geometryVersion = nextGeometryVersion();
    }
    return *this;
  }

  GeometryNode::GeometryNode(GeometryNode &&other) noexcept = default;
  GeometryNode &GeometryNode::operator=(GeometryNode &&other) noexcept = default;

  // Read-only accessors
  const std::vector<Vec> &GeometryNode::getVertices() const { return m_data->vertices; }
  const std::vector<Vec> &GeometryNode::getNormals() const { return m_data->normals; }
  const std::vector<GeomColor> &GeometryNode::getVertexColors() const { return m_data->vertexColors; }
  const std::vector<utils::Point32f> &GeometryNode::getTexCoords() const { return m_data->texCoords; }
  const std::vector<LinePrimitive> &GeometryNode::getLines() const { return m_data->lines; }
  const std::vector<TrianglePrimitive> &GeometryNode::getTriangles() const { return m_data->triangles; }
  const std::vector<QuadPrimitive> &GeometryNode::getQuads() const { return m_data->quads; }

  // Dirty tracking
  uint64_t GeometryNode::getGeometryVersion() const { return m_data->geometryVersion; }
  void GeometryNode::markGeometryDirty() { m_data->geometryVersion = nextGeometryVersion(); }

  // Protected mutable accessors — every handout bumps the version, since the
  // caller is about to mutate (the renderer reads through the const getters).
  std::vector<Vec> &GeometryNode::vertices() { m_data->geometryVersion = nextGeometryVersion(); return m_data->vertices; }
  std::vector<Vec> &GeometryNode::normals() { m_data->geometryVersion = nextGeometryVersion(); return m_data->normals; }
  std::vector<GeomColor> &GeometryNode::vertexColors() { m_data->geometryVersion = nextGeometryVersion(); return m_data->vertexColors; }
  std::vector<utils::Point32f> &GeometryNode::texCoords() { m_data->geometryVersion = nextGeometryVersion(); return m_data->texCoords; }
  std::vector<LinePrimitive> &GeometryNode::lines() { m_data->geometryVersion = nextGeometryVersion(); return m_data->lines; }
  std::vector<TrianglePrimitive> &GeometryNode::triangles() { m_data->geometryVersion = nextGeometryVersion(); return m_data->triangles; }
  std::vector<QuadPrimitive> &GeometryNode::quads() { m_data->geometryVersion = nextGeometryVersion(); return m_data->quads; }

  void GeometryNode::clearGeometryData() {
    m_data->vertices.clear();
    m_data->normals.clear();
    m_data->vertexColors.clear();
    m_data->texCoords.clear();
    m_data->lines.clear();
    m_data->triangles.clear();
    m_data->quads.clear();
    m_data->geometryVersion = nextGeometryVersion();
  }

  // Material
  void GeometryNode::setMaterial(std::shared_ptr<geom::Material> mat) { m_data->material = std::move(mat); }
  std::shared_ptr<geom::Material> GeometryNode::getMaterial() const { return m_data->material; }

  // Visibility
  void GeometryNode::setPrimitiveVisible(int mask, bool visible) {
    const int before = m_data->visibleMask;
    if (visible) m_data->visibleMask |= mask;
    else m_data->visibleMask &= ~mask;
    // The renderer only builds caches for visible primitive types, so a
    // visibility change must invalidate the cache or it won't take effect.
    if (m_data->visibleMask != before) m_data->geometryVersion = nextGeometryVersion();
  }
  bool GeometryNode::isPrimitiveVisible(int type) const { return (m_data->visibleMask & type) != 0; }

  // Rendering hints
  void GeometryNode::setPointSize(float s) { m_data->pointSize = s; }
  float GeometryNode::getPointSize() const { return m_data->pointSize; }
  void GeometryNode::setLineWidth(float w) { m_data->lineWidth = w; }
  float GeometryNode::getLineWidth() const { return m_data->lineWidth; }
  void GeometryNode::setRenderOnTop(bool on) { m_data->renderOnTop = on; }
  bool GeometryNode::getRenderOnTop() const { return m_data->renderOnTop; }
  void GeometryNode::setSmoothShading(bool on) {
    if (m_data->smoothShading == on) return;
    m_data->smoothShading = on;
    m_data->geometryVersion = nextGeometryVersion();   // shading affects the uploaded normals -> rebuild
  }
  bool GeometryNode::getSmoothShading() const { return m_data->smoothShading; }

  void GeometryNode::createAutoNormals(bool smooth) {
    m_data->geometryVersion = nextGeometryVersion();
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

} // namespace icl::viz3d
