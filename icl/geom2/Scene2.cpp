// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/Scene2.h>
#include <icl/geom/Camera.h>
#include <icl/qt/DrawWidget3D.h>
#include <algorithm>

namespace icl::geom2 {

  // ---- GLCallback implementation ----

  struct Scene2::GLCallback : public qt::ICLDrawWidget3D::GLCallback {
    Scene2 *scene;
    int camIndex;

    GLCallback(Scene2 *s, int ci) : scene(s), camIndex(ci) {}

    void draw(qt::ICLDrawWidget3D *widget) override {
      (void)widget;
      scene->render(camIndex);
    }
  };

  // ---- Data ----

  struct Scene2::Data {
    std::vector<std::shared_ptr<SceneNode>> objects;
    std::vector<geom::Camera> cameras;
    Renderer renderer;
    std::vector<std::shared_ptr<GLCallback>> callbacks;
  };

  // ---- Scene2 implementation ----

  Scene2::Scene2() : m_data(new Data) {}
  Scene2::~Scene2() { delete m_data; }

  void Scene2::addObject(std::shared_ptr<SceneNode> node) {
    m_data->objects.push_back(std::move(node));
  }

  SceneNode *Scene2::getObject(int i) {
    return (i >= 0 && i < (int)m_data->objects.size()) ? m_data->objects[i].get() : nullptr;
  }

  const SceneNode *Scene2::getObject(int i) const {
    return (i >= 0 && i < (int)m_data->objects.size()) ? m_data->objects[i].get() : nullptr;
  }

  int Scene2::getObjectCount() const { return (int)m_data->objects.size(); }

  void Scene2::removeObject(int i) {
    if (i >= 0 && i < (int)m_data->objects.size()) {
      m_data->objects.erase(m_data->objects.begin() + i);
    }
  }

  void Scene2::removeObject(SceneNode *node) {
    auto &o = m_data->objects;
    o.erase(std::remove_if(o.begin(), o.end(),
            [node](const auto &p) { return p.get() == node; }), o.end());
  }

  void Scene2::clear() {
    m_data->objects.clear();
    m_data->renderer.invalidateCache();
  }

  // Cameras
  void Scene2::addCamera(const geom::Camera &cam) {
    m_data->cameras.push_back(cam);
  }

  geom::Camera &Scene2::getCamera(int i) { return m_data->cameras.at(i); }
  const geom::Camera &Scene2::getCamera(int i) const { return m_data->cameras.at(i); }
  int Scene2::getCameraCount() const { return (int)m_data->cameras.size(); }

  // Rendering
  Renderer &Scene2::getRenderer() { return m_data->renderer; }

  void Scene2::render(int cameraIndex) {
    if (cameraIndex < 0 || cameraIndex >= (int)m_data->cameras.size()) return;

    const auto &cam = m_data->cameras[cameraIndex];
    Mat viewGL = cam.getCSTransformationMatrixGL();
    Mat projGL = cam.getProjectionMatrixGL();

    m_data->renderer.render(m_data->objects, viewGL, projGL);
  }

  // GL callback for ICLQt integration
  std::shared_ptr<Scene2::GLCallback> Scene2::getGLCallback(int cameraIndex) {
    // Ensure enough callbacks exist
    while ((int)m_data->callbacks.size() <= cameraIndex) {
      m_data->callbacks.push_back(
          std::make_shared<GLCallback>(this, (int)m_data->callbacks.size()));
    }
    return m_data->callbacks[cameraIndex];
  }

} // namespace icl::geom2
