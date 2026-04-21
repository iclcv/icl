// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/GeometryNode.h>
#include <optional>

namespace icl::geom2 {

  /// Bulk geometry data for MeshNode::ingest()
  /** Use designated initializers + std::move for zero-copy ingestion:
      @code
      ingest({
          .vertices  = std::move(verts),
          .normals   = std::move(norms),
          .triangles = std::move(tris),
          // uvs, colors absent → left empty
      });
      @endcode */
  struct MeshData {
    std::optional<std::vector<Vec>> vertices;
    std::optional<std::vector<Vec>> normals;
    std::optional<std::vector<utils::Point32f>> uvs;
    std::optional<std::vector<GeomColor>> colors;
    std::optional<std::vector<TrianglePrimitive>> triangles;
    std::optional<std::vector<QuadPrimitive>> quads;
    std::optional<std::vector<LinePrimitive>> lines;
  };

  /// Mutable geometry leaf — for freeform meshes, physics, dynamic geometry
  /** MeshNode adds public mutable access to GeometryNode's geometry:
      - getVertices() returns non-const reference (for physics vertex updates)
      - addVertex/addTriangle/addQuad/addLine for building geometry
      - clearGeometry() to reset

      For parametric shapes (sphere, cube, etc.), use SphereNode/CuboidNode
      which inherit GeometryNode directly and don't expose mutable vertices. */
  class ICLGeom2_API MeshNode : public GeometryNode {
  public:
    MeshNode();
    ~MeshNode() override;
    MeshNode(const MeshNode &other);
    MeshNode &operator=(const MeshNode &other);
    MeshNode(MeshNode &&other) noexcept;
    MeshNode &operator=(MeshNode &&other) noexcept;
    Node *deepCopy() const override;

    // --- Mutable geometry access (for physics, dynamic meshes) ---
    std::vector<Vec> &getVertices();
    std::vector<GeomColor> &getVertexColors();

    // --- Geometry building ---
    void addVertex(const Vec &pos,
                   const GeomColor &color = GeomColor(255, 0, 0, 255));
    void addNormal(const Vec &n);
    void addTexCoord(float u, float v);

    void addLine(int a, int b,
                 const GeomColor &color = GeomColor(100, 100, 100, 255));
    void addTriangle(int a, int b, int c,
                     int na = -1, int nb = -1, int nc = -1,
                     int ta = -1, int tb = -1, int tc = -1);
    void addQuad(int a, int b, int c, int d,
                 int na = -1, int nb = -1, int nc = -1, int nd = -1,
                 int ta = -1, int tb = -1, int tc = -1, int td = -1);

    /// Bulk-ingest geometry data, moving vectors in without copying
    /** Clears existing geometry, then moves present fields into the node.
        Absent (nullopt) fields are left empty. Auto-generates normals
        if vertices and triangles/quads are present but normals are not. */
    void ingest(MeshData data);

    /// Clears all geometry (vertices, normals, colors, primitives)
    void clearGeometry();

    /// Load mesh(es) from file (.obj, .glb, .gltf)
    static std::vector<std::shared_ptr<MeshNode>> load(const std::string &filename);
  };

} // namespace icl::geom2
