// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLGeom/Scene.h>
#include <ICLPhysics/PhysicsWorld.h>
namespace icl{
  namespace physics{
    class ICLPhysics_API PhysicsScene: public geom::Scene, public PhysicsWorld {
      public:
			PhysicsScene(PhysicsWorld::BulletSolverType type = PhysicsWorld::Default);
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
