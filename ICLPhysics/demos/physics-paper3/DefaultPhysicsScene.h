/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/demos/physics-paper3/DefaultPhysicsScene.h  **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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

#include <ICLPhysics/PhysicsScene.h>

namespace icl{

  /// Utility scene class
  /** The scene will automatically add the given camera (from file) and it
      will set up the 2nd scene Light as a centered spot */
  struct DefaultPhysicsScene : public PhyiscsScene{
    DefaultPhysicsScene(const std::string &firstCameraName=""){
      if(firstCameraName != ""){
        addCamera(Camera(firstCameraName));
      }else{
        addCamera(Camera(Vec(205.614,-324.024,68.2348,1),
                         Vec(-0.506236,0.767777,-0.392739,1),
                         Vec(0.228754,-0.409613,-0.883113,1),
                         3, Point32f(320,240),200,200,
                         0, Camera::RenderParams(Size(640,480),
                                                 1,100000,
                                                 Rect(0,0,640,480),
                                                 0,1)));
      }
      getCamera(0).getRenderParams().clipZFar = 100000;

#if 0
      SceneLight &l = getLight(1);
      l.setAnchorToWorld();
      l.setPosition(Vec(0,0,1500,1));
      l.setShadowEnabled();
      l.setSpecular(GeomColor(255,200,50,255));
      l.setSpecularEnabled();
      l.getShadowCam()->setNorm(Vec(0,0,-1,1));
      l.setOn(true);

      SceneLight &l0 = getLight(0);
      l0.setAmbientEnabled(true);
      l0.setAmbient(GeomColor(255,255,255,5));
#endif

    }
  };

}
