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
#include <sstream>

namespace icl{
  namespace geom{
    ShaderUtil::ShaderUtil():
      m_camera(0),
      m_shaders(0),
      activeShader(0),
      project2shadow(0),
      m_shadowBias(0),
      renderingShadow(true){}
  
    ShaderUtil::ShaderUtil(const Camera *camera, qt::GLFragmentShader** shaders, const std::vector<Mat> *project2shadow,float shadowBias):
      m_camera(camera),
      m_shaders(shaders),
      activeShader(0),
      project2shadow(project2shadow),
      m_shadowBias(shadowBias),
      renderingShadow(false){}
  
    const Camera &ShaderUtil::getCurrentCamera() const throw (utils::ICLException){
      if(!m_camera) throw utils::ICLException("shader util has no camera!");
      return *m_camera;
    }

    void ShaderUtil::activateShader(Primitive::Type type, bool withShadow) {
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
        activeShader->setUniform("projection_map0", 8);
        activeShader->setUniform("projection_map1", 9);
        activeShader->setUniform("projection_map2", 10);
        activeShader->setUniform("projection_map3", 11);
        activeShader->setUniform("projection_map4", 12);
        activeShader->setUniform("projection_map5", 13);
        activeShader->setUniform("projection_map6", 14);
        activeShader->setUniform("projection_map7", 15);
      }
    }

    void ShaderUtil::deactivateShaders() {
      if(activeShader) {
        activeShader->deactivate();
        activeShader = 0;
      }
    }


