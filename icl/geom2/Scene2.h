// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Node.h>
#include <icl/geom2/Renderer.h>
#include <icl/geom2/LightNode.h>
#include <memory>
#include <vector>
#include <type_traits>

namespace icl::geom {
  class Camera;
}

namespace icl::geom2 {

  class LightNode;

  /// Scene manager for geom2 — owns nodes, cameras, and renderer
  class ICLGeom2_API Scene2 {
  public:
    Scene2();
    ~Scene2();

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

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::geom2
