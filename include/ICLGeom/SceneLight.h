/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/SceneLight.h                           **
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

#ifndef ICL_SCENE_LIGHT_H
#define ICL_SCENE_LIGHT_H

#include <ICLGeom/GeomDefs.h>

namespace icl{
  
  /** \cond */
  class SceneObject;
  class Scene;
  class Camera;
  /** \endcond */

  /// Wrapper class for OpenGL lights
  /** The light is integrated with the scene and is used to define
      lights in scenes. Each light is associated with one of the
      eight lights provided by OpenGL.*/
  class SceneLight{
    /// called by the scene 
    /** This method is called by the scene and sets up the
        light in OpenGL by using the instances parameters */
    void setupGL(const Scene &scene, const Camera &cam) const;
    
    /// wrapped opengl light index
    const int index;
    
    /// flag that is used to swith off/on the light entirely 
    bool on;
    
    /// the light's position
    Vec position;
    
    /// flag wheather ambient light component is enabled
    bool ambientOn;

    /// flag wheather diffuse light component is enabled
    bool diffuseOn;

    /// flag wheather specular light component is enabled
    bool specularOn;
    
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
    
    /// for the object anchor mode
    SceneObject *objectAnchor;

    /// private constructor -> only Scene's can create lights
    SceneLight(int index);
    
    public:

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
    
    /// sets all paramters to OpenGL's default values
    void reset();
  };
}

#endif
