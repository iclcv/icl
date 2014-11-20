/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsScene.h               **
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

#include <ICLGeom/Scene.h>
#include <ICLPhysics/PhysicsWorld.h>
namespace icl{
  namespace physics{
    class ICLPhysics_API PhysicsScene: public geom::Scene, public PhysicsWorld {
      public:
      /// adds a new top-level object to the Scene instance 
      /** By default, the object's memory is managed externally. If you want
          to pass the ownership to the Scene instance, you have to set
          passOwnerShip to true.
          */
      void addObject(PhysicsObject *object, bool passOwnerShip=false);
    
      /// removed object at given index
      /** The object is deleted if it's ownwership was passed */
      void removeObject(int idx);
    
      /// removes given top-level object from scene (not recursive)
      /** The object is deleted if it's ownwership was passed */
      void removeObject(PhysicsObject *obj);

      /// removed object at given indices
      /** The object's are deleted if their ownwership was passed */
      void removeObjects(int startIndex, int endIndex=-1);
    };
  }
}
