// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/PointCloud.h>
#include <icl/utils/Exception.h>
#include <vector>

namespace icl::geom2 {

  struct PointCloud::Data {
    std::vector<Vec> positions;      // XYZH (4 floats, H=1)
    std::vector<Vec> normals;        // (nx,ny,nz,curvature)
    std::vector<GeomColor> colors;   // RGBA float
    std::vector<icl32s> labels;
    std::vector<float> depth;
    std::vector<float> intensity;

    int features = 0;
    bool organized = false;
    int width = 0;
    int height = 0;

    mutable std::recursive_mutex mutex;

    void resizeAll(int n) {
      positions.resize(n, Vec(0, 0, 0, 1));
      if (features & Normal)    normals.resize(n, Vec(0, 0, 0, 0));
      if (features & RGBA32f)   colors.resize(n, GeomColor(0, 0, 0, 0));
      if (features & Label)     labels.resize(n, 0);
      if (features & Depth)     depth.resize(n, 0.0f);
      if (features & Intensity) intensity.resize(n, 0.0f);
    }
  };

  PointCloud::PointCloud() : m_data(std::make_unique<Data>()) {}

  PointCloud::PointCloud(int numPoints, int features)
    : m_data(std::make_unique<Data>()) {
    m_data->features = features | XYZ;
    m_data->organized = false;
    m_data->width = numPoints;
    m_data->height = 1;
    m_data->resizeAll(numPoints);
  }

  PointCloud::PointCloud(int width, int height, int features)
    : m_data(std::make_unique<Data>()) {
    m_data->features = features | XYZ;
    m_data->organized = true;
    m_data->width = width;
    m_data->height = height;
    m_data->resizeAll(width * height);
  }

  PointCloud::~PointCloud() = default;

  PointCloud::PointCloud(const PointCloud &other)
    : m_data(std::make_unique<Data>()) {
    std::lock_guard<std::recursive_mutex> g(other.m_data->mutex);
    m_data->positions = other.m_data->positions;
    m_data->normals = other.m_data->normals;
    m_data->colors = other.m_data->colors;
    m_data->labels = other.m_data->labels;
    m_data->depth = other.m_data->depth;
    m_data->intensity = other.m_data->intensity;
    m_data->features = other.m_data->features;
    m_data->organized = other.m_data->organized;
    m_data->width = other.m_data->width;
    m_data->height = other.m_data->height;
  }

  PointCloud &PointCloud::operator=(const PointCloud &other) {
    if (this != &other) {
      std::scoped_lock lock(m_data->mutex, other.m_data->mutex);
      m_data->positions = other.m_data->positions;
      m_data->normals = other.m_data->normals;
      m_data->colors = other.m_data->colors;
      m_data->labels = other.m_data->labels;
      m_data->depth = other.m_data->depth;
      m_data->intensity = other.m_data->intensity;
      m_data->features = other.m_data->features;
      m_data->organized = other.m_data->organized;
      m_data->width = other.m_data->width;
      m_data->height = other.m_data->height;
    }
    return *this;
  }

  PointCloud::PointCloud(PointCloud &&other) noexcept = default;
  PointCloud &PointCloud::operator=(PointCloud &&other) noexcept = default;

  // --- Size ---

  bool PointCloud::isOrganized() const { return m_data->organized; }

  utils::Size PointCloud::getSize() const {
    return utils::Size(m_data->width, m_data->height);
  }

  int PointCloud::getDim() const {
    return static_cast<int>(m_data->positions.size());
  }

  void PointCloud::setSize(const utils::Size &size) {
    m_data->organized = true;
    m_data->width = size.width;
    m_data->height = size.height;
    m_data->resizeAll(size.width * size.height);
  }

  void PointCloud::resize(int numPoints) {
    m_data->organized = false;
    m_data->width = numPoints;
    m_data->height = 1;
    m_data->resizeAll(numPoints);
  }

  // --- Features ---

  bool PointCloud::supports(Feature f) const {
    return (m_data->features & f) != 0;
  }

  int PointCloud::getFeatures() const { return m_data->features; }

  void PointCloud::addFeature(Feature f) {
    if (m_data->features & f) return;
    m_data->features |= f;
    int n = getDim();
    if (n == 0) return;
    switch (f) {
      case Normal:    m_data->normals.resize(n, Vec(0, 0, 0, 0)); break;
      case RGBA32f:   m_data->colors.resize(n, GeomColor(0, 0, 0, 0)); break;
      case Label:     m_data->labels.resize(n, 0); break;
      case Depth:     m_data->depth.resize(n, 0.0f); break;
      case Intensity: m_data->intensity.resize(n, 0.0f); break;
      default: break;
    }
  }

