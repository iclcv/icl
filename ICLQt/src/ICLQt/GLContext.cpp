/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/GLContext.cpp                          **
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

#include <ICLQt/GLContext.h>

#include <QtOpenGL/QGLContext>

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glx.h>
#elif ICL_SYSTEM_WINDOWS
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

namespace icl{
  namespace qt{
    
    GLContext GLContext::current_glx_context(0,false,0,0);
    
    void GLContext::set_current_glx_context(Handle handle, long unsigned int pbuffer, Handle display){
      current_glx_context = GLContext(handle,true,pbuffer,display);
    }
  
    void GLContext::unset_current_glx_context(){
      current_glx_context = GLContext(0,false,0,0);
    }
  
    GLContext GLContext::currentContext(){
      const QGLContext *ctx = QGLContext::currentContext();
      if(!ctx) return current_glx_context;
      return GLContext((void*)ctx,false);
    }
    
    void GLContext::makeCurrent() const{
      if(isNull()) return;
      ((QGLContext*)handle)->makeCurrent();
    }
  
    
  
  } // namespace qt
}
