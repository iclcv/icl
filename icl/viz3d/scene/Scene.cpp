// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/scene/Scene.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/core/prop/Constraints.h>
#include <icl/viz3d/scene/SceneMouseHandler.h>
#include <icl/viz3d/nodes/GroupNode.h>
#include <icl/viz3d/nodes/GeometryNode.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/TextNode.h>
#include <icl/viz3d/scene/Driver.h>
#include <icl/viz3d/pointcloud/PointCloud.h>
#include <icl/viz3d/render/BVH.h>
#include <icl/viz3d/render/GLRenderBackend.h>
#ifdef ICL_HAVE_FILAMENT
#include <icl/viz3d/render/detail/FilamentRenderBackend.h>
#endif
#include <icl/cv3d/Camera.h>
#include <icl/cv3d/ViewRay.h>
#include <icl/viz3d/render/Material.h>
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
#include <atomic>
#include <cmath>
#include <cstdlib>

namespace icl::viz3d {

  // ---- GLCallback implementation ----
  //
  // Implementation-internal subclass of `qt::GLCallback`; not
  // exposed in Scene.h.  Scene::getGLCallback() returns a
  // `shared_ptr<qt::GLCallback>` (base) so callers never see this
  // type.
  namespace {
    struct SceneGLCallback : public qt::GLCallback {
      Scene *scene;
      int camIndex;
      bool needLink = true;
      qt::GUI *gui = nullptr;

      SceneGLCallback(Scene *s, int ci) : scene(s), camIndex(ci) {}

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

  // ---- Render-backend selection + onscreen compositing ----

  namespace {
    // Filament is the default real-time backend when built + the Metal engine
    // initialises; the GL backend is the legacy fallback (also forced by
    // ICL_VIZ3D_BACKEND=gl, or used when Filament is unavailable).
    std::unique_ptr<RenderBackend> makeRenderBackend() {
      const char *env = std::getenv("ICL_VIZ3D_BACKEND");
      const bool forceGL = env && std::string(env) == "gl";
#ifdef ICL_HAVE_FILAMENT
      if (!forceGL) {
        auto fb = std::make_unique<FilamentRenderBackend>();
        if (fb->isValid()) return fb;   // upcasts to unique_ptr<RenderBackend>
      }
#endif
      (void)forceGL;
      return std::make_unique<GLRenderBackend>();
    }

#ifdef ICL_HAVE_OPENGL
    // Composites an image-producing backend's frame (e.g. Filament's Metal
    // render) into the current GL framebuffer over a viewport rect — a textured
    // fullscreen quad, no depth. This is the interim "A1" transport (GPU→CPU→GPU
    // readback + upload); the zero-copy shared-texture path is a later step.
    // The 2D annotation layer (ICLDrawWidget) still paints on top afterwards, so
    // the existing 2D pipeline is untouched.
    struct GLBlitter {
      unsigned int prog = 0, vao = 0, tex = 0;
      std::vector<unsigned char> rgb;   // planar→interleaved upload scratch

      void ensureGL() {
        if (prog) return;
        auto compile = [](GLenum t, const char *src) {
          GLuint s = glCreateShader(t);
          glShaderSource(s, 1, &src, nullptr); glCompileShader(s); return s;
        };
        const char *vs =
            "#version 330 core\n"
            "out vec2 uv;\n"
            "void main(){ vec2 q = vec2((gl_VertexID & 1) * 2 - 1, (gl_VertexID >> 1) * 2 - 1);\n"
            "  uv = vec2((q.x + 1.0) * 0.5, (1.0 - q.y) * 0.5);\n"   // row 0 = image top
            "  gl_Position = vec4(q, 0.0, 1.0); }\n";
        const char *fs =
            "#version 330 core\n"
            "in vec2 uv; out vec4 c; uniform sampler2D img;\n"
            "void main(){ c = vec4(texture(img, uv).rgb, 1.0); }\n";
        GLuint v = compile(GL_VERTEX_SHADER, vs), f = compile(GL_FRAGMENT_SHADER, fs);
        prog = glCreateProgram();
        glAttachShader(prog, v); glAttachShader(prog, f); glLinkProgram(prog);
        glDeleteShader(v); glDeleteShader(f);
        glGenVertexArrays(1, &vao);
        glGenTextures(1, &tex);
      }

