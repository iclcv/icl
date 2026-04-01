// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#ifndef ICL_DEFAULT_GROUND_OBJECT_H
#define ICL_DEFAULT_GROUND_OBJECT_H

#include <ICLPhysics/RigidBoxObject.h>

namespace icl{

 struct DefaultGroundObject : public RigidBoxObject{
    DefaultGroundObject():RigidBoxObject(0,0,-100,1000,1000,70,0){
      setCollisionMargin(10);
      setFriction(5);
      so->setVisible(false);
    }
  };
  struct DefaultGroundObjectVisualization : public SceneObject{
    DefaultGroundObjectVisualization():SceneObject("cuboid",FixedColVector<float,6>(0,0,-102,1000,1000,70).data()){

      setVisible(Primitive::vertex,false);
      setVisible(Primitive::line,false);
      setColor(Primitive::quad,GeomColor(200,200,200,255));
      //((QuadPrimitive*)getPrimitives()[12])->tesselationResolution = 50;
    }
  };

}


#endif
