// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/nodes/Node.h>
#include <memory>

namespace icl::viz3d {

  /// A pure container node — owns children, no geometry
  class ICLViz3d_API GroupNode : public Node {
  public:
    GroupNode();
    ~GroupNode() override;
    GroupNode(const GroupNode &other);
    GroupNode &operator=(const GroupNode &other);
    GroupNode(GroupNode &&other) noexcept;
    GroupNode &operator=(GroupNode &&other) noexcept;
    NodePtr deepCopy() const override;

    /// Add (co-own) a child. The group keeps the child alive.
    void addChild(NodePtr child);
    /// Remove a child, identified by a non-owning pointer (not deleted here).
    void removeChild(Node *child);
    void removeAllChildren();
    int getChildCount() const;
    /// Non-owning view of the child at \a index (use getChildPtr to co-own).
    Node *getChild(int index);
    const Node *getChild(int index) const;
    /// Owning handle to the child at \a index.
    NodePtr getChildPtr(int index);

  private:
    /// Propagate the owning-scene back-pointer to the whole subtree.
    void setScene(Scene *scene) override;

    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::viz3d
