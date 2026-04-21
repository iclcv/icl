// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/DrawWidget3D.h>
#include <icl/qt/GLImg.h>

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <mutex>
#endif

using namespace icl::utils;

namespace icl::qt {
  ICLDrawWidget3D::ICLDrawWidget3D(QWidget *parent):
    ICLDrawWidget(parent),m_linkedCallback(0){
  }

  void ICLDrawWidget3D::customPaintEvent(PaintEngine *e){
    std::scoped_lock<std::recursive_mutex> lock(m_linkMutex);

    GLint profileMask = 0;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
    bool coreProfile = (profileMask & GL_CONTEXT_CORE_PROFILE_BIT) != 0;

    if (coreProfile) {
      // Core Profile two-phase rendering:
      //   Phase 1 (e==nullptr): run GL callback only
      //   Phase 2 (e!=nullptr): run 2D draw commands only
      if (!e) {
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        if (m_linkedCallback) {
          m_linkedCallback->draw(this);
        }
      } else {
        ICLDrawWidget::customPaintEvent(e);
      }
      return;
    }

    // Legacy compatibility profile
    glClear(GL_DEPTH_BUFFER_BIT);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    gluPerspective( 45,  float(width())/height(), 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0, 0, -1,   // pos
              0, 0,  0,   // view center point
              1, 0,  0 );// up vector

    if(m_linkedCallback){
      m_linkedCallback->draw(this);
    }

    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    ICLDrawWidget::customPaintEvent(e);
  }

  void ICLDrawWidget3D::link(ICLDrawWidget3D::GLCallback *cb){
    std::scoped_lock<std::recursive_mutex> lock(m_linkMutex);

    if(cb == m_linkedCallback) return;

    if(m_linkedCallback){
      m_linkedCallback->unlink(this);
    }

    m_linkedCallback = cb;

    if(m_linkedCallback){
      m_linkedCallback->link(this);
    }
  }

  } // namespace icl::qt
