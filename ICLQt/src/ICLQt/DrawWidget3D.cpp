/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DrawWidget3D.cpp                       **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLQt/DrawWidget3D.h>
#include <ICLQt/GLImg.h>

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif ICL_SYSTEM_WINDOWS
#define NOMINMAX
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

using namespace icl::utils;

namespace icl{
  namespace qt{
    ICLDrawWidget3D::ICLDrawWidget3D(QWidget *parent):
      ICLDrawWidget(parent),m_linkedCallback(0){
    }
    
    void ICLDrawWidget3D::customPaintEvent(PaintEngine *e){
      Mutex::Locker lock(m_linkMutex);
      //m_oCommandMutex.lock();
      
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
      //m_oCommandMutex.unlock();
  
      ICLDrawWidget::customPaintEvent(e);
    }

    void ICLDrawWidget3D::link(ICLDrawWidget3D::GLCallback *cb){
      Mutex::Locker lock(m_linkMutex);

      if(cb == m_linkedCallback) return;

      if(m_linkedCallback){
        m_linkedCallback->unlink(this);
      }
      
      m_linkedCallback = cb;
      
      if(m_linkedCallback){
        m_linkedCallback->link(this);
      }
    }
    
  } // namespace qt
}
