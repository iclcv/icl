/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ShaderUtil.cpp                     **
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
