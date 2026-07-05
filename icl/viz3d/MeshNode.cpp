// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/MeshNode.h>
#include <icl/viz3d/Loader.h>
#include <icl/viz3d/Material.h>

namespace icl::viz3d {

  static const float COLOR_SCALE = 1.0f / 255.0f;

  std::shared_ptr<MeshNode> MeshNode::createTexturedQuad(float w, float h,
                                                         const core::Image &texture) {
    auto m = std::make_shared<MeshNode>();
    const float hw = w * 0.5f, hh = h * 0.5f;
    m->addVertex(Vec(-hw,  hh, 0, 1));   // 0 top-left
    m->addVertex(Vec( hw,  hh, 0, 1));   // 1 top-right
    m->addVertex(Vec( hw, -hh, 0, 1));   // 2 bottom-right
    m->addVertex(Vec(-hw, -hh, 0, 1));   // 3 bottom-left
    m->addTexCoord(0, 0); m->addTexCoord(1, 0);
    m->addTexCoord(1, 1); m->addTexCoord(0, 1);
    m->addNormal(Vec(0, 0, 1, 0));
    m->addQuad(0, 1, 2, 3, /*normals*/ 0, 0, 0, 0, /*texcoords*/ 0, 1, 2, 3);
    m->setMaterial(viz3d::Material::fromTexture(texture));
    return m;
  }

  MeshNode::MeshNode() = default;
  MeshNode::~MeshNode() = default;
  MeshNode::MeshNode(const MeshNode &other) = default;
  MeshNode &MeshNode::operator=(const MeshNode &other) = default;
  MeshNode::MeshNode(MeshNode &&other) noexcept = default;
  MeshNode &MeshNode::operator=(MeshNode &&other) noexcept = default;

  NodePtr MeshNode::deepCopy() const { return std::make_shared<MeshNode>(*this); }

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

  void MeshNode::ingest(MeshData data) {
    clearGeometryData();

    if (data.vertices) vertices() = std::move(*data.vertices);
    if (data.normals)  normals() = std::move(*data.normals);
    if (data.uvs)      texCoords() = std::move(*data.uvs);
    if (data.colors)   vertexColors() = std::move(*data.colors);
    if (data.triangles) triangles() = std::move(*data.triangles);
    if (data.quads)    quads() = std::move(*data.quads);
    if (data.lines)    lines() = std::move(*data.lines);

    // Auto-generate normals if geometry present but normals absent
    if (!data.normals && !vertices().empty()
        && (!triangles().empty() || !quads().empty())) {
      createAutoNormals(true);
    }
  }

  void MeshNode::clearGeometry() {
    clearGeometryData();
  }

  std::vector<std::shared_ptr<MeshNode>> MeshNode::load(const std::string &filename) {
    return loadFile(filename);
  }

} // namespace icl::viz3d
