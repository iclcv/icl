/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsScene.cpp             **
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
#include <ICLPhysics/PhysicsScene.h>
#include <ICLPhysics/PhysicsObject.h>
namespace icl{
  namespace physics{

			PhysicsScene::PhysicsScene(PhysicsWorld::BulletSolverType type)
				: geom::Scene(), PhysicsWorld(type) {}

      void PhysicsScene::addObject(PhysicsObject *object, bool passOwnerShip) {
        Scene::addObject((geom::SceneObject*)object, passOwnerShip);
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
        Scene::removeObject((geom::SceneObject*)obj);
      }

      void PhysicsScene::removeObjects(int startIndex, int endIndex) {
        if(endIndex < 0) endIndex = m_objects.size();
        ICLASSERT_RETURN(startIndex >= 0 && startIndex < (int)m_objects.size());
        ICLASSERT_RETURN(endIndex >= 0 && endIndex <= (int)m_objects.size());
        ICLASSERT_RETURN(endIndex > startIndex);

        int pos = startIndex;
        while(startIndex++ < endIndex){
          removeObject(pos);
        }
      }
  }
}
