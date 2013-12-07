/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/GLContext.h                            **
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


#pragma once

#include <ICLUtils/CompatMacros.h>

namespace icl{
  namespace qt{
    
    /// Shallow Wrapper class around GLX and QGLContext instances
    /** The GLContext class can be used to query the current GLContext activated.
        Right now, ICL knows two different kinds of GLContexts:
        -# QGLContext created by all 3D drawing components
        -# GLXContext created for offscreen rendering of scenes
    
        The GLContex class provides a generic access to both types of contexts.
        
        <b>Please note</b> that the GLContext class does right now only work
        in ICL. It is used in the GLFragmentShader class to determine wheter
        a given shader has already been compiled and set up for the current
        active GLContext.
     */
    class ICL_QT_API GLContext{
      public:
      /// internal handle type
      typedef void* Handle;
      
      protected:
      
      /// context handle (either of type QGLContext* or GLXContext)
      Handle handle;
      
      /// defines how to interpret handle
      bool isGLX;
      
      /// for GLXContexts only: corresponding pbuffer ID
      long unsigned int pbuffer;
      
      /// for GLXContexts only: current X-Display
      Handle display;
      
      /// static instance (only necessary for GLXContexts)
      static GLContext current_glx_context;
      
      public:
      /// creates a null context
      GLContext():handle(0),isGLX(0){}
      
      /// create a new context either of type GLXContext or of type QGLContext*
      /** Actually, this constructor should not be called manually */
      GLContext(Handle handle, bool isGLX, long unsigned int pbuffer=0, Handle display=0):
      handle(handle),isGLX(isGLX),pbuffer(pbuffer),display(display){}
      
      /// returns the current context
      static GLContext currentContext();
        
      /// sets ths curretn GLXContext
      /** This is automatically done as long as the scene class instances is rendered into
          an offscreen rendering context */
      static void set_current_glx_context(Handle handle, long unsigned int pbuffer, Handle display);
      
      // unsets the current GLXContext
      /** This is automatically called when Scene::render is finished and not offscreen rendering contexts
          are active any more.*/
      static void unset_current_glx_context();
      
      /// selects the given context (makes it 'current')
      void makeCurrent() const;
      
      
      /// returns whether a context is null
      inline bool isNull() const { return !handle; }
      
      /// returns whether a contest is no null
      inline operator bool() const { return !isNull(); }
      
      
      // lt comparison (for strict ordering)
      inline bool operator<(const GLContext &other) const{
        if(isGLX == other.isGLX){
          return handle < other.handle;
        }else return isGLX;
      }
  
      /// comparison operator
      inline bool operator==(const GLContext &other) const{
        return handle == other.handle && isGLX == other.isGLX;
      }      
      
    };
  } // namespace qt
}

