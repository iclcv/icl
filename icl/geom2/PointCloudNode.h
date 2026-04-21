// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Node.h>
#include <memory>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::geom {
  class Material;
}

namespace icl::geom2 {

  class PointCloud;

  /// Scene graph node that wraps a shared PointCloud for rendering
  /** PointCloudNode holds a shared_ptr<PointCloud> and renders it as
      GL_POINTS via a dedicated path in the Renderer. The point cloud
      data lives outside the node and can be shared, swapped, or
      updated independently.

      Locking: delegates to the PointCloud's own mutex. Lock the
      PointCloud before writing, the renderer locks before reading.

      Does NOT inherit GeometryNode — point clouds are fundamentally
      different from mesh geometry (massive, updated every frame,
      rendered as points not triangles). */
  class ICLGeom2_API PointCloudNode : public Node {
  public:
    PointCloudNode();
    explicit PointCloudNode(std::shared_ptr<PointCloud> cloud);
    ~PointCloudNode() override;

    // Rule of 5
    PointCloudNode(const PointCloudNode &other);
    PointCloudNode &operator=(const PointCloudNode &other);
    PointCloudNode(PointCloudNode &&other) noexcept;
    PointCloudNode &operator=(PointCloudNode &&other) noexcept;

    Node *deepCopy() const override;

    /// Set/swap the underlying point cloud (shallow, atomic pointer swap)
    void setPointCloud(std::shared_ptr<PointCloud> cloud);

    /// Get the underlying point cloud
    std::shared_ptr<PointCloud> getPointCloud() const;

    // --- Rendering hints ---
    void setPointSize(float size);
    float getPointSize() const;

    // --- Material (optional, for base color / rendering style) ---
    void setMaterial(std::shared_ptr<geom::Material> mat);
    std::shared_ptr<geom::Material> getMaterial() const;

    /// Factory
    static std::shared_ptr<PointCloudNode> create(std::shared_ptr<PointCloud> cloud);

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::geom2
