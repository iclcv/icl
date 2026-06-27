// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/SceneCapture.h>
#include <icl/geom2/Scene2.h>
#include <icl/utils/Macros.h>
#include <algorithm>

#ifdef ICL_HAVE_QT
#include <GL/glew.h>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QSurfaceFormat>
#endif

namespace icl::geom2 {

  BVHSceneCapture::BVHSceneCapture() = default;
  BVHSceneCapture::~BVHSceneCapture() = default;

  void BVHSceneCapture::setSubsampling(int stepX, int stepY) {
    m_stepX = std::max(1, stepX);
    m_stepY = std::max(1, stepY);
  }

  void BVHSceneCapture::setCaching(bool enabled) {
    m_caching = enabled;
    m_dirty = true;   // force a (re)build on the next capture either way
  }

  void BVHSceneCapture::invalidate() { m_dirty = true; }

  BVH::ImageResult BVHSceneCapture::capture(Scene2 &scene, int cameraIndex,
                                            BVH::DepthMode mode) {
    if (cameraIndex < 0 || cameraIndex >= scene.getCameraCount()) return {};
    if (m_caching) {
      // Build once; reuse until the caller invalidate()s. Rebuilding the BVH per
      // frame is the dominant cost for a static scene — caching removes it.
      if (m_dirty) { m_bvh = scene.buildBVH(); m_dirty = false; }
      return m_bvh.raycastToImage(scene.getCamera(cameraIndex), mode, m_stepX, m_stepY);
    }
    BVH bvh = scene.buildBVH();
    return bvh.raycastToImage(scene.getCamera(cameraIndex), mode, m_stepX, m_stepY);
  }

  // Owned offscreen GL context (ported from legacy geom::Scene::PBuffer).
  // Lazily created in (and thread-bound to) the thread of the first capture().
  struct GLSceneCapture::OffscreenContext {
#ifdef ICL_HAVE_QT
    // Heap-owned and lazily constructed inside makeCurrent() so the
    // QOpenGLContext's thread affinity is the *calling* (worker) thread, not
    // whatever thread happened to construct the GLSceneCapture.
    std::unique_ptr<QOpenGLContext>    context;
    std::unique_ptr<QOffscreenSurface> surface;

    ~OffscreenContext() {
      // A global / late-destroyed GLSceneCapture is torn down at static-
      // destruction time — often on a thread whose Qt thread-local state (and
      // the QApplication itself) is already gone. doneCurrent() / ~QOpenGLContext
      // then dereference dead QThreadStorage and crash (seen as a fault in
      // ~GLSceneCapture at process exit). Intentionally LEAK the GL context +
      // surface; the OS reclaims them at process exit. (No-op when never used —
      // the pointers are null until the first capture().)
      (void)context.release();
      (void)surface.release();
    }

    /// makeCurrent this context (creating it on first use). Returns success.
    bool makeCurrent() {
      if (!context) {
        surface.reset(new QOffscreenSurface);
        surface->setFormat(QSurfaceFormat::defaultFormat());
        surface->create();
        context.reset(new QOpenGLContext);
        // Share lists with the global context so textures/materials uploaded
        // by an on-screen widget are visible here (no-op if none exists).
        context->setShareContext(QOpenGLContext::globalShareContext());
        context->setFormat(QSurfaceFormat::defaultFormat());
        if (!context->create()) { context.reset(); return false; }
      }
      if (!context->makeCurrent(surface.get())) return false;
      // ICL's Renderer reaches FBO/VAO/shader entry points through GLEW. The
      // on-screen path initialises GLEW in ICLWidget::paintGL; an owned
      // offscreen context has no widget, so initialise it once here — otherwise
      // the first glGenFramebuffers (a null GLEW pointer) segfaults. Core-profile
      // contexts need glewExperimental so the function pointers resolve.
      static bool glewReady = false;
      if (!glewReady) {
        glewExperimental = GL_TRUE;
        glewInit();
        glGetError();   // swallow the benign INVALID_ENUM glewInit leaves behind
        glewReady = true;
      }
      return true;
    }
    void doneCurrent() { context->doneCurrent(); }
#endif
  };

  GLSceneCapture::GLSceneCapture(bool ownContext)
    : m_ctx(ownContext ? new OffscreenContext : nullptr) {}

  GLSceneCapture::~GLSceneCapture() = default;

  BVH::ImageResult GLSceneCapture::capture(Scene2 &scene, int cameraIndex,
                                           BVH::DepthMode mode) {
#ifdef ICL_HAVE_QT
    if (m_ctx) {
      if (!m_ctx->makeCurrent()) return {};
      BVH::ImageResult r = scene.renderToImage(cameraIndex, mode);
      m_ctx->doneCurrent();
      return r;
    }
#endif
    // Borrowed-context mode: render straight on the caller's current context.
    return scene.renderToImage(cameraIndex, mode);
  }

} // namespace icl::geom2
