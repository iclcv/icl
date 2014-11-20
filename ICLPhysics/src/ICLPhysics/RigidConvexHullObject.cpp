/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/RigidConvexHullObject.cpp    **
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
#include <ICLPhysics/RigidConvexHullObject.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <ICLPhysics/MotionState.h>

namespace icl{
  namespace physics{
    RigidConvexHullObject::RigidConvexHullObject(float x, float y, float z, 
                                                               const std::vector<int> &indices, const std::vector<geom::Vec> &vertices,
                                                               geom::Vec offset,
                                                               float mass){
      btConvexHullShape *shape = new btConvexHullShape();
      for(unsigned int i = 0; i < indices.size(); i++){
        geom::Vec point = vertices[indices[i]] - offset;
        addVertex(point);
        shape->addPoint(btVector3(icl2bullet(point[0]),icl2bullet(point[1]),icl2bullet(point[2])));
      }
      btTransform T;
      T.setIdentity();
      T.setOrigin(btVector3(icl2bullet(x),icl2bullet(y),icl2bullet(z)));
      MotionState* motion = new MotionState(T, (RigidObject*)this);
      btVector3 inertia(0,0,0);
      shape->calculateLocalInertia(mass, inertia);
      btRigidBody::btRigidBodyConstructionInfo ci(mass,motion,shape,inertia);
      ci.m_linearSleepingThreshold *= METER_TO_BULLET_UNIT;
      ci.m_angularSleepingThreshold *= METER_TO_BULLET_UNIT;
      setPhysicalObject(new btRigidBody(ci));
    }
    
    RigidConvexHullObject::RigidConvexHullObject(float x, float y, float z, 
                                                               const std::vector<geom::Vec> &vertices,
                                                               geom::Vec offset,
                                                               float mass){
      std::vector<int> indices;
      indices.reserve(vertices.size());
      for(unsigned int i = 0; i < vertices.size(); i++){
        indices.push_back(i);
      }
      RigidConvexHullObject(x, y, z, indices, vertices, offset, mass);
    }
  }
}
