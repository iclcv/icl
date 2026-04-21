// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/SceneNode.h>
#include <icl/geom2/Renderer.h>
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
    void addObject(std::shared_ptr<SceneNode> node);

    /// Move a stack-constructed node into the scene, returns shared_ptr
    template<class T, class = std::enable_if_t<std::is_base_of_v<SceneNode, T>>>
    std::shared_ptr<T> addObject(T &&node) {
      auto p = std::make_shared<T>(std::move(node));
      addObject(std::static_pointer_cast<SceneNode>(p));
      return p;
    }

    SceneNode *getObject(int index);
    const SceneNode *getObject(int index) const;
    int getObjectCount() const;
    void removeObject(int index);
    void removeObject(SceneNode *node);
    void clear();

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
    Data *m_data;
  };

} // namespace icl::geom2
