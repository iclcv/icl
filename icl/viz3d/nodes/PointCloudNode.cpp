// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/nodes/PointCloudNode.h>
#include <icl/viz3d/pointcloud/PointCloud.h>
#include <icl/core/Img.h>
#include <icl/viz3d/render/Material.h>

namespace icl::viz3d {

  struct PointCloudNode::Data {
    std::shared_ptr<PointCloud> cloud;
    float pointSize = 3.0f;
    std::shared_ptr<viz3d::Material> material;
  };

  PointCloudNode::PointCloudNode() : m_data(std::make_unique<Data>()) {}

  PointCloudNode::PointCloudNode(std::shared_ptr<PointCloud> cloud)
    : m_data(std::make_unique<Data>()) {
    m_data->cloud = std::move(cloud);
  }

  PointCloudNode::~PointCloudNode() = default;

  PointCloudNode::PointCloudNode(const PointCloudNode &other)
    : Node(other), m_data(std::make_unique<Data>(*other.m_data)) {
    if (m_data->material) m_data->material = m_data->material->deepCopy();
  }

  PointCloudNode &PointCloudNode::operator=(const PointCloudNode &other) {
    if (this != &other) {
      Node::operator=(other);
      m_data = std::make_unique<Data>(*other.m_data);
      if (m_data->material) m_data->material = m_data->material->deepCopy();
    }
    return *this;
  }

  PointCloudNode::PointCloudNode(PointCloudNode &&other) noexcept = default;
  PointCloudNode &PointCloudNode::operator=(PointCloudNode &&other) noexcept = default;

  NodePtr PointCloudNode::deepCopy() const {
    return std::make_shared<PointCloudNode>(*this);
  }

  void PointCloudNode::setPointCloud(std::shared_ptr<PointCloud> cloud) {
    m_data->cloud = std::move(cloud);
  }

  std::shared_ptr<PointCloud> PointCloudNode::getPointCloud() const {
    return m_data->cloud;
  }

  void PointCloudNode::setPointSize(float size) { m_data->pointSize = size; }
  float PointCloudNode::getPointSize() const { return m_data->pointSize; }

  void PointCloudNode::setMaterial(std::shared_ptr<viz3d::Material> mat) {
    m_data->material = std::move(mat);
  }
  std::shared_ptr<viz3d::Material> PointCloudNode::getMaterial() const {
    return m_data->material;
  }

  std::shared_ptr<PointCloudNode> PointCloudNode::create(std::shared_ptr<PointCloud> cloud) {
    return std::make_shared<PointCloudNode>(std::move(cloud));
  }

} // namespace icl::viz3d
