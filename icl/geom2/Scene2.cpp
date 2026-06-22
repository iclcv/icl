// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/Scene2.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/core/prop/Constraints.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/GeometryNode.h>
#include <icl/geom2/Driver.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/BVH.h>
#include <icl/geom/Camera.h>
#include <icl/geom/ViewRay.h>
#include <icl/geom/Material.h>
#include <icl/qt/DrawWidget3D.h>
#include <icl/qt/Widget.h>
#include <icl/qt/IconFactory.h>
#include <icl/qt/ContainerGUIComponents.h>
#include <icl/qt/ui.h>
#include <icl/qt/GUIComponents.h>
#include <icl/core/Img.h>

#ifdef ICL_HAVE_OPENGL
#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#endif

#include <algorithm>
#include <cmath>

namespace icl::geom2 {

  // ---- GLCallback implementation ----
  //
  // Implementation-internal subclass of `qt::GLCallback`; not
  // exposed in Scene2.h.  Scene2::getGLCallback() returns a
  // `shared_ptr<qt::GLCallback>` (base) so callers never see this
  // type.
  namespace {
    struct SceneGLCallback : public qt::GLCallback {
      Scene2 *scene;
      int camIndex;
      bool needLink = true;
      qt::GUI *gui = nullptr;

      SceneGLCallback(Scene2 *s, int ci) : scene(s), camIndex(ci) {}

      void performLink(qt::ICLDrawWidget3D *widget) {
        std::string id = "scene2-" + utils::str(this);
        std::string save = scene->getConfigurableID();
        scene->setConfigurableID(id);

        gui = new qt::GUI(qt::VBox());
        *gui << qt::Prop(id) << qt::Create();

        static const core::Img8u &icon = qt::IconFactory::create_image("scene-props");
        widget->addSpecialButton("scene2-props",
            &icon,
            [this]{ gui->switchVisibility(); },
            "3D scene properties");
        scene->setConfigurableID(save);
      }

      void draw(qt::ICLDrawWidget3D *widget) override {
        if (needLink && widget) {
          performLink(widget);
          needLink = false;
        }
        scene->render(camIndex);
      }
    };
  }  // namespace

  // ---- Data ----

  struct Scene2::Data {
    std::vector<std::shared_ptr<Node>> objects;
    std::vector<std::shared_ptr<LightNode>> lights;  // also in objects, tracked for fast access
    std::vector<geom::Camera> cameras;
    Renderer renderer;
    std::vector<std::shared_ptr<SceneGLCallback>> callbacks;
    std::vector<std::unique_ptr<Scene2MouseHandler>> mouseHandlers;
    Vec cursor{0, 0, 0, 1};
    float explicitBounds = -1.0f;          // >0: user override (setBounds); else auto
    mutable float autoBounds = 1000.0f;    // cached scene-derived size
    mutable bool autoBoundsDirty = true;   // recompute lazily on structure change only
    std::recursive_mutex mutex;
  };

  // ---- Scene2 implementation ----

  void Scene2::lock() { m_data->mutex.lock(); }
  void Scene2::unlock() { m_data->mutex.unlock(); }

  Scene2::Scene2() : m_data(std::make_unique<Data>()) {
    addProperty("background color", core::prop::Color{}, core::Color(0,0,0));
    addProperty("wireframe",utils::prop::Flag{}, false);
    addProperty("enable lighting",utils::prop::Flag{}, true);
    addProperty("debug", utils::prop::Menu{"shaded", "normals", "albedo", "UVs",
                "lighting", "NdotL", "SSR confidence", "depth", "SSR only"}, "shaded");
    addProperty("point size",utils::prop::Range{.min=1, .max=20}, 3);
    addProperty("info.Nodes",utils::prop::Info{}, utils::str(0));
    addProperty("info.Lights",utils::prop::Info{}, utils::str(0));
  }
  Scene2::~Scene2() = default;

  void Scene2::addNode(NodePtr node) {
    m_data->objects.push_back(std::move(node));
    m_data->autoBoundsDirty = true;
  }

  Node *Scene2::getNode(int i) {
    return (i >= 0 && i < (int)m_data->objects.size()) ? m_data->objects[i].get() : nullptr;
  }

