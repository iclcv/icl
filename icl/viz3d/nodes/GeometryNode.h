// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/nodes/Node.h>
#include <icl/viz3d/render/Primitive.h>
#include <icl/math/la/FixedVector.h>
#include <icl/utils/Point.h>
#include <vector>
#include <memory>
#include <cstdint>

namespace icl::viz3d { class Material; }

namespace icl::viz3d {

  using Vec = math::FixedColVector<float, 4>;

  /// Abstract geometry node — read-only geometry access for renderers
  /** GeometryNode owns vertices, normals, primitives, and material.
      Renderers access geometry through const getters.
      Subclasses use protected mutable access:
      - MeshNode exposes it publicly (for freeform/dynamic meshes)
      - Parametric nodes (SphereNode etc.) use it internally in generateMesh() */
  class ICLViz3d_API GeometryNode : public Node {
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
    void setMaterial(std::shared_ptr<viz3d::Material> mat);
    std::shared_ptr<viz3d::Material> getMaterial() const;

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

    /// Render lines/points with depth-test disabled (always-on-top overlay).
    /** Useful for debug/annotation geometry (e.g. collision wireframes) that
        would otherwise z-fight with / be occluded by the solid surfaces it
        traces. Default false. */
    void setRenderOnTop(bool on);
    bool getRenderOnTop() const;

    // --- Normals ---
    void createAutoNormals(bool smooth = true);

    // --- Dirty tracking (per-node renderer cache invalidation) ---
    /// Monotonic counter bumped on every geometry mutation. The renderer
    /// caches GL buffers per node and re-uploads only when this changes —
    /// so a single dynamic mesh (soft body, cloth) no longer forces a
    /// full-scene rebuild via Renderer::invalidateCache().
    uint64_t getGeometryVersion() const;
    /// Force a version bump. Mutations through the sanctioned mutable API
    /// (the protected accessors, clearGeometryData, createAutoNormals, and
    /// MeshNode's builders) bump automatically; call this after editing
    /// geometry in place via a non-const accessor such as
    /// MeshNode::getVertices().
    void markGeometryDirty();

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

} // namespace icl::viz3d
