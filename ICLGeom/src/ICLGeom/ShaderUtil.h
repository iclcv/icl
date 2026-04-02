// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#pragma once
#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/Primitive.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLGeom/SceneLight.h>

#include <ICLUtils/Uncopyable.h>

/** \cond */
namespace icl::qt { class GLFragmentShader; }
/** \endcond */

namespace icl::geom {

    /** The ShaderUtil is an easy to use wrapper for activating the
        correct shader for the primitive that is to be rendered.*/
    class ICLGeom_API ShaderUtil {
      const Camera *m_camera;
      icl::qt::GLFragmentShader** m_shaders;
      icl::qt::GLFragmentShader* activeShader;
      const std::vector<Mat> *project2shadow;
      float m_shadowBias;
      bool renderingShadow;
      public:
      ShaderUtil(const ShaderUtil&) = delete;
      ShaderUtil& operator=(const ShaderUtil&) = delete;

      ///Enum representing the different shader types
      enum ShaderType{SHADOW, SHADOW_TEXTURE, NO_SHADOW, NO_SHADOW_TEXTURE, COUNT};
      ShaderUtil(const Camera *camera, icl::qt::GLFragmentShader** shaders, const std::vector<Mat> *project2shadow, float shadowBias);
      /** This constructor can be used, when the ShaderUtil is not supposed to activate any shaders.
          The main use for this is to make it transparent to the render function of an object if it is to be
          rendered into the shadowbuffer or not.*/
      ShaderUtil();

      void activateShader(Primitive::Type type, bool withShadow);
      void deactivateShaders();
      const Camera &getCurrentCamera() const;
      static void recompilePerPixelShader(icl::qt::GLFragmentShader** shaders, const std::shared_ptr<SceneLight>* lights, int numShadowLights);
    };
} // namespace icl::geom
