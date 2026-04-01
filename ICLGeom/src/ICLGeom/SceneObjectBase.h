// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#ifndef ICL_HAVE_OPENGL
  #if WIN32
    #pragma WARNING("this header must not be included if ICL_HAVE_OPENGL is not defined")
  #else
    #warning "this header must not be included if ICL_HAVE_OPENGL is not defined"
  #endif
#else

#include <ICLUtils/CompatMacros.h>
#include <mutex>
#include <ICLQt/GLFragmentShader.h>

namespace icl{
  namespace geom{

    /** \cond */
    class Scene;
    /** \endcond */


    /// The SceneObjectBase class defines and abstract interface for visible entities in 3D scenes
    /** TODO */
    class SceneObjectBase{
      public:
      virtual SceneObjectBase *copy() const = 0;

      virtual void customRender() = 0;
    };


  } // namespace geom
}

#endif
