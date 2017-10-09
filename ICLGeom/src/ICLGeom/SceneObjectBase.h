/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SceneObjectBase.h                  **
** Module : ICLGeom                                                **
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

#ifndef ICL_HAVE_OPENGL
  #if WIN32
    #pragma WARNING("this header must not be included if ICL_HAVE_OPENGL is not defined")
  #else
    #warning "this header must not be included if ICL_HAVE_OPENGL is not defined"
  #endif
#else

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Mutex.h>
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
