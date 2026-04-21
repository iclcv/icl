// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/Scene2.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/GeometryNode.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/BVH.h>
#include <icl/geom/Camera.h>
#include <icl/geom/ViewRay.h>
#include <icl/geom/Material.h>
#include <icl/qt/DrawWidget3D.h>
#include <icl/core/Img.h>
#include <algorithm>
#include <cmath>

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
    std::vector<std::shared_ptr<Node>> objects;
    std::vector<std::shared_ptr<LightNode>> lights;  // also in objects, tracked for fast access
    std::vector<geom::Camera> cameras;
    Renderer renderer;
    std::vector<std::shared_ptr<GLCallback>> callbacks;
    std::vector<std::unique_ptr<Scene2MouseHandler>> mouseHandlers;
    Vec cursor{0, 0, 0, 1};
    float bounds = 1000.0f;
  };

  // ---- Scene2 implementation ----

  Scene2::Scene2() : m_data(std::make_unique<Data>()) {}
  Scene2::~Scene2() = default;

  void Scene2::addNode(std::shared_ptr<Node> node) {
    m_data->objects.push_back(std::move(node));
  }

  Node *Scene2::getNode(int i) {
    return (i >= 0 && i < (int)m_data->objects.size()) ? m_data->objects[i].get() : nullptr;
  }

  const Node *Scene2::getNode(int i) const {
    return (i >= 0 && i < (int)m_data->objects.size()) ? m_data->objects[i].get() : nullptr;
  }

  int Scene2::getNodeCount() const { return (int)m_data->objects.size(); }

  void Scene2::removeNode(int i) {
    if (i >= 0 && i < (int)m_data->objects.size()) {
      m_data->objects.erase(m_data->objects.begin() + i);
    }
  }

  void Scene2::removeNode(Node *node) {
    auto &o = m_data->objects;
    o.erase(std::remove_if(o.begin(), o.end(),
            [node](const auto &p) { return p.get() == node; }), o.end());
  }

  void Scene2::clear() {
    m_data->objects.clear();
    m_data->lights.clear();
    m_data->renderer.invalidateCache();
  }

  // Lights
  void Scene2::addLight(std::shared_ptr<LightNode> light) {
    m_data->lights.push_back(light);
    addNode(std::static_pointer_cast<Node>(light));
  }

  LightNode *Scene2::getLight(int i) {
    return (i >= 0 && i < (int)m_data->lights.size()) ? m_data->lights[i].get() : nullptr;
  }

  const LightNode *Scene2::getLight(int i) const {
    return (i >= 0 && i < (int)m_data->lights.size()) ? m_data->lights[i].get() : nullptr;
  }

  int Scene2::getLightCount() const { return (int)m_data->lights.size(); }

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

  // --- Mouse handler ---

  Scene2MouseHandler *Scene2::getMouseHandler(int cameraIndex) {
    while ((int)m_data->mouseHandlers.size() <= cameraIndex) {
      int idx = (int)m_data->mouseHandlers.size();
      auto h = std::make_unique<Scene2MouseHandler>(idx, this);
      h->setSensitivities(m_data->bounds);
      m_data->mouseHandlers.push_back(std::move(h));
    }
    return m_data->mouseHandlers[cameraIndex].get();
  }

  // --- Hit testing ---

  // Recursive hit collection against geom2 node graph
  static void collectHits(Node *node, const geom::ViewRay &ray, std::vector<Hit2> &hits) {
    if (!node || !node->isVisible()) return;

    // Recurse into groups
    if (auto *group = dynamic_cast<GroupNode*>(node)) {
      for (int i = 0; i < group->getChildCount(); i++) {
        collectHits(group->getChild(i), ray, hits);
      }
    }

    // Test geometry nodes (MeshNode, SphereNode, CuboidNode, etc.)
    if (auto *geom = dynamic_cast<GeometryNode*>(node)) {
      const auto &verts = geom->getVertices();
      if (verts.empty()) return;

      // Transform vertices to world space
      Mat xform = node->getTransformation(true);
      std::vector<Vec> ws(verts.size());
      for (size_t i = 0; i < verts.size(); i++) {
        ws[i] = xform * verts[i];
      }

      // Test triangles
      for (const auto &tri : geom->getTriangles()) {
        Vec ip;
        auto result = ray.getIntersectionWithTriangle(ws[tri.v[0]], ws[tri.v[1]], ws[tri.v[2]], &ip);
        if (result == geom::ViewRay::foundIntersection) {
          Vec d = ip - ray.offset;
          float dist = std::sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
          hits.push_back({node, ip, dist});
        }
      }

      // Test quads (as 2 triangles each)
      for (const auto &q : geom->getQuads()) {
        for (int t = 0; t < 2; t++) {
          int i0 = q.v[0], i1 = q.v[t+1], i2 = q.v[t+2];
          Vec ip;
          auto result = ray.getIntersectionWithTriangle(ws[i0], ws[i1], ws[i2], &ip);
          if (result == geom::ViewRay::foundIntersection) {
            Vec d = ip - ray.offset;
            float dist = std::sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
            hits.push_back({node, ip, dist});
            break;  // one hit per quad is enough
          }
        }
      }
    }
  }

  Hit2 Scene2::findObject(const geom::ViewRay &ray) const {
    std::vector<Hit2> hits;
    for (auto &node : m_data->objects) {
      collectHits(node.get(), ray, hits);
    }
    return hits.empty() ? Hit2() : *std::min_element(hits.begin(), hits.end());
  }

  std::vector<Hit2> Scene2::findObjects(const geom::ViewRay &ray) const {
    std::vector<Hit2> hits;
    for (auto &node : m_data->objects) {
      collectHits(node.get(), ray, hits);
    }
    std::sort(hits.begin(), hits.end());
    return hits;
  }

  Hit2 Scene2::findObject(int cameraIndex, int x, int y) const {
    return findObject(getCamera(cameraIndex).getViewRay(utils::Point32f(x, y)));
  }

  // --- Cursor ---

  void Scene2::setCursor(const Vec &pos) { m_data->cursor = pos; }
  Vec Scene2::getCursor() const { return m_data->cursor; }

  // --- Bounds ---

  void Scene2::setBounds(float maxDim) {
    m_data->bounds = maxDim;
    // Update sensitivities on existing handlers
    for (auto &h : m_data->mouseHandlers) {
      if (h) h->setSensitivities(maxDim);
    }
  }
  float Scene2::getBounds() const { return m_data->bounds; }

  // --- Batch raycast ---

  // Pre-computed geometry for fast batch raycasting
  struct PreparedGeom {
    Node *node;
    GeomColor color{200,200,200,255};
    std::vector<Vec> verts;       // world-space vertices
    // Flattened triangle list: 3 indices per triangle
    std::vector<int> triIndices;
    // AABB for early rejection
    float aabbMin[3], aabbMax[3];

    void computeAABB() {
      aabbMin[0] = aabbMin[1] = aabbMin[2] = 1e30f;
      aabbMax[0] = aabbMax[1] = aabbMax[2] = -1e30f;
      for (const auto &v : verts) {
        for (int i = 0; i < 3; i++) {
          if (v[i] < aabbMin[i]) aabbMin[i] = v[i];
          if (v[i] > aabbMax[i]) aabbMax[i] = v[i];
        }
      }
    }

    // Fast AABB-ray test (slab method)
    bool intersectsRay(const Vec &origin, const Vec &invDir) const {
      float t1 = (aabbMin[0] - origin[0]) * invDir[0];
      float t2 = (aabbMax[0] - origin[0]) * invDir[0];
      float tmin = std::min(t1, t2);
      float tmax = std::max(t1, t2);
      t1 = (aabbMin[1] - origin[1]) * invDir[1];
      t2 = (aabbMax[1] - origin[1]) * invDir[1];
      tmin = std::max(tmin, std::min(t1, t2));
      tmax = std::min(tmax, std::max(t1, t2));
      t1 = (aabbMin[2] - origin[2]) * invDir[2];
      t2 = (aabbMax[2] - origin[2]) * invDir[2];
      tmin = std::max(tmin, std::min(t1, t2));
      tmax = std::min(tmax, std::max(t1, t2));
      return tmax >= std::max(tmin, 0.0f);
    }
  };

  static void collectPreparedGeom(Node *node, std::vector<PreparedGeom> &out) {
    if (!node || !node->isVisible()) return;

    if (auto *group = dynamic_cast<GroupNode*>(node)) {
      for (int i = 0; i < group->getChildCount(); i++)
        collectPreparedGeom(group->getChild(i), out);
    }

    if (auto *geom = dynamic_cast<GeometryNode*>(node)) {
      const auto &verts = geom->getVertices();
      if (verts.empty()) return;

      PreparedGeom pg;
      pg.node = node;

      // Get material color
      auto mat = geom->getMaterial();
      if (mat) pg.color = mat->baseColor;

      // Transform vertices once
      Mat xform = node->getTransformation(true);
      pg.verts.resize(verts.size());
      for (size_t i = 0; i < verts.size(); i++) {
        pg.verts[i] = xform * verts[i];
      }

      // Flatten triangles
      for (const auto &tri : geom->getTriangles()) {
        pg.triIndices.push_back(tri.v[0]);
        pg.triIndices.push_back(tri.v[1]);
        pg.triIndices.push_back(tri.v[2]);
      }
      for (const auto &q : geom->getQuads()) {
        pg.triIndices.push_back(q.v[0]);
        pg.triIndices.push_back(q.v[1]);
        pg.triIndices.push_back(q.v[2]);
        pg.triIndices.push_back(q.v[0]);
        pg.triIndices.push_back(q.v[2]);
        pg.triIndices.push_back(q.v[3]);
      }

      if (!pg.triIndices.empty()) {
        pg.computeAABB();
        out.push_back(std::move(pg));
      }
    }
  }

  // Build a BVH from the scene's prepared geometry
  static BVH buildSceneBVH(const std::vector<PreparedGeom> &geoms) {
    std::vector<BVH::Triangle> tris;
    for (const auto &pg : geoms) {
      int nTri = (int)pg.triIndices.size() / 3;
      for (int t = 0; t < nTri; t++) {
        tris.push_back({
          pg.verts[pg.triIndices[t*3]],
          pg.verts[pg.triIndices[t*3+1]],
          pg.verts[pg.triIndices[t*3+2]],
          pg.node,
          pg.color
        });
      }
    }
    BVH bvh;
    bvh.build(std::move(tris));
    return bvh;
  }

  BVH Scene2::buildBVH() const {
    std::vector<PreparedGeom> geoms;
    for (auto &node : m_data->objects) {
      collectPreparedGeom(node.get(), geoms);
    }
    return buildSceneBVH(geoms);
  }

} // namespace icl::geom2
