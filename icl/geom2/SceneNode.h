// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/FixedMatrix.h>
#include <string>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::geom2 {

  using Mat = math::FixedMatrix<float, 4, 4>;

  class GroupNode;

  /// Abstract base for all scene graph nodes
  /** Provides transform, visibility, name, and locking.
      No children (GroupNode), no geometry (GeometryNode). */
  class ICLGeom2_API SceneNode {
  public:
    virtual ~SceneNode();
    virtual SceneNode *deepCopy() const = 0;

    // --- Transform ---
    void setTransformation(const Mat &m);
    void removeTransformation();
    void transform(const Mat &m);
    void rotate(float rx, float ry, float rz);
    void translate(float dx, float dy, float dz);
    void scale(float sx, float sy, float sz);
    Mat getTransformation(bool includeParent = false) const;
    bool hasTransformation(bool includeParent = false) const;

    // --- Visibility ---
    void setVisible(bool visible);
    bool isVisible() const;

    // --- Parent (set by GroupNode::addChild) ---
    SceneNode *getParent();
    const SceneNode *getParent() const;

    // --- Lifecycle ---
    virtual void prepareForRendering() {}

    // --- Locking ---
    void lock() const;
    void unlock() const;

    // --- Name ---
    void setName(const std::string &name);
    const std::string &getName() const;

  protected:
    SceneNode();
    SceneNode(const SceneNode &other);
    SceneNode &operator=(const SceneNode &other);
    SceneNode(SceneNode &&other) noexcept;
    SceneNode &operator=(SceneNode &&other) noexcept;

  private:
    void setParent(SceneNode *parent);

    struct Data;
    Data *m_data;
    friend class GroupNode;  // GroupNode sets parent via setParent()
  };

} // namespace icl::geom2
