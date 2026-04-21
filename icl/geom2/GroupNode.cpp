// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/GroupNode.h>
#include <algorithm>

namespace icl::geom2 {

  struct GroupNode::Data {
    std::vector<std::shared_ptr<Node>> children;
  };

  GroupNode::GroupNode() : m_data(std::make_unique<Data>()) {}

  GroupNode::~GroupNode() = default;

  GroupNode::GroupNode(const GroupNode &other) : Node(other), m_data(std::make_unique<Data>()) {
    m_data->children.resize(other.m_data->children.size());
    for (size_t i = 0; i < other.m_data->children.size(); ++i) {
      m_data->children[i].reset(other.m_data->children[i]->deepCopy());
    }
  }

  GroupNode &GroupNode::operator=(const GroupNode &other) {
    if (this != &other) {
      GroupNode tmp(other);
      Node::operator=(std::move(tmp));
      std::swap(m_data, tmp.m_data);
    }
    return *this;
  }

  GroupNode::GroupNode(GroupNode &&other) noexcept = default;
  GroupNode &GroupNode::operator=(GroupNode &&other) noexcept = default;

  Node *GroupNode::deepCopy() const { return new GroupNode(*this); }

  void GroupNode::addChild(std::shared_ptr<Node> child) {
    child->setParent(this);
    m_data->children.push_back(std::move(child));
  }

  void GroupNode::removeChild(Node *child) {
    auto &c = m_data->children;
    auto it = std::find_if(c.begin(), c.end(),
                           [child](const auto &p) { return p.get() == child; });
    if (it != c.end()) {
      (*it)->setParent(nullptr);
      c.erase(it);
    }
  }

  void GroupNode::removeAllChildren() {
    for (auto &c : m_data->children) c->setParent(nullptr);
    m_data->children.clear();
  }

  int GroupNode::getChildCount() const { return (int)m_data->children.size(); }

  Node *GroupNode::getChild(int i) {
    return (i >= 0 && i < (int)m_data->children.size()) ? m_data->children[i].get() : nullptr;
  }

  const Node *GroupNode::getChild(int i) const {
    return (i >= 0 && i < (int)m_data->children.size()) ? m_data->children[i].get() : nullptr;
  }

  std::shared_ptr<Node> GroupNode::getChildPtr(int i) {
    return (i >= 0 && i < (int)m_data->children.size()) ? m_data->children[i] : nullptr;
  }

} // namespace icl::geom2