    void ShaderUtil::recompilePerPixelShader(qt::GLFragmentShader** shaders, const utils::SmartPtr<SceneLight>* lights, int numShadowLights) {
      for(unsigned int i = 0; i < ShaderUtil::COUNT; i++) {
        delete shaders[i];
      }
      std::stringstream fragmentBuffer;
      std::stringstream vertexBuffer;

      //creating the vertex shader
      vertexBuffer
      <<"varying vec4 V;\n"
      <<"varying vec3 vertex_normal;\n";
      if(numShadowLights>0) {
        vertexBuffer
        <<"varying vec4 shadow_coord["<<numShadowLights<<"];\n"
        <<"uniform mat4 shadowMat["<<numShadowLights<<"];\n";
      }
      vertexBuffer
      <<"void main()\n"
      <<"{\n"
      <<"  V = gl_ModelViewMatrix * gl_Vertex;\n"
      <<"  vertex_normal = gl_NormalMatrix * gl_Normal;\n"
      <<"  gl_FrontColor = gl_Color;\n"
      <<"  gl_TexCoord[0] = gl_MultiTexCoord0;\n"
      <<"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n";

      //creating the fragment shader
      fragmentBuffer
      <<"varying vec4 V;\n"
      <<"varying vec3 vertex_normal;\n";
      if(numShadowLights>0) {
        fragmentBuffer
        <<"varying vec4 shadow_coord["<<numShadowLights<<"];\n"
        <<"const int num_shadow_lights = " <<numShadowLights<<";\n";
      }
      fragmentBuffer
      <<"vec3 N;\n"
      <<"vec4 texture_Color;\n"
      <<"uniform sampler2D shadow_map;\n"
      <<"uniform sampler2D image_map;\n"
      <<"uniform sampler2D projection_map0;\n"
      <<"uniform sampler2D projection_map1;\n"
      <<"uniform sampler2D projection_map2;\n"
      <<"uniform sampler2D projection_map3;\n"
      <<"uniform sampler2D projection_map4;\n"
      <<"uniform sampler2D projection_map5;\n"
      <<"uniform sampler2D projection_map6;\n"
      <<"uniform sampler2D projection_map7;\n"
      <<"uniform float bias;\n"
      <<"void computeColors(int light, out vec3 ambient, out vec3 diffuse, out vec3 specular, out float cos_light){\n"
      <<"  vec3 L = normalize(gl_LightSource[light].position.xyz - V.xyz);\n"
      <<"  vec3 E = normalize(-V.xyz);\n"
      <<"  vec3 R = normalize(-reflect(L, N));\n"
      <<"  cos_light = max(dot(N,L),0.0);\n"
      <<"#ifdef USE_TEXTURE\n"
      <<"  vec3 color = gl_Color.rgb * texture_Color.rgb;\n"
      <<"#else\n"
      <<"  vec3 color = gl_Color.rgb;\n"
      <<"#endif\n"
      <<"  ambient = gl_LightSource[light].ambient.rgb\n"
      <<"            * color;\n"
      <<"  diffuse = gl_LightSource[light].diffuse.rgb\n"
      <<"            * cos_light\n"
      <<"            * color;\n"
      <<"  specular = gl_LightSource[light].specular.rgb\n"
      <<"             * pow(max(0.0,dot(R,E)),gl_FrontMaterial.shininess)\n"
      <<"             * gl_FrontMaterial.specular.rgb;\n"
      <<"}\n"
      <<"void computeColorsLightColor(int light, vec3 light_color, out vec3 diffuse, out vec3 specular, out float cos_light){\n"
      <<"  vec3 L = normalize(gl_LightSource[light].position.xyz - V.xyz);\n"
      <<"  vec3 E = normalize(-V.xyz);\n"
      <<"  vec3 R = normalize(-reflect(L, N));\n"
      <<"  cos_light = max(dot(N,L),0.0);\n"
      <<"#ifdef USE_TEXTURE\n"
      <<"  vec3 color = gl_Color.rgb * texture_Color.rgb;\n"
      <<"#else\n"
      <<"  vec3 color = gl_Color.rgb;\n"
      <<"#endif\n"
      <<"  diffuse = light_color\n"
      <<"            * cos_light\n"
      <<"            * color;\n"
      <<"  specular = light_color\n"
      <<"             * pow(max(0.0,dot(R,E)),gl_FrontMaterial.shininess)\n"
      <<"             * gl_FrontMaterial.specular.rgb;\n"
      <<"}\n";

      fragmentBuffer
      <<"void computeColorsTwoSided(int light, out vec3 ambient, out vec3 diffuse, out vec3 specular, float cos_light){\n"
      <<"  vec3 L = normalize(gl_LightSource[light].position.xyz - V.xyz);\n"
      <<"  vec3 E = normalize(-V.xyz);\n"
      <<"  vec3 R = normalize(-reflect(L, N));\n"
      <<"  cos_light = abs(dot(N,L));\n"
      <<"#ifdef USE_TEXTURE\n"
      <<"  vec3 color = gl_Color.rgb * texture_Color.rgb;\n"
      <<"#else\n"
      <<"  vec3 color = gl_Color.rgb;\n"
      <<"#endif\n"
      <<"  ambient = gl_LightSource[light].ambient.rgb\n"
      <<"            * color;\n"
      <<"  diffuse = gl_LightSource[light].diffuse.rgb\n"
      <<"            * cos_light\n"
      <<"            * color;\n"
      <<"  specular = gl_LightSource[light].specular.rgb\n"
      <<"             * pow(abs(dot(R,E)),gl_FrontMaterial.shininess)\n"
      <<"             * gl_FrontMaterial.specular.rgb;\n"
      <<"}\n"
      <<"void computeColorsTwoSidedLightColor(int light, vec3 light_color, out vec3 diffuse, out vec3 specular, float cos_light){\n"
      <<"  vec3 L = normalize(gl_LightSource[light].position.xyz - V.xyz);\n"
      <<"  vec3 E = normalize(-V.xyz);\n"
      <<"  vec3 R = normalize(-reflect(L, N));\n"
      <<"  cos_light = abs(dot(N,L));\n"
      <<"#ifdef USE_TEXTURE\n"
      <<"  vec3 color = gl_Color.rgb * texture_Color.rgb;\n"
      <<"#else\n"
      <<"  vec3 color = gl_Color.rgb;\n"
      <<"#endif\n"
      <<"  diffuse = light_color\n"
      <<"            * cos_light\n"
      <<"            * color;\n"
      <<"  specular = light_color\n"
      <<"             * pow(abs(dot(R,E)),gl_FrontMaterial.shininess)\n"
      <<"             * gl_FrontMaterial.specular.rgb;\n"
      <<"}\n";

      if(numShadowLights>0) {
        fragmentBuffer
        <<"vec3 computeLightWithShadow(int light, int shadow, bool isTwoSided){\n"
        <<"  vec3 ambient, diffuse, specular;\n"
        <<"  float cos_light = 0.0;\n"
        <<"  //compute phong lighting\n"
        <<"  if(isTwoSided)computeColorsTwoSided(light, ambient, diffuse, specular, cos_light);\n"
        <<"  else computeColors(light, ambient, diffuse, specular, cos_light);\n"
        <<"  //get screen space coordinates\n"
        <<"  vec4 shadow_divided = shadow_coord[shadow] / shadow_coord[shadow].w;\n"
        <<"  //check if the coordinate is out of bounds\n"
        <<"  if(shadow_divided.s < -1.0 || shadow_divided.s > 1.0) return ambient;\n"
        <<"  if(shadow_divided.t < -1.0 || shadow_divided.t > 1.0) return ambient;\n"
        <<"  //transform to texture space coordinates\n"
        <<"  shadow_divided = shadow_divided * 0.5 + 0.5;\n"
        <<"  shadow_divided.s = (float(shadow) + shadow_divided.s) / float(num_shadow_lights);\n"
        <<"  //get shadow depth + offset\n"
        <<"  float d = length(gl_LightSource[light].position.xyz - V.xyz);\n"
        <<"  //normalize bias over distance and try to remove artifacts very acute angles\n"
        <<"  float normalized_bias = bias * 0.03 / ((d * d - 2.0 * d) * max(cos_light,0.1));\n"
        <<"  float shadow_depth = texture2D(shadow_map,shadow_divided.st).z + normalized_bias;\n"
        <<"  //check if fragment is in shadow\n"
        <<"  if(shadow_coord[shadow].w > 0.0)\n"
        <<"    if(shadow_divided.z > shadow_depth) return ambient;\n"
        <<"  return ambient + diffuse + specular;\n"
        <<"}\n"
        <<"vec3 computeLightWithProjection(int light, int shadow, sampler2D projection_map, bool isTwoSided){\n"
        <<"  vec3 ambient, diffuse, specular;\n"
        <<"  float cos_light = 0.0;\n"
        <<"  //get screen space coordinates\n"
        <<"  vec4 shadow_divided = shadow_coord[shadow] / shadow_coord[shadow].w;\n"
        <<"  //compute ambient color\n"
        <<"#ifdef USE_TEXTURE\n"
        <<"  vec3 color = gl_Color.rgb * texture_Color.rgb;\n"
        <<"#else\n"
        <<"  vec3 color = gl_Color.rgb;\n"
        <<"#endif\n"
        <<"  ambient = gl_LightSource[light].ambient.rgb\n"
        <<"            * color;\n"
        <<"  //check if the coordinate is out of bounds\n"
        <<"  if(shadow_divided.s < -1.0 || shadow_divided.s > 1.0) return ambient;\n"
        <<"  if(shadow_divided.t < -1.0 || shadow_divided.t > 1.0) return ambient;\n"
        <<"  //transform to texture space coordinates\n"
        <<"  shadow_divided = shadow_divided * 0.5 + 0.5;\n"
        <<"  //get color of projection before converting to shadowtexture space\n"
        <<"  vec3 proj = texture2D(projection_map,shadow_divided.st).rgb;\n"
        <<"  //converting to shadowtexture space\n"
        <<"  shadow_divided.s = (float(shadow) + shadow_divided.s) / float(num_shadow_lights);\n"
        <<"  //get shadow depth + offset\n"
        <<"  float d = length(gl_LightSource[light].position.xyz - V.xyz);\n"
        <<"  //normalize bias over distance and try to remove artifacts very acute angles\n"
        <<"  float normalized_bias = bias * 0.03 / ((d * d - 2.0 * d) * max(cos_light,0.1));\n"
        <<"  float shadow_depth = texture2D(shadow_map,shadow_divided.st).z + normalized_bias;\n"
        <<"  //compute phong lighting\n"
        <<"  if(isTwoSided)computeColorsTwoSidedLightColor(light, proj, diffuse, specular, cos_light);\n"
        <<"  else computeColorsLightColor(light, proj, diffuse, specular, cos_light);\n"
        <<"  //check if fragment is in shadow\n"
        <<"  if(shadow_coord[shadow].w > 0.0)\n"
        <<"    if(shadow_divided.z > shadow_depth) return ambient;\n"
        <<"  return ambient + diffuse + specular;\n"
        <<"}\n";
      }

      // celbrech: note, for shadow lights, one-sided lighting is used
      //           for other lights, GL_LIGHT_MODEL_TWO_SIDE is emulated

      fragmentBuffer
      <<"vec3 computeLight(int light, bool isTwoSided){\n"
      <<"  vec3 ambient, diffuse, specular;\n"
      <<"  float cos_light = 0.0;\n"
      <<"  //compute phong lighting\n"
      <<"  if(isTwoSided)computeColorsTwoSided(light, ambient, diffuse, specular, cos_light);\n"
      <<"  else computeColors(light, ambient, diffuse, specular, cos_light);\n"
      <<"  return ambient + diffuse + specular;\n"
      <<"}\n"
      <<"void main(void){\n"
      <<"  N = normalize(vertex_normal);\n"
      <<"  texture_Color = texture2D(image_map, gl_TexCoord[0].st);\n"
      <<"  vec3 color = vec3(0,0,0);\n";

      int currentShadow = 0;
      for(unsigned int i = 0; i < 8; i++) {
        if(lights[i] && lights[i]->isOn()) {
          std::string twoSided;
          if(lights[i]->getTwoSidedEnabled()) {
            twoSided = "true";
          }else {
            twoSided = "false";
          }
          if(lights[i]->getShadowEnabled()) {
            vertexBuffer
            <<"  shadow_coord["<<currentShadow<<"] = shadowMat["<<currentShadow<<"] * V;\n";
            fragmentBuffer
            <<"#ifdef RENDER_SHADOW\n";
            if(lights[i]->getProjectionEnabled()) {
              fragmentBuffer
              <<"  color += computeLightWithProjection("<<i<<","<<currentShadow<<","<<"projection_map"<<i<<","<<twoSided<<");\n";
            }else{
              fragmentBuffer
              <<"  color += computeLightWithShadow("<<i<<","<<currentShadow<<","<<twoSided<<");\n";
            }
            fragmentBuffer
            <<"#else\n"
            <<"  color += computeLight("<<i<<");\n"
            <<"#endif\n";
            currentShadow++;
          }else{
            fragmentBuffer << "  color += computeLight("<<i<<","<<twoSided<<");\n";
          }

        }
      }
      vertexBuffer
      <<"}\n";
      fragmentBuffer
      <<"#ifdef USE_TEXTURE\n"
      <<"  gl_FragColor =  vec4(color,gl_Color.a * texture_Color.a);\n"
      <<"#else\n"
      <<"  gl_FragColor =  vec4(color,gl_Color.a);\n"
      <<"#endif\n"
      <<"}\n";

      shaders[ShaderUtil::SHADOW] = new qt::GLFragmentShader( vertexBuffer.str(), "#define RENDER_SHADOW\n" + fragmentBuffer.str());
      shaders[ShaderUtil::SHADOW_TEXTURE] = new qt::GLFragmentShader( vertexBuffer.str(), "#define USE_TEXTURE;\n#define RENDER_SHADOW\n" + fragmentBuffer.str());
      shaders[ShaderUtil::NO_SHADOW] = new qt::GLFragmentShader( vertexBuffer.str(), fragmentBuffer.str());
      shaders[ShaderUtil::NO_SHADOW_TEXTURE] = new qt::GLFragmentShader( vertexBuffer.str(), "#define USE_TEXTURE;\n" + fragmentBuffer.str());
    }


  }


}
