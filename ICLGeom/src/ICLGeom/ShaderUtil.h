#pragma once
#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/Primitive.h>
#include <ICLGeom/GeomDefs.h>

namespace icl{
    /** \cond */
  namespace qt {class GLFragmentShader;}
    /** \endcond */
  namespace geom{
    
    /** The ShaderUtil is an easy to use wrapper for activating the 
        correct shader for the primitive that is to be rendered.*/
    class ShaderUtil {
      icl::qt::GLFragmentShader** m_shaders;
      icl::qt::GLFragmentShader* activeShader;
      const std::vector<Mat> *project2shadow;
      float m_shadowBias;
      bool renderingShadow;
      public:
      ///Enum representing the different shader types
      enum ShaderType{SHADOW, SHADOW_TEXTURE, NO_SHADOW, NO_SHADOW_TEXTURE, COUNT};
      ICLGeom_API ShaderUtil(icl::qt::GLFragmentShader** shaders, const std::vector<Mat> *project2shadow, float shadowBias);
      /** This constructor can be used, when the ShaderUtil is not supposed to activate any shaders.
          The main use for this is to make it transparent to the render function of an object if it is to be
          rendered into the shadowbuffer or not.*/
      ICLGeom_API ShaderUtil();
      
      ICLGeom_API void activateShader(Primitive::Type type, bool withShadow);
      ICLGeom_API void deactivateShaders();
    };
  }
}
