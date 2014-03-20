/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ShaderUtil.h                       **
** Module : ICLGeom                                                **
** Authors: Matthias Esau                                          **
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
#include <ICLGeom/Primitive.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLGeom/SceneLight.h>

namespace icl{
    /** \cond */
  namespace qt {class GLFragmentShader;}
    /** \endcond */
  namespace geom{
    
    /** The ShaderUtil is an easy to use wrapper for activating the 
        correct shader for the primitive that is to be rendered.*/
    class ICLGeom_API ShaderUtil {
      icl::qt::GLFragmentShader** m_shaders;
      icl::qt::GLFragmentShader* activeShader;
      const std::vector<Mat> *project2shadow;
      float m_shadowBias;
      bool renderingShadow;
      public:
      ///Enum representing the different shader types
      enum ShaderType{SHADOW, SHADOW_TEXTURE, NO_SHADOW, NO_SHADOW_TEXTURE, COUNT};
      ShaderUtil(icl::qt::GLFragmentShader** shaders, const std::vector<Mat> *project2shadow, float shadowBias);
      /** This constructor can be used, when the ShaderUtil is not supposed to activate any shaders.
          The main use for this is to make it transparent to the render function of an object if it is to be
          rendered into the shadowbuffer or not.*/
      ShaderUtil();
      
      void activateShader(Primitive::Type type, bool withShadow);
      void deactivateShaders();
      static void recompilePerPixelShader(icl::qt::GLFragmentShader** shaders, const icl::utils::SmartPtr<SceneLight>* lights, int numShadowLights);
    };
  }
}
