/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SceneLight.h                       **
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

#pragma once

#ifndef ICL_HAVE_OPENGL
  #ifdef WIN32
    #pragma WARNING("this header must not be included if ICL_HAVE_OPENGL is not defined")
  #else
    #warning "this header must not be included if ICL_HAVE_OPENGL is not defined"
  #endif
#else

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  namespace geom{
    
    /** \cond */
    class SceneObject;
    class Scene;
    class Camera;
    class SceneLightObject;
    /** \endcond */
  
    /// Wrapper class for OpenGL lights
    /** The light is integrated with the scene and is used to define
        lights in scenes. Each light is associated with one of the
        eight lights provided by OpenGL.
        
        \section SHADOWS Shadows
        The class allows casting shadows, which can be enabled by
        using setShadowEnabled(). To change the direction of the 
        shadow use getShadowCam() to get the shadow camera. Do not
        change the position of the camera since it will be back
        changed to the lights position every frame.
        */
    class ICLGeom_API SceneLight : public utils::Uncopyable{
      /// called by the scene 
      /** This method is called by the scene and sets up the
          light in OpenGL by using the instances parameters */
      void setupGL(const Scene &scene, const Camera &cam) const;
      
      void updatePositions(const Scene &scene, const Camera &cam) const;
      
      /// wrapped opengl light index
      const int index;
      
      /// flag that is used to swith off/on the light entirely 
      bool on;
      
      /// the light's position
      Vec position;
      
      /// flag whether ambient light component is enabled
      bool ambientOn;
  
      /// flag whether diffuse light component is enabled
      bool diffuseOn;
  
      /// flag whether specular light component is enabled
      bool specularOn;
      
      //flag weather the light casts a shadow
      bool shadowOn;
      
      //flag weather the light casts a shadow
      bool twoSidedOn;
      
      /// ambient light color
      GeomColor ambient;
      
      /// diffuse light color
      GeomColor diffuse;
      
      /// specular light color
      GeomColor specular;
      
      /// direction vector for directed light (0,0,-1) at default
      Vec spotDirection;
      
      /// defines the intensity distribution of the light
      /** possible values are 0-128 */
      float spotExponent;
      
      /// defines the cutoff for spot-lights
      /** possible values are 0-90 and one special value 180 which is 
          default and defines a uniform light distribution */
      float spotCutoff;
      
      /// special factor for light attenuation
      /** only positive values are allowed */
      Vec attenuation;
  
      /// internally used anchor mode
      enum Anchor{
        WorldAnchor, //!< light's position is relative to the world
        CamAnchor,   //!< light's position is relative to a camera
        ObjectAnchor //!< light's position is relative to an object
      } anchor; //!< current anchor mode
      
      /// for camera anchor-mode
      int camAnchor;
      
      ///Shadow Camera for rendering ShadowMaps
      Camera *shadowCam;
      
      /// for the object anchor mode
      SceneObject *objectAnchor;
  
      /// associated scene light Object
      SceneLightObject *lightObject;
      
      /// private constructor -> only Scene's can create lights
      SceneLight(Scene *parent, int index);
      
      /// copies everything (overwrites uncopyable)
      /** Only accessible by friends, object anchor must be tackled manually */
      SceneLight(const SceneLight &other);
     

      public:
  
      /// Destructor
      ~SceneLight();
      
      /// returns the associated light object
      inline SceneLightObject *getLightObject(){ return lightObject; }

      /// returns the associated light object (const)
      inline const SceneLightObject *getLightObject() const { return lightObject; }

      /// sets the light objects scaling (default is 1)
      void setObjectSize(float size);
      
      /// for tight integration with the Scene class
      friend class Scene;
  
      /// globally switches the light on or off
      /** At default: only light 0 is on */
      void setOn(bool on=true);
      
      /// sets that this camera's position is given relatively to the world
      void setAnchorToWorld();
  
      /// sets an object, the light is liked to
      /** In this case, the light's position and spot-
          direction is given relatively to the given
          object.*/
      void setAnchor(SceneObject *sceneObject);
      
      /// sets that this light's position is given relatively to a camera in the scene
      /** If the given cameraIndex is < 0, then the light is positioned relatively
          to the current rendering camera. This is default. */
      void setCameraAnchor(int cameraIndex=-1);
  
      /// sets the light's position
      /** How the position is interpreted depends on the current
          internal anchor mode. Please note, that the internal anchor
          mode cannot be set explicitly using a setAnchorMode method.
          Instead, the AnchorMode is always implicitly adapted when
          one of the methods
          - setAnchorToWorld
          - setAnchor(SceneObject*)
          - setCameraAnchor(int)
      */
      void setPosition(const Vec &position);
      
      /// sets the light's ambient color
      void setAmbient(const GeomColor &color);
  
      /// sets the light's diffuse color
      void setDiffuse(const GeomColor &color);
      
      /// sets the light's specular color
      void setSpecular(const GeomColor &color);
      
      /// sets whether the light's ambient component is enabled
      void setAmbientEnabled(bool on=true);
  
      /// sets whether the light's diffuse component is enabled
      void setDiffuseEnabled(bool on=true);
  
      /// sets whether the light's specular component is enabled
      void setSpecularEnabled(bool on=true);
      
      /// sets the spot direction of this light
      void setSpotDirection(const Vec &vec=Vec(0,0,-1,1));
  
      /// sets the spot exponent of this light
      void setSpotExponent(float value=0);
  
      /// sets the spot cutoff of this light
      void setSpotCutoff(float value=180);
  
      /// sets the attenuation factors of this light
      void setAttenuation(float constant=1, float linear=0, float quadratic=0);
      
      /// sets whether the light casts shadows or not
      void setShadowEnabled(bool on=true);
      
      /// sets whether the light casts two-sided light or not
      void setTwoSidedEnabled(bool on=true);

      /// returns whether the light is activated
      bool isOn() const;

      /// returns whether the light casts shadows or not
      bool getShadowEnabled() const;
      
      /// returns whether the light casts two-sided light or not
      bool getTwoSidedEnabled() const;
      
      /// returns the camera used for casting the shadows
      const Camera* getShadowCam() const;
      
      /// returns the camera used for casting the shadows
      Camera* getShadowCam();
      
      /// changes the shadowcam to the provided camera
      void setShadowCam(Camera* cam);
      
      /// sets all paramters to OpenGL's default values
      void reset();
    };
  } // namespace geom
}

#endif
