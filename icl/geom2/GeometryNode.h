// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Node.h>
#include <icl/geom2/Primitive.h>
#include <icl/math/FixedVector.h>
#include <icl/utils/Point32f.h>
#include <vector>
#include <memory>

namespace icl::geom {
  class Material;
}

namespace icl::geom2 {

  using Vec = math::FixedColVector<float, 4>;

  /// Abstract geometry node — read-only geometry access for renderers
  /** GeometryNode owns vertices, normals, primitives, and material.
      Renderers access geometry through const getters.
      Subclasses use protected mutable access:
      - MeshNode exposes it publicly (for freeform/dynamic meshes)
      - Parametric nodes (SphereNode etc.) use it internally in generateMesh() */
  class ICLGeom2_API GeometryNode : public Node {
  public:
    ~GeometryNode() override;

    // --- Read-only geometry access (for renderers) ---
    const std::vector<Vec> &getVertices() const;
    const std::vector<Vec> &getNormals() const;
    const std::vector<GeomColor> &getVertexColors() const;
    const std::vector<utils::Point32f> &getTexCoords() const;

    const std::vector<LinePrimitive> &getLines() const;
    const std::vector<TrianglePrimitive> &getTriangles() const;
    const std::vector<QuadPrimitive> &getQuads() const;

    // --- Material ---
    void setMaterial(std::shared_ptr<geom::Material> mat);
    std::shared_ptr<geom::Material> getMaterial() const;

    // --- Primitive-type visibility ---
    void setPrimitiveVisible(int typeMask, bool visible);
    bool isPrimitiveVisible(int primitiveType) const;

    // --- Rendering hints ---
    void setPointSize(float size);
    float getPointSize() const;
    void setLineWidth(float width);
    float getLineWidth() const;
    void setSmoothShading(bool on);
    bool getSmoothShading() const;

    // --- Normals ---
    void createAutoNormals(bool smooth = true);

  protected:
    GeometryNode();
    GeometryNode(const GeometryNode &other);
    GeometryNode &operator=(const GeometryNode &other);
    GeometryNode(GeometryNode &&other) noexcept;
    GeometryNode &operator=(GeometryNode &&other) noexcept;

    // Mutable access for subclasses
    std::vector<Vec> &vertices();
    std::vector<Vec> &normals();
    std::vector<GeomColor> &vertexColors();
    std::vector<utils::Point32f> &texCoords();
    std::vector<LinePrimitive> &lines();
    std::vector<TrianglePrimitive> &triangles();
    std::vector<QuadPrimitive> &quads();
    void clearGeometryData();

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::geom2