  const Node *Scene2::getNode(int i) const {
    return (i >= 0 && i < (int)m_data->objects.size()) ? m_data->objects[i].get() : nullptr;
  }

  NodePtr Scene2::getNodePtr(int i) {
    return (i >= 0 && i < (int)m_data->objects.size()) ? m_data->objects[i] : nullptr;
  }

  int Scene2::getNodeCount() const { return (int)m_data->objects.size(); }

  // A node may also be a light (lights live in both vectors); drop it from the
  // light list too so removal can't leave a dangling light still shining.
  static void eraseLight(std::vector<std::shared_ptr<LightNode>> &lights, Node *node) {
    lights.erase(std::remove_if(lights.begin(), lights.end(),
                 [node](const auto &p) { return p.get() == node; }), lights.end());
  }

  void Scene2::removeNode(int i) {
    if (i >= 0 && i < (int)m_data->objects.size()) {
      eraseLight(m_data->lights, m_data->objects[i].get());
      m_data->objects.erase(m_data->objects.begin() + i);
      m_data->autoBoundsDirty = true;
    }
  }

  void Scene2::removeNode(Node *node) {
    eraseLight(m_data->lights, node);
    auto &o = m_data->objects;
    o.erase(std::remove_if(o.begin(), o.end(),
            [node](const auto &p) { return p.get() == node; }), o.end());
    m_data->autoBoundsDirty = true;
  }

