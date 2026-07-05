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

namespace icl::viz3d {

  using Mat = math::FixedMatrix<float, 4, 4>;

  class GroupNode;
  class Driver;
  class Node;
  class Scene;

  /// Canonical owning handle for a scene-graph node.
  /** viz3d references follow a three-tier **edge** model — pick the pointer by
      the relationship, not by habit:

      - **Ownership (down-edge)** — `NodePtr` / `shared_ptr` / `unique_ptr`.
        The parent keeps the child alive: scenes own nodes, groups own children,
        a node owns its drivers.
      - **Observe-your-owner (up-edge)** — a raw, non-owning `Node*`.
        `getParent()`, `Driver::node()`. The owner outlives you by construction,
        so a raw view is correct here (a `shared_ptr` back up would be a cycle).
        Never delete it.
      - **Cross-edge** — `weak_ptr<T>`. A long-lived reference to a *peer* you
        neither own nor are owned by (e.g. a physics constraint or a driver that
        tracks another node's body). Lock on use; an expired handle means the
        peer is gone. Use `getDriverPtr<T>()` to obtain the strong handle to
        downgrade.

      A raw `Node*`/`Driver*` is also fine as a *transient* observer (a getter
      result, a removal key) — it just must never be stored as a cross-edge. */
  using NodePtr = std::shared_ptr<Node>;
  using ConstNodePtr = std::shared_ptr<const Node>;

  /// Abstract base for all scene graph nodes
  /** Provides transform, visibility, name, and locking.
      No children (GroupNode), no geometry (GeometryNode). */
  class ICLViz3d_API Node {
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

    // --- Owning scene (set by Scene::addNode, propagated by GroupNode) ---
    /// Non-owning view of the Scene this node lives in (nullptr if unparented).
    /** An up-edge: the scene co-owns the node, so a raw view back up is correct
        (never delete it). High-level mutators use it to self-lock and
        auto-invalidate the scene — see ScopedEdit. */
    Scene *getScene() const;

    // --- Drivers (attachable per-node behaviours; see Driver.h) ---
    /// Attach an already-constructed driver (sets its node, calls onAttach())
    void addDriver(std::shared_ptr<Driver> driver);

    /// Construct + attach a driver in place, returns the typed shared_ptr
    /** Usage: node->addDriver<SpinDriver>(rate);  (T must derive Driver) */
    template<class T, class... A>
    std::shared_ptr<T> addDriver(A &&... args) {
      static_assert(std::is_base_of_v<Driver, T>, "T must derive viz3d::Driver");
      auto d = std::make_shared<T>(std::forward<A>(args)...);
      addDriver(std::static_pointer_cast<Driver>(d));
      return d;
    }

    /// First attached driver of type T as a non-owning view (nullptr if none).
    /** Transient observer — fine to use and drop. To keep a *cross-edge* to a
        driver (store a reference to a peer's driver), use getDriverPtr<T>() and
        downgrade it to a weak_ptr. */
    template<class T>
    T *getDriver() const {
      for (const auto &d : getDrivers()) {
        if (auto *t = dynamic_cast<T*>(d.get())) return t;
      }
      return nullptr;
    }

    /// First attached driver of type T as an owning handle (nullptr if none).
    /** The strong handle a cross-edge downgrades from: `weak_ptr<T> w =
        node->getDriverPtr<T>();`. Mirrors getChildPtr / getNodePtr. */
    template<class T>
    std::shared_ptr<T> getDriverPtr() const {
      for (const auto &d : getDrivers()) {
        if (auto t = std::dynamic_pointer_cast<T>(d)) return t;
      }
      return nullptr;
    }

    /// All attached drivers (used by Scene::sync() traversal)
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

    /// RAII guard for a node's high-level mutators: locks the owning scene (if
    /// the node is in one) for the edit's duration, and on exit marks the scene
    /// changed (Scene::touch() → invalidate renderer/Cycles caches + bump
    /// version). Re-entrant: the scene mutex is recursive, so it composes with
    /// an outer render/sync lock. A detached node (no scene) makes it a no-op.
    /** Usage in a mutator:  ScopedEdit edit(this); ... rebuild geometry ... */
    class ScopedEdit {
      Scene *m_scene;
    public:
      explicit ScopedEdit(Node *node);
      ~ScopedEdit();
      ScopedEdit(const ScopedEdit &) = delete;
      ScopedEdit &operator=(const ScopedEdit &) = delete;
    };

  private:
    void setParent(Node *parent);
    /// Set/clear the owning scene back-pointer. Virtual so GroupNode propagates
    /// it to its subtree. Called by Scene::addNode/removeNode and GroupNode.
    virtual void setScene(Scene *scene);

    struct Data;
    std::unique_ptr<Data> m_data;
    friend class GroupNode;  // sets parent via setParent(), propagates setScene()
    friend class Scene;     // sets the owning-scene back-pointer via setScene()
  };

} // namespace icl::viz3d