      void blit(const core::Img8u &img, int x, int y, int w, int h) {
        const int iw = img.getWidth(), ih = img.getHeight();
        if (iw <= 0 || ih <= 0) return;
        ensureGL();
        rgb.resize(size_t(iw) * ih * 3);
        const icl8u *rp = img.begin(0), *gp = img.begin(1), *bp = img.begin(2);
        for (size_t i = 0, n = size_t(iw) * ih; i < n; ++i) {
          rgb[i * 3] = rp[i]; rgb[i * 3 + 1] = gp[i]; rgb[i * 3 + 2] = bp[i];
        }

        // Save EVERY GL state this touches — the ICLDrawWidget 2D layer paints
        // afterwards in the same context and must find the state pristine. A left
        // GL_UNPACK_ALIGNMENT=1, a bound texture, an active program, etc. corrupts
        // the 2D glyph uploads (garbled overlay text).
        GLint pProg = 0, pVAO = 0, pTex = 0, pActive = 0, pAlign = 4, pVP[4] = {0, 0, 0, 0};
        GLint pArrBuf = 0;
        GLboolean depthWas = glIsEnabled(GL_DEPTH_TEST);
        GLboolean blendWas = glIsEnabled(GL_BLEND);
        glGetIntegerv(GL_CURRENT_PROGRAM, &pProg);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &pVAO);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &pArrBuf);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &pActive);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &pAlign);
        glGetIntegerv(GL_VIEWPORT, pVP);
        glActiveTexture(GL_TEXTURE0);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &pTex);

        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iw, ih, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);   // never wireframe the blit quad
        glViewport(x, y, w, h);
        glUseProgram(prog);
        glUniform1i(glGetUniformLocation(prog, "img"), 0);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Restore all captured state.
        glBindVertexArray(pVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pArrBuf);
        glBindTexture(GL_TEXTURE_2D, (GLuint)pTex);
        glActiveTexture((GLenum)pActive);
        glUseProgram((GLuint)pProg);
        glPixelStorei(GL_UNPACK_ALIGNMENT, pAlign);
        glViewport(pVP[0], pVP[1], pVP[2], pVP[3]);
        if (depthWas) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (blendWas) glEnable(GL_BLEND); else glDisable(GL_BLEND);
      }
    };
