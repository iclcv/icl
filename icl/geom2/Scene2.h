// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Node.h>
#include <icl/geom2/Renderer.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/BVH.h>
#include <icl/math/FixedVector.h>
#include <icl/utils/Configurable.h>
#include <memory>
#include <mutex>
#include <vector>
#include <type_traits>
#include <iostream>

namespace icl::geom {
  class Camera;
  struct ViewRay;
}

namespace icl::geom2 {

  class LightNode;
  class PointCloud;
  class Scene2MouseHandler;

  using Vec = math::FixedColVector<float, 4>;

  /// Hit result from ray-node intersection
  struct Hit2 {
    Node *node = nullptr;    ///< hit node (nullptr if no hit)
    Vec pos{0,0,0,1};       ///< world-space intersection point
    float dist = -1;         ///< distance from ray origin
    operator bool() const { return node != nullptr; }
    bool operator<(const Hit2 &h) const { return dist < h.dist; }
    friend std::ostream &operator<<(std::ostream &s, const Hit2 &h) {
      return h ? (s << "Hit2(dist=" << h.dist << ", pos=" << h.pos.transp() << ")")
               : (s << "Hit2(NULL)");
    }
  };

  /// Scene manager for geom2 — owns nodes, cameras, and renderer
  /** Inherits Configurable to expose scene properties (background color,
      wireframe, lighting, etc.) via an OSD button when linked to a Canvas3D. */
  class ICLGeom2_API Scene2 : public utils::Configurable {
  public:
    Scene2();
    ~Scene2();

    // --- Thread safety ---
    /// Lock the scene for multi-threaded access (run thread vs GL thread)
    /** Use std::lock_guard<Scene2> or call lock()/unlock() manually.
        The GL callback locks automatically during render(). */
    void lock();
    void unlock();

    // --- Objects (shared_ptr only, no raw pointers) ---
    void addNode(std::shared_ptr<Node> node);

    /// Move a stack-constructed node into the scene, returns shared_ptr
    template<class T, class = std::enable_if_t<std::is_base_of_v<Node, T>>>
    std::shared_ptr<T> addNode(T &&node) {
      auto p = std::make_shared<T>(std::move(node));
      addNode(std::static_pointer_cast<Node>(p));
      return p;
    }

    Node *getNode(int index);
    const Node *getNode(int index) const;
    int getNodeCount() const;
    void removeNode(int index);
    void removeNode(Node *node);
    void clear();

    // --- Lights (also added to scene graph for traversal) ---
    void addLight(std::shared_ptr<LightNode> light);
    LightNode *getLight(int index);
    const LightNode *getLight(int index) const;
    int getLightCount() const;

    // --- Cameras ---
    void addCamera(const geom::Camera &cam);
    geom::Camera &getCamera(int index);
    const geom::Camera &getCamera(int index) const;
    int getCameraCount() const;

    // --- Rendering ---
    void render(int cameraIndex);
    Renderer &getRenderer();

    // --- GL callback for ICLQt integration ---
    /// Returns a callback suitable for canvas->link()
    /** Usage: canvas->link(scene.getGLCallback(0).get()); */
    struct GLCallback;
    std::shared_ptr<GLCallback> getGLCallback(int cameraIndex);

    // --- Mouse interaction ---
    /// Returns mouse handler for camera navigation (lazy-created per camera)
    /** Usage: canvas->install(scene.getMouseHandler(0)); */
    Scene2MouseHandler *getMouseHandler(int cameraIndex);

    // --- Hit testing ---
    /// Find closest node hit by the given view ray
    Hit2 findObject(const geom::ViewRay &ray) const;

    /// Find all nodes hit by the given view ray, sorted by distance
    std::vector<Hit2> findObjects(const geom::ViewRay &ray) const;

    /// Find closest hit at screen coordinates for given camera
    Hit2 findObject(int cameraIndex, int x, int y) const;

    /// Build a BVH from all visible geometry in the scene (world-space)
    /** The returned BVH can be used for fast ray queries, picking, etc.
        Thread-safe for concurrent queries after construction. */
    BVH buildBVH() const;

    // --- Cursor (rotation center for mouse navigation) ---
    void setCursor(const Vec &pos);
    Vec getCursor() const;

    // --- Scene bounds (for sensitivity auto-scaling) ---
    void setBounds(float maxDim);
    float getBounds() const;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::geom2
