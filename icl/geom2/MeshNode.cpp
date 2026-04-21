// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/MeshNode.h>

namespace icl::geom2 {

  static const float COLOR_SCALE = 1.0f / 255.0f;

  MeshNode::MeshNode() = default;
  MeshNode::~MeshNode() = default;
  MeshNode::MeshNode(const MeshNode &other) = default;
  MeshNode &MeshNode::operator=(const MeshNode &other) = default;
  MeshNode::MeshNode(MeshNode &&other) noexcept = default;
  MeshNode &MeshNode::operator=(MeshNode &&other) noexcept = default;

  SceneNode *MeshNode::deepCopy() const { return new MeshNode(*this); }

  // Mutable access (delegates to protected GeometryNode accessors)
  std::vector<Vec> &MeshNode::getVertices() { return vertices(); }
  std::vector<GeomColor> &MeshNode::getVertexColors() { return vertexColors(); }

  // Building
  void MeshNode::addVertex(const Vec &pos, const GeomColor &color) {
    vertices().push_back(pos);
    vertexColors().push_back(color * COLOR_SCALE);
  }

  void MeshNode::addNormal(const Vec &n) {
    normals().push_back(n);
  }

  void MeshNode::addTexCoord(float u, float v) {
    texCoords().push_back(utils::Point32f(u, v));
  }

  void MeshNode::addLine(int a, int b, const GeomColor &color) {
    lines().push_back({a, b, color * COLOR_SCALE});
  }

  void MeshNode::addTriangle(int a, int b, int c,
                              int na, int nb, int nc,
                              int ta, int tb, int tc) {
    triangles().push_back({{a, b, c}, {na, nb, nc}, {ta, tb, tc}});
  }

  void MeshNode::addQuad(int a, int b, int c, int d,
                          int na, int nb, int nc, int nd,
                          int ta, int tb, int tc, int td) {
    quads().push_back({{a, b, c, d}, {na, nb, nc, nd}, {ta, tb, tc, td}});
  }

  void MeshNode::clearGeometry() {
    clearGeometryData();
  }

  std::vector<std::shared_ptr<MeshNode>> MeshNode::load(const std::string &filename) {
    // TODO: implement file loading (.obj, .glb, .gltf)
    (void)filename;
    return {};
  }

} // namespace icl::geom2
