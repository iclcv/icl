/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/demos/physics-paper/DefaultGroundObject.h   **
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
