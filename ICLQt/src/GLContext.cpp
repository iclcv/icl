/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/GLContext.h                                  **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQt/GLContext.h>

#include <QtOpenGL/QGLContext>

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glx.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

namespace icl{
  
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
    if(isGLX) glXMakeCurrent((Display*)display,(GLXPbuffer)pbuffer, (GLXContext) handle);
    else ((QGLContext*)handle)->makeCurrent();
  }

  

}
