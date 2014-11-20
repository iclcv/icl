/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/RigidObject.cpp              **
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
#include <ICLPhysics/RigidObject.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <ICLPhysics/PhysicsDefs.h>

namespace icl{
  namespace physics{
    RigidObject::RigidObject(){}  
    
    btRigidBody *RigidObject::getRigidBody() { 
      return dynamic_cast<btRigidBody*>(getCollisionObject()); 
    }

    void RigidObject::setLinearVelocity(geom::Vec velocity){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::setLinearVelocity: physical object was null");
      getRigidBody()->setLinearVelocity(icl2bullet_scaled(velocity));
    }
    
    void RigidObject::setAngularVelocity(geom::Vec velocity){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::setAngularVelocity: physical object was null");
      getRigidBody()->setAngularVelocity(icl2bullet_unscaled(velocity));
    }
    
    
    geom::Vec RigidObject::getLinearVelocity(){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::getLinearVelocity: physical object was null");
      return bullet2icl_scaled(getRigidBody()->getLinearVelocity());
    }
    
    geom::Vec RigidObject::getAngularVelocity(){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::getAngularVelocity: physical object was null");
      return bullet2icl_unscaled(getRigidBody()->getAngularVelocity());
    }
    
    void RigidObject::applyForce(geom::Vec force, geom::Vec relPos){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::applyForce: physical object was null");
      getRigidBody()->applyForce(icl2bullet_scaled(force), icl2bullet_scaled(relPos));
    }
    
    void RigidObject::applyCentralForce(geom::Vec force){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::applyCentralForce: physical object was null");
      getRigidBody()->applyCentralForce(icl2bullet_scaled(force));
    }
    
    void RigidObject::setDamping(float linear, float angular){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::setDamping: physical object was null");
      getRigidBody()->setDamping(linear, angular);
    }
    
    RigidObject::~RigidObject(){
      delete getRigidBody()->getMotionState();
    }
    
    float RigidObject::getMass(){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::getMass: physical object was null");
      return 1.0 / getRigidBody()->getInvMass();
    }
    
    void RigidObject::setMass(float mass){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::setMass: physical object was null");
      btVector3 intertia;
      getRigidBody()->getCollisionShape()->calculateLocalInertia(mass, intertia);
      getRigidBody()->setMassProps(mass, intertia);
    }
  }
}
