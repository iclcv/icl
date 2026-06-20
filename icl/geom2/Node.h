// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/la/FixedMatrix.h>
#include <string>
#include <memory>
#include <vector>
#include <type_traits>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::geom2 {

  using Mat = math::FixedMatrix<float, 4, 4>;

  class GroupNode;
  class Driver;
  class Node;

  /// Canonical owning handle for a scene-graph node.
  /** Ownership convention across geom2: **own a node through `NodePtr`**
      (scenes, group children and drivers all hold these); **observe a node
      through a raw `Node*`** (a non-owning view — `getParent`, `getChild`,
      `Scene2::getNode`, `Hit2::node`). A raw `Node*` never implies ownership
      and must not be deleted. */
  using NodePtr = std::shared_ptr<Node>;
  using ConstNodePtr = std::shared_ptr<const Node>;

  /// Abstract base for all scene graph nodes
  /** Provides transform, visibility, name, and locking.
      No children (GroupNode), no geometry (GeometryNode). */
  class ICLGeom2_API Node {
  public:
    virtual ~Node();
    /// Independent deep copy of this node (and its subtree, for GroupNode).
    virtual NodePtr deepCopy() const = 0;

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
    /// Non-owning view of the parent (nullptr if unparented). Never delete it;
    /// the parent owns this node, not the other way around (no ownership cycle).
    Node *getParent();
    const Node *getParent() const;

    // --- Drivers (attachable per-node behaviours; see Driver.h) ---
    /// Attach an already-constructed driver (sets its node, calls onAttach())
    void addDriver(std::shared_ptr<Driver> driver);

    /// Construct + attach a driver in place, returns the typed shared_ptr
    /** Usage: node->addDriver<SpinDriver>(rate);  (T must derive Driver) */
    template<class T, class... A>
    std::shared_ptr<T> addDriver(A &&... args) {
      static_assert(std::is_base_of_v<Driver, T>, "T must derive geom2::Driver");
      auto d = std::make_shared<T>(std::forward<A>(args)...);
      addDriver(std::static_pointer_cast<Driver>(d));
      return d;
    }

    /// First attached driver of type T (nullptr if none)
    template<class T>
    T *getDriver() const {
      for (const auto &d : getDrivers()) {
        if (auto *t = dynamic_cast<T*>(d.get())) return t;
      }
      return nullptr;
    }

    /// All attached drivers (used by Scene2::sync() traversal)
    const std::vector<std::shared_ptr<Driver>> &getDrivers() const;

    /// Detach + remove a driver (calls its onDetach())
    void removeDriver(Driver *driver);

    // --- Lifecycle ---
    virtual void prepareForRendering() {}

    // --- Locking ---
    void lock() const;
    void unlock() const;

    // --- Name ---
    void setName(const std::string &name);
    const std::string &getName() const;

  protected:
    Node();
    Node(const Node &other);
    Node &operator=(const Node &other);
    Node(Node &&other) noexcept;
    Node &operator=(Node &&other) noexcept;

  private:
    void setParent(Node *parent);

    struct Data;
    std::unique_ptr<Data> m_data;
    friend class GroupNode;  // GroupNode sets parent via setParent()
  };

} // namespace icl::geom2
