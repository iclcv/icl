// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/nodes/Node.h>
#include <icl/viz3d/render/RenderBackend.h>
#include <icl/viz3d/nodes/LightNode.h>
#include <icl/viz3d/render/BVH.h>
#include <icl/math/Types.h>
#include <icl/qt/GLCallback.h>
#include <icl/utils/config/Configurable.h>
#include <memory>
#include <mutex>
#include <vector>
#include <type_traits>
#include <iostream>

namespace icl::cv3d {
  class Camera;
  struct ViewRay;
}

namespace icl::viz3d {

  class LightNode;
  class PointCloud;
  class SceneMouseHandler;

  using Vec = math::Vec4;

  /// Hit result from ray-node intersection
  struct Hit2 {
    Node *node = nullptr;    ///< non-owning view of the hit node (nullptr if no hit)
    Vec pos{0,0,0,1};       ///< world-space intersection point
    float dist = -1;         ///< distance from ray origin
    operator bool() const { return node != nullptr; }
    bool operator<(const Hit2 &h) const { return dist < h.dist; }
    friend std::ostream &operator<<(std::ostream &s, const Hit2 &h) {
      return h ? (s << "Hit2(dist=" << h.dist << ", pos=" << h.pos.transp() << ")")
               : (s << "Hit2(NULL)");
    }
  };

  /// Scene manager for viz3d — owns nodes, cameras, and renderer
  /** Inherits Configurable to expose scene properties (background color,
      wireframe, lighting, etc.) via an OSD button when linked to a Canvas3D. */
  class ICLViz3d_API Scene : public utils::Configurable {
  public:
    Scene();
    ~Scene();

    // --- Thread safety ---
    /// Lock the scene for multi-threaded access (run thread vs GL thread)
    /** Use std::lock_guard<Scene> or call lock()/unlock() manually.
        The GL callback locks automatically during render(). */
    void lock();
    void unlock();

    // --- Objects (ownership: the scene co-owns nodes via NodePtr) ---
    /// Add (co-own) a node. The scene keeps it alive until removed.
    void addNode(NodePtr node);

    /// Move a stack-constructed node into the scene, returns the typed handle
    template<class T, class = std::enable_if_t<std::is_base_of_v<Node, T>>>
    std::shared_ptr<T> addNode(T &&node) {
      auto p = std::make_shared<T>(std::move(node));
      addNode(std::static_pointer_cast<Node>(p));
      return p;
    }

    /// Non-owning view of the node at \a index (use getNodePtr to co-own).
    Node *getNode(int index);
    const Node *getNode(int index) const;
    /// Owning handle to the node at \a index.
    NodePtr getNodePtr(int index);
    int getNodeCount() const;
    void removeNode(int index);
    /// Remove a node, identified by a non-owning pointer (not deleted here).
    void removeNode(Node *node);
    void clear();

    // --- Change notification ---
    /// Signal that scene content changed (geometry / materials / structure).
    /** Bumps sceneVersion(), drops the renderer's geometry/texture cache, and
        flags the cached bounds for recompute. Renderers and offscreen views
        poll sceneVersion() to re-sync (the GL Renderer cache, the Cycles
        SceneSynchronizer). Called automatically by addNode/removeNode/clear and
        by node high-level mutators (via Node::ScopedEdit); call it yourself only
        after a manual low-level node edit that bypasses those paths. */
    void touch();
    /// Monotonic counter bumped by touch(); poll it to detect scene edits cheaply.
    unsigned sceneVersion() const;

    // --- Lights (also added to scene graph for traversal) ---
    void addLight(std::shared_ptr<LightNode> light);
    /// Non-owning view of the light at \a index (use getLightPtr to co-own).
    LightNode *getLight(int index);
    const LightNode *getLight(int index) const;
    /// Owning handle to the light at \a index.
    std::shared_ptr<LightNode> getLightPtr(int index);
    int getLightCount() const;

    // --- Cameras ---
    void addCamera(const cv3d::Camera &cam);
    cv3d::Camera &getCamera(int index);
    const cv3d::Camera &getCamera(int index) const;
    int getCameraCount() const;

    // --- Driver update (UI thread) ---
    /// Advance all node drivers one render frame.
    /** Pre-order traversal (parent before children); calls Driver::sync(dt,
        alpha) on every driver of every node. Call once per frame on the UI
        thread before render(). `alpha` is the interpolation fraction for
        state-publishing drivers (physics); time-driven drivers ignore it and
        it defaults to 1. */
    void sync(double dt, double alpha = 1.0);

    // --- Rendering ---
    void render(int cameraIndex);
    RenderBackend &getRenderer();

    /// Offscreen GL render through \a cameraIndex → RGB color + metric depth.
    /** GPU counterpart to BVH::raycastToImage (same BVH::ImageResult /
        BVH::DepthMode vocabulary), so a SceneCapture can swap CPU↔GL backends
        transparently. Full PBR shading; depth is returned in millimetres in the
        requested format (DistToCamPlane = Z-depth, DistToCamCenter = Euclidean).

        REQUIRES a current GL context whose Renderer is this scene's — e.g.
        called on the GUI thread with the on-screen widget's context current, or
        inside a dedicated offscreen context owned by a headless capture. SSR is
        forced off for the duration: with SSR on, the geometry pass writes depth
        into an internal FBO and only color is blitted back, so the capture FBO's
        depth attachment would stay empty. Returns an empty result if there is no
        GL context / OpenGL support, or the camera index is invalid. */
    BVH::ImageResult renderToImage(int cameraIndex,
                                   BVH::DepthMode mode = BVH::DistToCamPlane);

    // --- GL callback for ICLQt integration ---
    /// Returns a callback suitable for `canvas->link()` /
    /// `gui["canvas"].link(...)`.
    /** Usage: gui["canvas"].link(scene.getGLCallback(0).get()); */
    std::shared_ptr<qt::GLCallback> getGLCallback(int cameraIndex);

    // --- Mouse interaction ---
    /// Returns mouse handler for camera navigation (lazy-created per camera)
    /** Usage: canvas->install(scene.getMouseHandler(0)); */
    SceneMouseHandler *getMouseHandler(int cameraIndex);

    // --- Hit testing ---
    /// Find closest node hit by the given view ray
    Hit2 findObject(const cv3d::ViewRay &ray) const;

    /// Find all nodes hit by the given view ray, sorted by distance
    std::vector<Hit2> findObjects(const cv3d::ViewRay &ray) const;

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
    /// Nodes to draw this pass: the scene objects, plus a lightweight gizmo per
    /// camera when the "show cameras" property is on (the active camera's own
    /// gizmo is omitted). Returns a reference valid until the next call.
    const std::vector<std::shared_ptr<Node>> &nodesToRender(int activeCam);

    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::viz3d
