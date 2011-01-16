/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/CoordinateFrameSceneObject.h           **
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

#ifndef ICL_COORDINATE_FRAME_SCENE_OBJECT_H
#define ICL_COORDINATE_FRAME_SCENE_OBJECT_H

#include <ICLGeom/SceneObject.h>

namespace icl{
  
  /// Special SceneObject implementation that define a visible coordinate frame
  /** The CoordinateFrameSceneObject is already integrated with the Scene
      class. Simply set scene.setDrawCoordinateFrameEnabled(true,l,t) to
      visualize a Scene's coordintate frame. If you need a coordinate frame
      that is not alligned with the scene's origin, you can use this class. */
  class CoordinateFrameSceneObject : public SceneObject{
    
    /// length for x-, y- and z-axis
    float axisLength;
    
    /// thickness of the axis'
    float axisThickness;
    
    /// only internally used for later adaption of parameters
    SceneObject *axis[3];

    public:
    
    /// Default constructor with useful default size
    CoordinateFrameSceneObject(float axisLength=100,float axisThickness=5):
    axisLength(axisLength),axisThickness(axisThickness){
      for(int i=0;i<3;++i){
        axis[i] = addCube(0,0,0,1);
        axis[i]->setColor(Primitive::quad,GeomColor((i==0)*255,(i==1)*255,(i==2)*255,255));
      }
      setVisible(Primitive::vertex,false,true);
      setVisible(Primitive::line,false,true);

      setParams(axisLength,axisThickness);
    }
    /// Dynamic adaption
    void setParams(float axisLength, float axisThickness){
      this->axisLength = axisLength;
      this->axisThickness = axisThickness;
      const float &l = axisLength, t=axisThickness;
      
      for(int i=0;i<3;++i){
        axis[i]->removeTransformation();
        axis[i]->scale((i==0)*l+(i!=0)*t,(i==1)*l+(i!=1)*t,(i==2)*l+(i!=2)*t);
        axis[i]->translate((i==0)*l/2,(i==1)*l/2,(i==2)*l/2);
      }
    }
    
    /// returns current length of the axis'
    float getAxisLength() const { return axisLength; }

    /// returns current thickness of the axis'
    float getAxisThickness() const { return axisThickness; }
  };
}


#endif

