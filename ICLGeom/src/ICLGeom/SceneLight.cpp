/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SceneLight.cpp                     **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Matthias Esau                     **
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

#include <ICLGeom/SceneLight.h>
#include <ICLGeom/SceneLightObject.h>
#include <ICLGeom/Scene.h>
#include <ICLQt/GLImg.h>

#ifdef ICL_HAVE_OPENGL

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif ICL_SYSTEM_WINDOWS
#define NOMINMAX
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#endif

namespace icl{
  namespace geom{
    

    SceneLight::SceneLight(const SceneLight &other):index(other.index){
      memcpy(this,&other,sizeof(other));
      objectAnchor = 0;
      lightObject = 0;
      shadowCam = 0;
    }
    
    void SceneLight::updatePositions(const Scene &scene, const Camera &cam) const{
        Mat T = Mat::id();
        try{
          switch(anchor){
            case CamAnchor:
              if(camAnchor < 0){
                glLoadIdentity();
                T = cam.getCSTransformationMatrix().inv();
              }else{        
                T = scene.getCamera(camAnchor).getCSTransformationMatrix().inv();
              }
              break;
            case ObjectAnchor:{
              T = objectAnchor->getTransformation(false);
              break;
            }
            default:
              break;
          }
        }catch(std::runtime_error &e){
          DEBUG_LOG("Error updating scene light position: " << e.what());
        }

        lightObject->lock();
        Vec currPos = lightObject->getTransformation().part<3,0,1,4>();
        Vec targetPos = T*position;
        lightObject->translate(targetPos - currPos);
        shadowCam->setPosition(position);
        lightObject->unlock();
    }

    /// Cam is the Camera that is acutally used for rendering ...
    void SceneLight::setupGL(const Scene &scene, const Camera &cam) const{
  #ifdef ICL_HAVE_OPENGL
      GLenum l = GL_LIGHT0 + index;
      if(!on){
        glDisable(l);
        return;
      }else{
        Mat T = Mat::id();
        
        static const GLfloat off[] = {0,0,0,0};
        // note: specular light is not working -> needs to be enabled explicitly
        // since 100% realistic visualization is not our focus, we skip this for now
        glEnable(l);
        glLightfv(l,GL_SPECULAR,specularOn ? specular.begin() : off);
        glLightfv(l,GL_AMBIENT,ambientOn ? ambient.begin() : off);
        glLightfv(l,GL_DIFFUSE,diffuseOn ? diffuse.begin() : off);
  
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        try{
          switch(anchor){
            case CamAnchor:
              if(camAnchor < 0){
                glLoadIdentity();
                T = cam.getCSTransformationMatrix().inv();
              }else{
                T = scene.getCamera(camAnchor).getCSTransformationMatrix().inv();
                glMultMatrixf(T.transp().data()); 
              }
              break;
            case ObjectAnchor:{
              T = objectAnchor->getTransformation(false);
              glMultMatrixf(T.transp().data());
              break;
            }
            default:
              break;
          }
        }catch(std::runtime_error &e){
          DEBUG_LOG("Error setting up scene light world pose: " << e.what());
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
    SceneLight::SceneLight(Scene *scene, int index):index(index){
      shadowCam = new Camera();
      lightObject = new SceneLightObject(scene,index);
      lightObject->setLockingEnabled(true);
      reset();
    }

    SceneLight::~SceneLight(){
      delete lightObject;
      delete shadowCam;
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
      this->position[3] = 1;
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

    void SceneLight::setShadowEnabled(bool on){
      this->shadowOn = on;
    }

    void SceneLight::setProjectionEnabled(bool on){
      this->projectionOn = on;
    }
      
    void SceneLight::setTwoSidedEnabled(bool on){
      this->twoSidedOn = on;
    }

    bool SceneLight::isOn() const{
      return this->on;
    }

    bool SceneLight::getShadowEnabled() const{
      return this->shadowOn;
    }

    bool SceneLight::getProjectionEnabled() const{
      return this->projectionOn;
    }
      
    bool SceneLight::getTwoSidedEnabled() const{
      return this->twoSidedOn;
    }
    
    const Camera* SceneLight::getShadowCam() const{
      return this->shadowCam;
    }
    
    Camera* SceneLight::getShadowCam(){
      return this->shadowCam;
    }
    
    void SceneLight::setShadowCam(Camera* cam){
      delete this->shadowCam;
      this->shadowCam = cam;
    }

    void SceneLight::setProjectionImage(qt::GLImg* img) {
      projectionImage = img;
    }


    qt::GLImg* SceneLight::getProjectionImage() const{
      return projectionImage;
    }
    
    void SceneLight::reset(){
      on = !index;
      position = Vec(0,0,2,1);
      ambientOn = false;
      diffuseOn = true;
      specularOn = false;
      shadowOn = false;
      projectionOn = false;
      twoSidedOn = false;
  
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
    
    void SceneLight::setObjectSize(float size){
      lightObject->lock();
      // todo: perhaps, we can extract the light's current scale using QR-decomposition?
      lightObject->removeTransformation();
      lightObject->scale(size,size,size);
      lightObject->unlock();
    }
    
  } // namespace geom
}
