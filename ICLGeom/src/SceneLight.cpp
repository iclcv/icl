/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/SceneLight.cpp                             **
** Module : ICLGeom                                                **
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

#include <ICLGeom/SceneLight.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Scene.h>

#ifdef HAVE_OPENGL

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#endif

namespace icl{
  
  /// Cam is the Camera that is acutally used for rendering ...
  void SceneLight::setupGL(const Scene &scene, const Camera &cam){
#ifdef HAVE_OPENGL
    GLenum l = GL_LIGHT0 + index;
    if(!on){
      glDisable(l);
      return;
    }else{
      static const GLfloat off[] = {0,0,0,0};
      // note: specular light is not working -> needs to be enabled explicitly
      // since 100% realistic visualization is not our focus, we skip this for now
      glEnable(l);
      glLightfv(l,GL_SPECULAR,specularOn ? specular.begin() : off);
      glLightfv(l,GL_AMBIENT,ambientOn ? ambient.begin() : off);
      glLightfv(l,GL_DIFFUSE,diffuseOn ? diffuse.begin() : off);

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();

      switch(anchor){
        case CamAnchor:
          if(camAnchor < 0){
            glLoadIdentity();
          }else{
            glMultMatrixf(scene.getCamera(camAnchor).getCSTransformationMatrix().inv().transp().data()); 
          }
          break;
        case ObjectAnchor:{
          glMultMatrixf( objectAnchor->getTransformation(false).transp().data());
          break;
        }
        default:
          break;
      }
          
      glLightfv(l,GL_POSITION,position.begin());
      
      glLightfv(l,GL_SPOT_DIRECTION,spotDirection.begin());
      glLightf(l,GL_SPOT_EXPONENT,spotExponent);
      glLightf(l,GL_SPOT_CUTOFF,spotCutoff);
      
      glLightf(l,GL_CONSTANT_ATTENUATION,attenuation[0]);
      glLightf(l,GL_LINEAR_ATTENUATION,attenuation[1]);
      glLightf(l,GL_QUADRATIC_ATTENUATION,attenuation[2]);
      glPopMatrix();
    }
#endif
  }
  SceneLight::SceneLight(int index):index(index){
    reset();
  }
    
  void SceneLight::setOn(bool on){
    this->on = on;
  }
    
  void SceneLight::setAnchorToWorld(){
    this->anchor = WorldAnchor;
  }

  void SceneLight::setAnchor(SceneObject *sceneObject){
    this->anchor = ObjectAnchor;
    this->objectAnchor = sceneObject;
  }
    
  void SceneLight::setCameraAnchor(int cameraIndex){
    this->anchor = CamAnchor;
    this->camAnchor = cameraIndex;
  }
  
  void SceneLight::setPosition(const Vec &position){
    this->position = position;
  }
    
  void SceneLight::setAmbient(const GeomColor &color){
    this->ambient = color/255;
  }

  void SceneLight::setDiffuse(const GeomColor &color){
    this->diffuse = color/255;
  }
  
  void SceneLight::setSpecular(const GeomColor &color){
    this->specular = color/255;
  }
    
  void SceneLight::setAmbientEnabled(bool on){
    this->ambientOn = on;
  }

  void SceneLight::setDiffuseEnabled(bool on){
    this->diffuseOn = on;
  }

  void SceneLight::setSpecularEnabled(bool on){
    this->specularOn = on;
  }
    
  void SceneLight::setSpotDirection(const Vec &vec){
    this->spotDirection = vec;
  }

  void SceneLight::setSpotExponent(float value){
    ICLASSERT_RETURN(value >= 0 && value <= 128);
    this->spotExponent = value;
  }

  void SceneLight::setSpotCutoff(float value){
    ICLASSERT_RETURN((value >= 0 && value <=90) || (value == 180));
    this->spotCutoff = value;
  }

  void SceneLight::setAttenuation(float constant, float linear, float quadratic){
    ICLASSERT_RETURN(constant >= 0);
    ICLASSERT_RETURN(linear >= 0);
    ICLASSERT_RETURN(quadratic >= 0);
    this->attenuation = Vec(constant,linear,quadratic,0);
  }
  
  void SceneLight::reset(){
    on = !index;
    position = Vec(0,0,1,0);
    ambientOn = false;
    diffuseOn = true;
    specularOn = false;

    ambient = GeomColor(0,0,0,0);
    diffuse = GeomColor(1,1,1,1);
    specular = GeomColor(0,0,0,0);
    
    spotDirection = Vec(0,0,-1,1);
    spotExponent = 0;
    spotCutoff = 180;
    attenuation = Vec(1,0,0,0);
    
    anchor = CamAnchor;
    camAnchor = -1;
    objectAnchor = 0;
  }
}