#endif
  }  // namespace

  // ---- Data ----

  struct Scene::Data {
    std::vector<std::shared_ptr<Node>> objects;
    std::vector<std::shared_ptr<LightNode>> lights;  // also in objects, tracked for fast access
    std::vector<cv3d::Camera> cameras;
    std::unique_ptr<RenderBackend> renderer = makeRenderBackend();
    std::vector<std::shared_ptr<SceneGLCallback>> callbacks;
    std::vector<std::unique_ptr<SceneMouseHandler>> mouseHandlers;
    // "show cameras" overlay: one lightweight gizmo per camera (3 axis lines +
    // a billboard label, lazy) + a scratch list that appends them to the
    // objects for a render pass.
    std::vector<std::shared_ptr<GroupNode>> cameraFrames;
    std::vector<std::shared_ptr<Node>> renderScratch;
    Vec cursor{0, 0, 0, 1};
    float explicitBounds = -1.0f;          // >0: user override (setBounds); else auto
    mutable float autoBounds = 1000.0f;    // cached scene-derived size
    mutable bool autoBoundsDirty = true;   // recompute lazily on structure change only
    std::atomic<unsigned> version{0};      // bumped by touch(); polled by renderers
    std::recursive_mutex mutex;
#ifdef ICL_HAVE_OPENGL
    // Offscreen capture FBO (renderToImage), lazily (re)allocated per size.
    // GL handles are leaked at process exit if no context is current at dtor —
    // acceptable for a process-lifetime resource (legacy cv3d::Scene did the same).
    unsigned int captureFBO = 0, captureColorRBO = 0, captureDepthRBO = 0;
    utils::Size captureSize{0, 0};
    // Composites an image-producing backend (Filament) into the widget FBO.
    std::unique_ptr<GLBlitter> blitter;
#endif
  };

  // ---- Scene implementation ----

  void Scene::lock() { m_data->mutex.lock(); }
  void Scene::unlock() { m_data->mutex.unlock(); }

  Scene::Scene() : m_data(std::make_unique<Data>()) {
    addProperty("background color", core::prop::Color{}, core::Color(0,0,0));
    addProperty("wireframe",utils::prop::Flag{}, false);
    addProperty("show cameras",utils::prop::Flag{}, false);
    addProperty("enable lighting",utils::prop::Flag{}, true);
    addProperty("debug", utils::prop::Menu{"shaded", "normals", "albedo", "UVs",
                "lighting", "NdotL", "SSR confidence", "depth", "SSR only"}, "shaded");
    addProperty("point size",utils::prop::Range{.min=1, .max=20}, 3);
    addProperty("info.Nodes",utils::prop::Info{}, utils::str(0));
    addProperty("info.Lights",utils::prop::Info{}, utils::str(0));
  }
  Scene::~Scene() = default;

  void Scene::addNode(NodePtr node) {
    node->setScene(this);                 // back-pointer (subtree, for a GroupNode)
    m_data->objects.push_back(std::move(node));
    // touch() bumps the version + drops the renderer cache. The latter matters
    // because the Renderer caches geometry/textures keyed by raw Node*/Material*:
    // a new node may reuse a just-freed address (and a fresh material starts at
    // the same version), so a stale cache entry would otherwise suppress the
    // upload and the new geometry/texture would never show.
    touch();
  }

  Node *Scene::getNode(int i) {
    return (i >= 0 && i < (int)m_data->objects.size()) ? m_data->objects[i].get() : nullptr;
  }

  const Node *Scene::getNode(int i) const {
    return (i >= 0 && i < (int)m_data->objects.size()) ? m_data->objects[i].get() : nullptr;
  }

  NodePtr Scene::getNodePtr(int i) {
    return (i >= 0 && i < (int)m_data->objects.size()) ? m_data->objects[i] : nullptr;
  }

  int Scene::getNodeCount() const { return (int)m_data->objects.size(); }

  // A node may also be a light (lights live in both vectors); drop it from the
  // light list too so removal can't leave a dangling light still shining.
  static void eraseLight(std::vector<std::shared_ptr<LightNode>> &lights, Node *node) {
    lights.erase(std::remove_if(lights.begin(), lights.end(),
                 [node](const auto &p) { return p.get() == node; }), lights.end());
  }

  void Scene::removeNode(int i) {
    if (i >= 0 && i < (int)m_data->objects.size()) {
      Node *n = m_data->objects[i].get();
      eraseLight(m_data->lights, n);
      n->setScene(nullptr);                 // clear back-pointer before erase
      m_data->objects.erase(m_data->objects.begin() + i);
      touch();                              // bump version + drop stale cache (see addNode)
    }
  }

  void Scene::removeNode(Node *node) {
    if (node) node->setScene(nullptr);
    eraseLight(m_data->lights, node);
    auto &o = m_data->objects;
    o.erase(std::remove_if(o.begin(), o.end(),
            [node](const auto &p) { return p.get() == node; }), o.end());
    touch();                                // bump version + drop stale cache (see addNode)
  }

  void Scene::clear() {
    for (auto &n : m_data->objects) n->setScene(nullptr);
    m_data->objects.clear();
    m_data->lights.clear();
    touch();
  }

  void Scene::touch() {
    m_data->version.fetch_add(1, std::memory_order_relaxed);
    m_data->autoBoundsDirty = true;
    m_data->renderer->invalidateCache();
  }

  unsigned Scene::sceneVersion() const {
    return m_data->version.load(std::memory_order_relaxed);
  }

  // Lights
  void Scene::addLight(std::shared_ptr<LightNode> light) {
    m_data->lights.push_back(light);
    addNode(std::static_pointer_cast<Node>(light));
  }

  LightNode *Scene::getLight(int i) {
    return (i >= 0 && i < (int)m_data->lights.size()) ? m_data->lights[i].get() : nullptr;
  }

  const LightNode *Scene::getLight(int i) const {
    return (i >= 0 && i < (int)m_data->lights.size()) ? m_data->lights[i].get() : nullptr;
  }

  std::shared_ptr<LightNode> Scene::getLightPtr(int i) {
    return (i >= 0 && i < (int)m_data->lights.size()) ? m_data->lights[i] : nullptr;
  }

  int Scene::getLightCount() const { return (int)m_data->lights.size(); }

  // Cameras
  void Scene::addCamera(const cv3d::Camera &cam) {
    m_data->cameras.push_back(cam);
  }

  cv3d::Camera &Scene::getCamera(int i) { return m_data->cameras.at(i); }
  const cv3d::Camera &Scene::getCamera(int i) const { return m_data->cameras.at(i); }
  int Scene::getCameraCount() const { return (int)m_data->cameras.size(); }

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

  void Scene::sync(double dt, double alpha) {
    std::scoped_lock guard(m_data->mutex);
    for (auto &node : m_data->objects) {
      syncNode(node.get(), dt, alpha);
    }
  }

  // Rendering
  RenderBackend &Scene::getRenderer() { return *m_data->renderer; }

  // A lightweight camera gizmo (built in the camera's local frame, so the group
  // transform = camera pose places it): 3 axis lines (RGB = XYZ), a stylized,
  // short view frustum, and a billboard label. Frustum corners come from the
  // camera's corner view rays mapped into local space — convention-independent.
  static std::shared_ptr<GroupNode> makeCameraGizmo(int index, float len,
                                                    const cv3d::Camera &cam) {
    auto g = std::make_shared<GroupNode>();
    auto m = std::make_shared<MeshNode>();

    // axes (apex = vertex 0)
    m->addVertex(Vec(0, 0, 0, 1));
    m->addVertex(Vec(len, 0, 0, 1));
    m->addVertex(Vec(0, len, 0, 1));
    m->addVertex(Vec(0, 0, len, 1));
    m->addLine(0, 1, cv3d::GeomColor(255, 0, 0, 255));   // X red  (colors are 0..255)
    m->addLine(0, 2, cv3d::GeomColor(0, 255, 0, 255));   // Y green
    m->addLine(0, 3, cv3d::GeomColor(0, 0, 255, 255));   // Z blue

    // stylized frustum: corner rays at a fixed short depth (not the far clip)
    const float fd = len * 2.0f;
    const cv3d::GeomColor fc(255, 210, 80, 255);         // soft yellow
    const Mat cs = cam.getCSTransformationMatrix();      // world -> cam (rotation)
    const utils::Size s = cam.getResolution();
    auto cornerLocal = [&](float px, float py) -> Vec {
      Vec wd = cam.getViewRay(utils::Point32f(px, py)).direction; wd[3] = 0;
      Vec ld = cs * wd;                                  // local-space direction
      float n = std::sqrt(ld[0]*ld[0] + ld[1]*ld[1] + ld[2]*ld[2]);
      if (n < 1e-6f) n = 1.0f;
      return Vec(ld[0]*fd/n, ld[1]*fd/n, ld[2]*fd/n, 1); // distance fd along the ray
    };
    const int b = 4;
    m->addVertex(cornerLocal(0, 0));                 // 4 top-left
    m->addVertex(cornerLocal(s.width, 0));           // 5 top-right
    m->addVertex(cornerLocal(s.width, s.height));    // 6 bottom-right
    m->addVertex(cornerLocal(0, s.height));          // 7 bottom-left
    for (int k = 0; k < 4; ++k) m->addLine(0, b + k, fc);            // apex → corners
    m->addLine(b+0, b+1, fc); m->addLine(b+1, b+2, fc);             // image rectangle
    m->addLine(b+2, b+3, fc); m->addLine(b+3, b+0, fc);

    m->setLineWidth(3.0f);          // thick lines via the geometry-shader path
    m->setRenderOnTop(true);        // gizmo overlays geometry — never occluded
    g->addChild(m);

    auto label = TextNode::create("cam " + utils::str(index), len * 0.6f);
    label->setBillboard(true);
    label->translate(0, 0, len * 0.15f);
    g->addChild(label);
    return g;
  }

  const std::vector<std::shared_ptr<Node>> &Scene::nodesToRender(int activeCam) {
    if (!(bool)prop("show cameras").value) return m_data->objects;

    // Lazily create one gizmo per camera, sized to the scene.
    const float len = std::max(25.0f, getBounds() * 0.10f);
    while (m_data->cameraFrames.size() < m_data->cameras.size()) {
      const int i = (int)m_data->cameraFrames.size();
      m_data->cameraFrames.push_back(makeCameraGizmo(i, len, m_data->cameras[i]));
    }

    auto &out = m_data->renderScratch;
    out.assign(m_data->objects.begin(), m_data->objects.end());
    for (int i = 0; i < (int)m_data->cameras.size(); ++i) {
      if (i == activeCam) continue;   // don't draw the frame we're looking through
      auto &f = m_data->cameraFrames[i];
      f->setTransformation(m_data->cameras[i].getInvCSTransformationMatrix());
      out.push_back(std::static_pointer_cast<Node>(f));
    }
    return out;
  }

  void Scene::render(int cameraIndex) {
    std::scoped_lock guard(m_data->mutex);
    if (cameraIndex < 0 || cameraIndex >= (int)m_data->cameras.size()) return;

    const auto &cam = m_data->cameras[cameraIndex];

    // Apply configurable properties
    // (the property is registered as a 3-component core::Color, see the ctor;
    //  reading it into a Color4D makes AutoParse throw on the 3->4 mismatch)
    core::Color bg = prop("background color").value;
    glClearColor(bg[0]/255.f, bg[1]/255.f, bg[2]/255.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Wireframe is a GL-only mode; only enable it for the in-place GL backend.
    bool wireframe = prop("wireframe").value;
    if (wireframe && !m_data->renderer->producesImage())
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    m_data->renderer->setLightingEnabled((bool)prop("enable lighting").value);

    // Debug visualization mode (menu order matches Renderer::setDebugMode codes)
    static const char *kDebugModes[] = {"shaded", "normals", "albedo", "UVs",
        "lighting", "NdotL", "SSR confidence", "depth", "SSR only"};
    std::string dbg = prop("debug").value;
    int dbgMode = 0;
    for (int i = 0; i < 9; i++) if (dbg == kDebugModes[i]) { dbgMode = i; break; }
    m_data->renderer->setDebugMode(dbgMode);

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
    Mat viewGL = cam.getCSTransformationMatrixGL();
    Mat projGL = cam.getProjectionMatrixGL();

    if (m_data->renderer->producesImage()) {
      // Filament (or any image-producing backend): render at the letterbox size,
      // then composite the frame into the widget FBO. The 2D annotation layer
      // (ICLDrawWidget) still paints on top afterwards. Interim A1 transport
      // (readback + upload) — the zero-copy shared-texture path is a later step.
      m_data->renderer->setTargetSize({vpW, vpH});
      m_data->renderer->render(nodesToRender(cameraIndex), viewGL, projGL);
      core::Img8u frame;
      if (m_data->renderer->readColor(frame)) {
        if (!m_data->blitter) m_data->blitter = std::make_unique<GLBlitter>();
        m_data->blitter->blit(frame, vpX, vpY, vpW, vpH);
      }
    } else {
      glViewport(vpX, vpY, vpW, vpH);
      m_data->renderer->render(nodesToRender(cameraIndex), viewGL, projGL);
    }

    // Restore state
    if (wireframe && !m_data->renderer->producesImage())
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glViewport(widgetVP[0], widgetVP[1], widgetVP[2], widgetVP[3]);
  }

  BVH::ImageResult Scene::renderToImage(int cameraIndex, BVH::DepthMode mode) {
    BVH::ImageResult result;

    // Image-producing backend (Filament): render straight to an image — no GL
    // context needed, so this works headless. Depth readback is a follow-up, so
    // the returned depth stays empty for now (colour parity first).
    if (m_data->renderer->producesImage()) {
      std::scoped_lock guard(m_data->mutex);
      if (cameraIndex < 0 || cameraIndex >= (int)m_data->cameras.size()) return result;
      const cv3d::Camera &cam = m_data->cameras[cameraIndex];
      const utils::Size s = cam.getResolution();
      if (s.width <= 0 || s.height <= 0) return result;
      m_data->renderer->setSSREnabled(false);
      m_data->renderer->setLightingEnabled((bool)prop("enable lighting").value);
      m_data->renderer->setTargetSize(s);
      m_data->renderer->render(nodesToRender(cameraIndex),
                              cam.getCSTransformationMatrixGL(),
                              cam.getProjectionMatrixGL());
      m_data->renderer->readColor(result.image);
      return result;
    }

#ifdef ICL_HAVE_OPENGL
    std::scoped_lock guard(m_data->mutex);
    if (cameraIndex < 0 || cameraIndex >= (int)m_data->cameras.size()) return result;

    const cv3d::Camera &cam = m_data->cameras[cameraIndex];
    const utils::Size s = cam.getResolution();
    const int w = s.width, h = s.height;
    if (w <= 0 || h <= 0) return result;

    // Save the caller's framebuffer + viewport so this is composable inside an
    // on-screen draw callback (a QOpenGLWidget's default FBO is NOT 0 — forcing
    // 0 would blank the widget). Restored before returning.
    GLint prevFBO = 0, prevVP[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
    glGetIntegerv(GL_VIEWPORT, prevVP);

    // (re)allocate the capture FBO when the size changes (color + depth RBOs)
    if (!m_data->captureFBO || m_data->captureSize != s) {
      if (m_data->captureFBO) {
        glDeleteFramebuffers(1, &m_data->captureFBO);
        glDeleteRenderbuffers(1, &m_data->captureColorRBO);
        glDeleteRenderbuffers(1, &m_data->captureDepthRBO);
      }
      glGenFramebuffers(1, &m_data->captureFBO);
      glGenRenderbuffers(1, &m_data->captureColorRBO);
      glGenRenderbuffers(1, &m_data->captureDepthRBO);
      glBindFramebuffer(GL_FRAMEBUFFER, m_data->captureFBO);
      glBindRenderbuffer(GL_RENDERBUFFER, m_data->captureColorRBO);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_RENDERBUFFER, m_data->captureColorRBO);
      glBindRenderbuffer(GL_RENDERBUFFER, m_data->captureDepthRBO);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_RENDERBUFFER, m_data->captureDepthRBO);
      m_data->captureSize = s;
    } else {
      glBindFramebuffer(GL_FRAMEBUFFER, m_data->captureFBO);
    }
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
      return result;
    }

    // Force SSR off so the geometry pass writes depth straight into our FBO.
    // (With SSR on, geometry depth lands in an internal ping-pong FBO and only
    //  color is blitted back — our depth attachment would read back cleared.)
    const bool prevSSR = m_data->renderer->isSSREnabled();
    m_data->renderer->setSSREnabled(false);
    m_data->renderer->setDebugMode(0);   // shaded — ignore the live "debug" prop
    m_data->renderer->setLightingEnabled((bool)prop("enable lighting").value);

    glViewport(0, 0, w, h);
    core::Color bg = prop("background color").value;
    glClearColor(bg[0]/255.f, bg[1]/255.f, bg[2]/255.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_data->renderer->render(nodesToRender(cameraIndex),
                            cam.getCSTransformationMatrixGL(),
                            cam.getProjectionMatrixGL());

    // The renderer restores the FBO binding it found (captureFBO), but rebind
    // defensively so the readback can never read a stray framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, m_data->captureFBO);

    // ---- color readback (RGBA8, GL bottom-up) → planar RGB Img8u (top-down) ----
    std::vector<icl8u> rgba((size_t)w * h * 4);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    result.image = core::Img8u(s, core::formatRGB);
    icl8u *R = result.image.getData(0), *G = result.image.getData(1), *B = result.image.getData(2);
    for (int y = 0; y < h; ++y) {
      const icl8u *row = &rgba[(size_t)(h - 1 - y) * w * 4];   // vertical flip
      const size_t o = (size_t)y * w;
      for (int x = 0; x < w; ++x) { R[o+x] = row[4*x]; G[o+x] = row[4*x+1]; B[o+x] = row[4*x+2]; }
    }

    // ---- depth readback → linearized metric mm (mirrors legacy cv3d::Scene) ----
    if (mode != BVH::NoDepth) {
      std::vector<float> z((size_t)w * h);
      glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, z.data());

      const float zNear = cam.getRenderParams().clipZNear;
      const float zFar  = cam.getRenderParams().clipZFar;
      const float Q = zFar / (zFar - zNear);
      const float A = (zFar - zNear) / zFar;
      const float b = zNear;

      // optional per-pixel 1/cos(angle-to-center-ray) → DistToCamCenter
      std::vector<float> corr;
      if (mode == BVH::DistToCamCenter) {
        corr.resize((size_t)w * h);
        utils::Array2D<cv3d::ViewRay> vr = cam.getAllViewRays();
        const Vec c = vr(w/2 - 1, h/2 - 1).direction;
        const float cn = std::sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
        for (int i = 0; i < w*h; ++i) {
          const Vec &dr = vr[i].direction;
          const float dn = std::sqrt(dr[0]*dr[0] + dr[1]*dr[1] + dr[2]*dr[2]);
          const float cosA = (dr[0]*c[0] + dr[1]*c[1] + dr[2]*c[2]) / (dn * cn);
          corr[i] = 1.0f / cosA;
        }
      }

      result.depth = core::Img32f(s, 1);
      float *d = result.depth.getData(0);
      for (int y = 0; y < h; ++y) {
        const float *zr = &z[(size_t)(h - 1 - y) * w];   // vertical flip
        const size_t o = (size_t)y * w;
        for (int x = 0; x < w; ++x) {
          const float plane = A / (Q - zr[x]) + b - 1;
          d[o+x] = (mode == BVH::DistToCamCenter) ? corr[o+x] * (plane + 1) - 1 : plane;
        }
      }
    }

    m_data->renderer->setSSREnabled(prevSSR);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
    glViewport(prevVP[0], prevVP[1], prevVP[2], prevVP[3]);
