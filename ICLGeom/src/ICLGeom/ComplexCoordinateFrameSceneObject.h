/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ComplexCoordinateFrameSceneObject. **
**          h                                                      **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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

#ifndef HAVE_OPENGL
#warning "this header must not be included if HAVE_OPENGL is not defined"
#else

#include <ICLGeom/SceneObject.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  namespace geom{
    
    /// Special SceneObject implementation that define a visible coordinate frame
    /** In constrast to the <em>normal</em> CoordinateFrameSceneObject class, the
        ComplexCoordinateFrameSceneObject is build of cones and cylinders and 
        it uses billboard text as axix label.
        The ComplexCoordinateFrameSceneObject is already integrated with the Scene
        class. Simply set scene.setDrawCoordinateFrameEnabled(true,l,t) to
        visualize a Scene's coordintate frame. If you need a coordinate frame
        that is not alligned with the scene's origin, you can use this class. */
    class ComplexCoordinateFrameSceneObject : public SceneObject{
      /// internally used mutex
      utils::Mutex mutex;
  
      /// length of each axis
      float axisLength;
      
      /// thickness of each axis
      float axisThickness;
      
      public:
      
      /// Default constructor with useful default size
      ICLGeom_API ComplexCoordinateFrameSceneObject(float axisLength=100,float axisThickness=5, 
                                        bool withXYZLabels=true);
  
      /// Dynamic adaption
      ICLGeom_API void setParams(float axisLength, float axisThickness, bool withXYZLabels = true);
      
      /// returns current length of the axis'
      inline float getAxisLength() const { return axisLength; }
  
      /// returns current thickness of the axis'
      inline float getAxisThickness() const { return axisThickness; }
      
      /// locks the internal mutex
      virtual void lock() { mutex.lock(); }
  
      /// unlocks the internal mutex
      virtual void unlock() { mutex.unlock(); }
    };
  } // namespace geom
}

#endif
