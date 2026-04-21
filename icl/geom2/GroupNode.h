// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/SceneNode.h>
#include <memory>

namespace icl::geom2 {

  /// A pure container node — owns children, no geometry
  class ICLGeom2_API GroupNode : public SceneNode {
  public:
    GroupNode();
    ~GroupNode() override;
    GroupNode(const GroupNode &other);
    GroupNode &operator=(const GroupNode &other);
    GroupNode(GroupNode &&other) noexcept;
    GroupNode &operator=(GroupNode &&other) noexcept;
    SceneNode *deepCopy() const override;

    void addChild(std::shared_ptr<SceneNode> child);
    void removeChild(SceneNode *child);
    void removeAllChildren();
    int getChildCount() const;
    SceneNode *getChild(int index);
    const SceneNode *getChild(int index) const;
    std::shared_ptr<SceneNode> getChildPtr(int index);

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::geom2