  // --- DataSegment accessors ---

  // Helper: organized width for DataSegment (-1 = linear)
  #define PC_ORG_W (m_data->organized ? m_data->width : -1)

  core::DataSegment<float,3> PointCloud::selectXYZ() {
    return { &m_data->positions[0][0], sizeof(Vec),
             m_data->positions.size(), PC_ORG_W };
  }
  core::DataSegment<float,4> PointCloud::selectXYZH() {
    return { &m_data->positions[0][0], sizeof(Vec),
             m_data->positions.size(), PC_ORG_W };
  }
  core::DataSegment<float,4> PointCloud::selectNormal() {
    ICLASSERT_THROW(m_data->features & Normal,
      utils::ICLException("PointCloud: Normal feature not enabled"));
    return { &m_data->normals[0][0], sizeof(Vec),
             m_data->normals.size(), PC_ORG_W };
  }
  core::DataSegment<float,4> PointCloud::selectRGBA32f() {
    ICLASSERT_THROW(m_data->features & RGBA32f,
      utils::ICLException("PointCloud: RGBA32f feature not enabled"));
    return { &m_data->colors[0][0], sizeof(GeomColor),
             m_data->colors.size(), PC_ORG_W };
  }
  core::DataSegment<icl32s,1> PointCloud::selectLabel() {
    ICLASSERT_THROW(m_data->features & Label,
      utils::ICLException("PointCloud: Label feature not enabled"));
    return { m_data->labels.data(), sizeof(icl32s),
             m_data->labels.size(), PC_ORG_W };
  }
  core::DataSegment<float,1> PointCloud::selectDepth() {
    ICLASSERT_THROW(m_data->features & Depth,
      utils::ICLException("PointCloud: Depth feature not enabled"));
    return { m_data->depth.data(), sizeof(float),
             m_data->depth.size(), PC_ORG_W };
  }
  core::DataSegment<float,1> PointCloud::selectIntensity() {
    ICLASSERT_THROW(m_data->features & Intensity,
      utils::ICLException("PointCloud: Intensity feature not enabled"));
    return { m_data->intensity.data(), sizeof(float),
             m_data->intensity.size(), PC_ORG_W };
  }

  // Const overloads (same logic, const_cast internally — DataSegment is non-owning)
  core::DataSegment<float,3> PointCloud::selectXYZ() const {
    return const_cast<PointCloud*>(this)->selectXYZ();
  }
  core::DataSegment<float,4> PointCloud::selectXYZH() const {
    return const_cast<PointCloud*>(this)->selectXYZH();
  }
  core::DataSegment<float,4> PointCloud::selectNormal() const {
    return const_cast<PointCloud*>(this)->selectNormal();
  }
  core::DataSegment<float,4> PointCloud::selectRGBA32f() const {
    return const_cast<PointCloud*>(this)->selectRGBA32f();
  }
  core::DataSegment<icl32s,1> PointCloud::selectLabel() const {
    return const_cast<PointCloud*>(this)->selectLabel();
  }
  core::DataSegment<float,1> PointCloud::selectDepth() const {
    return const_cast<PointCloud*>(this)->selectDepth();
  }
  core::DataSegment<float,1> PointCloud::selectIntensity() const {
    return const_cast<PointCloud*>(this)->selectIntensity();
  }

  #undef PC_ORG_W

  // --- Raw data pointers ---

  const Vec *PointCloud::getPositionData() const {
    return m_data->positions.empty() ? nullptr : m_data->positions.data();
  }
  const Vec *PointCloud::getNormalData() const {
    return m_data->normals.empty() ? nullptr : m_data->normals.data();
  }
  const GeomColor *PointCloud::getColorData() const {
    return m_data->colors.empty() ? nullptr : m_data->colors.data();
  }

  // --- Locking ---

  void PointCloud::lock() const { m_data->mutex.lock(); }
  void PointCloud::unlock() const { m_data->mutex.unlock(); }
  std::recursive_mutex &PointCloud::getMutex() const { return m_data->mutex; }

  // --- Deep copy ---

  PointCloud PointCloud::deepCopy() const {
    return PointCloud(*this);  // copy ctor already does deep copy
  }

} // namespace icl::geom2