  void Scene2::clear() {
    m_data->objects.clear();
    m_data->lights.clear();
    m_data->renderer.invalidateCache();
    m_data->autoBoundsDirty = true;
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

  std::shared_ptr<LightNode> Scene2::getLightPtr(int i) {
    return (i >= 0 && i < (int)m_data->lights.size()) ? m_data->lights[i] : nullptr;
  }

  int Scene2::getLightCount() const { return (int)m_data->lights.size(); }

  // Cameras
  void Scene2::addCamera(const geom::Camera &cam) {
    m_data->cameras.push_back(cam);
  }

  geom::Camera &Scene2::getCamera(int i) { return m_data->cameras.at(i); }
  const geom::Camera &Scene2::getCamera(int i) const { return m_data->cameras.at(i); }
  int Scene2::getCameraCount() const { return (int)m_data->cameras.size(); }

  // --- Driver update ---

  // Pre-order: run a node's drivers, then recurse into group children.
  static void syncNode(Node *node, double dt, double alpha) {
    if (!node) return;
    for (const auto &d : node->getDrivers()) d->sync(dt, alpha);
    if (auto *group = dynamic_cast<GroupNode*>(node)) {
      for (int i = 0; i < group->getChildCount(); i++) {
        syncNode(group->getChild(i), dt, alpha);
      }
    }
  }

  void Scene2::sync(double dt, double alpha) {
    std::scoped_lock guard(m_data->mutex);
    for (auto &node : m_data->objects) {
      syncNode(node.get(), dt, alpha);
    }
  }

  // Rendering
  Renderer &Scene2::getRenderer() { return m_data->renderer; }

  void Scene2::render(int cameraIndex) {
    std::scoped_lock guard(m_data->mutex);
    if (cameraIndex < 0 || cameraIndex >= (int)m_data->cameras.size()) return;

    const auto &cam = m_data->cameras[cameraIndex];

    // Apply configurable properties
    // (the property is registered as a 3-component core::Color, see the ctor;
    //  reading it into a Color4D makes AutoParse throw on the 3->4 mismatch)
    core::Color bg = prop("background color").value;
    glClearColor(bg[0]/255.f, bg[1]/255.f, bg[2]/255.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool wireframe = prop("wireframe").value;
    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    m_data->renderer.setLightingEnabled((bool)prop("enable lighting").value);

    // Debug visualization mode (menu order matches Renderer::setDebugMode codes)
    static const char *kDebugModes[] = {"shaded", "normals", "albedo", "UVs",
        "lighting", "NdotL", "SSR confidence", "depth", "SSR only"};
    std::string dbg = prop("debug").value;
    int dbgMode = 0;
    for (int i = 0; i < 9; i++) if (dbg == kDebugModes[i]) { dbgMode = i; break; }
    m_data->renderer.setDebugMode(dbgMode);

    // Update info properties (Info stores std::string — explicit str()
    // keeps the adapter's toString happy; direct int write would put an
    // int any into typed_value and break saveProperties).
    prop("info.Nodes").value  = utils::str((int)m_data->objects.size());
    prop("info.Lights").value = utils::str((int)m_data->lights.size());

    // Compute letterbox viewport to preserve camera aspect ratio
    GLint widgetVP[4];
    glGetIntegerv(GL_VIEWPORT, widgetVP);
    int ww = widgetVP[2], wh = widgetVP[3];

    const utils::Size &chip = cam.getRenderParams().chipSize;
    float camAR = (float)chip.width / chip.height;
    float widgetAR = (float)ww / std::max(wh, 1);
    int vpX = widgetVP[0], vpY = widgetVP[1], vpW = ww, vpH = wh;
    if (widgetAR > camAR) {
      vpW = (int)(wh * camAR);
      vpX += (ww - vpW) / 2;
    } else {
      vpH = (int)(ww / camAR);
      vpY += (wh - vpH) / 2;
    }
    glViewport(vpX, vpY, vpW, vpH);

    Mat viewGL = cam.getCSTransformationMatrixGL();
    Mat projGL = cam.getProjectionMatrixGL();

    m_data->renderer.render(m_data->objects, viewGL, projGL);

    // Restore state
    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glViewport(widgetVP[0], widgetVP[1], widgetVP[2], widgetVP[3]);
  }

  // GL callback for ICLQt integration
  std::shared_ptr<qt::GLCallback> Scene2::getGLCallback(int cameraIndex) {
    // Ensure enough callbacks exist
    while ((int)m_data->callbacks.size() <= cameraIndex) {
      m_data->callbacks.push_back(
          std::make_shared<SceneGLCallback>(this, (int)m_data->callbacks.size()));
    }
    return m_data->callbacks[cameraIndex];
  }

  // --- Mouse handler ---

  Scene2MouseHandler *Scene2::getMouseHandler(int cameraIndex) {
    while ((int)m_data->mouseHandlers.size() <= cameraIndex) {
      int idx = (int)m_data->mouseHandlers.size();
      auto h = std::make_unique<Scene2MouseHandler>(idx, this);
      h->setSensitivities(10.0f);  // translation multiplier; scene size comes from getBounds()
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

  // Accumulate the world-space AABB of all visible geometry under `node`.
  static void accumulateBounds(Node *node, Vec &lo, Vec &hi, bool &any) {
    if (!node || !node->isVisible()) return;
    if (auto *group = dynamic_cast<GroupNode*>(node))
      for (int i = 0; i < group->getChildCount(); i++)
        accumulateBounds(group->getChild(i), lo, hi, any);
    if (auto *geom = dynamic_cast<GeometryNode*>(node)) {
      const auto &verts = geom->getVertices();
      if (verts.empty()) return;
      const Mat xform = node->getTransformation(true);
      for (const auto &v : verts) {
        const Vec w = xform * v;
        for (int k = 0; k < 3; k++) { lo[k] = std::min(lo[k], w[k]); hi[k] = std::max(hi[k], w[k]); }
      }
      any = true;
    }
  }

  // Explicit override if setBounds(>0) was called; otherwise the scene's own
  // size, computed from geometry and cached (recomputed only when the node set
  // changes — NOT every frame, so a dynamic scene's moving vertices don't churn
  // the camera sensitivity).
  void Scene2::setBounds(float maxDim) { m_data->explicitBounds = maxDim; }

  float Scene2::getBounds() const {
    if (m_data->explicitBounds > 0) return m_data->explicitBounds;
    if (m_data->autoBoundsDirty) {
      std::lock_guard<std::recursive_mutex> lock(m_data->mutex);
      Vec lo(1e30f,1e30f,1e30f,1), hi(-1e30f,-1e30f,-1e30f,1);
      bool any = false;
      for (auto &n : m_data->objects) accumulateBounds(n.get(), lo, hi, any);
      float maxDim = any ? std::max({hi[0]-lo[0], hi[1]-lo[1], hi[2]-lo[2]}) : 0.0f;
      m_data->autoBounds = maxDim > 1e-3f ? maxDim : 1000.0f;   // sane fallback for empty scenes
      m_data->autoBoundsDirty = false;
    }
    return m_data->autoBounds;
  }

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
