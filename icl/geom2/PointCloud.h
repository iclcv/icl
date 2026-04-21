// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Size.h>
#include <icl/core/DataSegment.h>
#include <icl/core/Types.h>
#include <icl/math/FixedVector.h>
#include <icl/geom2/Primitive.h>  // for GeomColor
#include <memory>
#include <mutex>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::geom2 {

  using Vec = math::FixedColVector<float, 4>;

  /// Point cloud data container — independent of the scene graph
  /** Stores point positions, normals, colors, labels, depth, and intensity
      as flat vectors with DataSegment<T,N> views for strided zero-copy access.

      Can be organized (2D grid from depth cameras) or unorganized (1D).
      Thread-safe via lock()/unlock(). Designed to be shared between
      processing and rendering threads via shared_ptr.

      Positions are stored in XYZH format (4 floats, H=1). selectXYZ()
      returns a 3-float view with stride=sizeof(Vec), selectXYZH() returns
      the full 4 floats. Normals are stored as (nx,ny,nz,curvature).
      Colors are stored as float RGBA in [0,255]. */
  class ICLGeom2_API PointCloud {
  public:
    /// Feature flags (bitmask)
    enum Feature {
      XYZ       = 1 << 0,  ///< 3D position (always present after construction with points)
      Normal    = 1 << 1,  ///< Surface normal (nx,ny,nz,curvature)
      RGBA32f   = 1 << 2,  ///< Per-point color (float RGBA)
      Label     = 1 << 3,  ///< Integer label per point
      Depth     = 1 << 4,  ///< Scalar depth value
      Intensity = 1 << 5   ///< Scalar intensity value
    };

    /// Empty point cloud (zero points, no features)
    PointCloud();

    /// Unorganized point cloud with numPoints entries
    explicit PointCloud(int numPoints, int features = XYZ);

    /// Organized (2D) point cloud with width x height entries
    PointCloud(int width, int height, int features = XYZ);

    ~PointCloud();

    // Rule of 5 — deep copy, efficient move
    PointCloud(const PointCloud &other);
    PointCloud &operator=(const PointCloud &other);
    PointCloud(PointCloud &&other) noexcept;
    PointCloud &operator=(PointCloud &&other) noexcept;

    // --- Size ---
    bool isOrganized() const;
    utils::Size getSize() const;  ///< (width x height) if organized, (dim x 1) otherwise
    int getDim() const;           ///< Total point count
    void setSize(const utils::Size &size);  ///< Resize as organized (2D)
    void resize(int numPoints);             ///< Resize as unorganized (1D)

    // --- Features ---
    bool supports(Feature f) const;
    int getFeatures() const;      ///< Bitmask of enabled features
    void addFeature(Feature f);   ///< Enable feature, allocating storage

    // --- Typed channel access (DataSegment views, non-owning) ---
    core::DataSegment<float,3> selectXYZ();
    core::DataSegment<float,4> selectXYZH();
    core::DataSegment<float,4> selectNormal();
    core::DataSegment<float,4> selectRGBA32f();
    core::DataSegment<icl32s,1> selectLabel();
    core::DataSegment<float,1> selectDepth();
    core::DataSegment<float,1> selectIntensity();

    // Const overloads
    core::DataSegment<float,3> selectXYZ() const;
    core::DataSegment<float,4> selectXYZH() const;
    core::DataSegment<float,4> selectNormal() const;
    core::DataSegment<float,4> selectRGBA32f() const;
    core::DataSegment<icl32s,1> selectLabel() const;
    core::DataSegment<float,1> selectDepth() const;
    core::DataSegment<float,1> selectIntensity() const;

    // --- Raw data pointers (for efficient GPU upload) ---
    const Vec *getPositionData() const;       ///< XYZH array, null if empty
    const Vec *getNormalData() const;         ///< null if no normals
    const GeomColor *getColorData() const;    ///< null if no colors

    // --- Locking ---
    void lock() const;
    void unlock() const;
    std::recursive_mutex &getMutex() const;

    // --- Copy ---
    PointCloud deepCopy() const;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::geom2
