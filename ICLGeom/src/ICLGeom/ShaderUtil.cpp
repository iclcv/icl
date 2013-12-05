#include <ICLGeom/ShaderUtil.h>
#include <ICLQt/GLFragmentShader.h>

icl::geom::ShaderUtil::ShaderUtil():
m_shaders(0),
activeShader(0),
project2shadow(0),
m_shadowBias(0),
renderingShadow(true)
{}

icl::geom::ShaderUtil::ShaderUtil(icl::qt::GLFragmentShader** shaders, const std::vector<Mat> *project2shadow,float shadowBias):
m_shaders(shaders),
activeShader(0),
project2shadow(project2shadow),
m_shadowBias(shadowBias),
renderingShadow(false)
{}

void icl::geom::ShaderUtil::activateShader(Primitive::Type type, bool withShadow) {
  if(renderingShadow)return;
  if(activeShader)activeShader->deactivate();
  switch(type) {
  case Primitive::text:
  case Primitive::texture:
    if(withShadow) {
      activeShader = m_shaders[SHADOW_TEXTURE];
    } else {
      activeShader = m_shaders[NO_SHADOW_TEXTURE];
    }
    break;
  case Primitive::vertex:
  case Primitive::line:
    activeShader = 0;
    break;
  default:
    if(withShadow) {
      activeShader = m_shaders[SHADOW];
    } else {
      activeShader = m_shaders[NO_SHADOW];
    }
  }
  if(activeShader) {
    activeShader->activate();
    activeShader->setUniform("bias", m_shadowBias);
    activeShader->setUniform("shadowMat", *project2shadow);
    activeShader->setUniform("shadow_map", 7);
    activeShader->setUniform("image_map", 0);
  }
}

void icl::geom::ShaderUtil::deactivateShaders() {
  if(activeShader) {
    activeShader->deactivate();
    activeShader = 0;
  }
}