#else
    (void)cameraIndex; (void)mode;
#endif
    return result;
  }

  // GL callback for ICLQt integration
  std::shared_ptr<qt::GLCallback> Scene::getGLCallback(int cameraIndex) {
    // Ensure enough callbacks exist
    while ((int)m_data->callbacks.size() <= cameraIndex) {
      m_data->callbacks.push_back(
          std::make_shared<SceneGLCallback>(this, (int)m_data->callbacks.size()));
    }
    return m_data->callbacks[cameraIndex];
  }

  // --- Mouse handler ---

  SceneMouseHandler *Scene::getMouseHandler(int cameraIndex) {
    while ((int)m_data->mouseHandlers.size() <= cameraIndex) {
      int idx = (int)m_data->mouseHandlers.size();
      auto h = std::make_unique<SceneMouseHandler>(idx, this);
      h->setSensitivities(10.0f);  // translation multiplier; scene size comes from getBounds()
      m_data->mouseHandlers.push_back(std::move(h));
    }
    return m_data->mouseHandlers[cameraIndex].get();
  }

  // --- Hit testing ---

  // Recursive hit collection against viz3d node graph
  static void collectHits(Node *node, const cv3d::ViewRay &ray, std::vector<Hit2> &hits) {
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
        if (result == cv3d::ViewRay::foundIntersection) {
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
          if (result == cv3d::ViewRay::foundIntersection) {
            Vec d = ip - ray.offset;
            float dist = std::sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
            hits.push_back({node, ip, dist});
            break;  // one hit per quad is enough
          }
        }
      }
    }
  }

  Hit2 Scene::findObject(const cv3d::ViewRay &ray) const {
    std::vector<Hit2> hits;
    for (auto &node : m_data->objects) {
      collectHits(node.get(), ray, hits);
    }
    return hits.empty() ? Hit2() : *std::min_element(hits.begin(), hits.end());
  }

  std::vector<Hit2> Scene::findObjects(const cv3d::ViewRay &ray) const {
    std::vector<Hit2> hits;
    for (auto &node : m_data->objects) {
      collectHits(node.get(), ray, hits);
    }
    std::sort(hits.begin(), hits.end());
    return hits;
  }

  Hit2 Scene::findObject(int cameraIndex, int x, int y) const {
    return findObject(getCamera(cameraIndex).getViewRay(utils::Point32f(x, y)));
  }

  // --- Cursor ---

  void Scene::setCursor(const Vec &pos) { m_data->cursor = pos; }
  Vec Scene::getCursor() const { return m_data->cursor; }

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
  void Scene::setBounds(float maxDim) { m_data->explicitBounds = maxDim; }

  float Scene::getBounds() const {
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

  BVH Scene::buildBVH() const {
    std::vector<PreparedGeom> geoms;
    for (auto &node : m_data->objects) {
      collectPreparedGeom(node.get(), geoms);
    }
    return buildSceneBVH(geoms);
  }

} // namespace icl::viz3d
