// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLPhysics/PhysicsScene.h>
#include <ICLPhysics/PhysicsObject.h>
namespace icl{
  namespace physics{

			PhysicsScene::PhysicsScene(PhysicsWorld::BulletSolverType type)
				: geom::Scene(), PhysicsWorld(type) {}

      void PhysicsScene::addObject(PhysicsObject *object, bool passOwnerShip) {
        Scene::addObject(static_cast<geom::SceneObject*>(object), passOwnerShip);
        PhysicsWorld::addObject(object);
      }

      void PhysicsScene::removeObject(int idx) {
        PhysicsObject* object = dynamic_cast<PhysicsObject*>(Scene::getObject(idx));
        if(!object) {
          PhysicsWorld::addObject(object);
        }
        Scene::removeObject(idx);
      }

      void PhysicsScene::removeObject(PhysicsObject *obj) {
        PhysicsWorld::removeObject(obj);
        Scene::removeObject(static_cast<geom::SceneObject*>(obj));
      }

      void PhysicsScene::removeObjects(int startIndex, int endIndex) {
        if(endIndex < 0) endIndex = m_objects.size();
        ICLASSERT_RETURN(startIndex >= 0 && startIndex < static_cast<int>(m_objects.size()));
        ICLASSERT_RETURN(endIndex >= 0 && endIndex <= static_cast<int>(m_objects.size()));
        ICLASSERT_RETURN(endIndex > startIndex);

        int pos = startIndex;
        while(startIndex++ < endIndex){
          removeObject(pos);
        }
      }
  }
}
