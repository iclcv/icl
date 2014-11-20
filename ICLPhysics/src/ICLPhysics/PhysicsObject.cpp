/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsObject.cpp            **
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
#include <ICLPhysics/PhysicsObject.h>
#include <ICLPhysics/PhysicsWorld.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletSoftBody/btSoftBody.h>

namespace icl{
  namespace physics{
    PhysicsObject::~PhysicsObject(){
      if(m_physicalObject) {
        // delete m_physicalObject->getCollisionShape(); this  is automatically deleted by the physical object!
        delete m_physicalObject;
      }
    }
    void PhysicsObject::setPhysicalObject(btCollisionObject *obj){
      delete m_physicalObject;
      m_physicalObject = obj;
      //set the userpointer to the physicsobject so it can be accessed in collision callbacks
      m_physicalObject->setUserPointer(this);
    }

    void PhysicsObject::setCollisionMargin(float margin){
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::setCollisionMargin physical object was null");
      m_physicalObject->getCollisionShape()->setMargin(icl2bullet(margin));
    }
    
    void PhysicsObject::setFriction(float friction){
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::setFriction: physical object was null");
      m_physicalObject->setFriction(friction);
    }
    
    void PhysicsObject::setRollingFriction(float friction){
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::setRollingFriction: physical object was null");
      m_physicalObject->setRollingFriction(friction);
    }
    
    void PhysicsObject::setTransformation(const geom::Mat &m)
    {
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::setTransformation: physical object was null");
      btTransform trans = icl2bullet_scaled_mat(m);
      getCollisionObject()->setWorldTransform(trans);
      m_stateChanged = true;
    }
      
    void PhysicsObject::transform(const geom::Mat &m)
    {
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::transform: physical object was null");
      btTransform trans = getCollisionObject()->getWorldTransform();
      trans = icl2bullet_scaled_mat(m) * trans;
      getCollisionObject()->setWorldTransform(trans);
      m_stateChanged = true;
    }
    
    geom::Mat PhysicsObject::getTransformation(){
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::getTransform: physical object was null");
      updateSceneObject();
      return SceneObject::getTransformation();
    }
    
    void PhysicsObject::setScale(geom::Vec scale) {
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::setScale: physical object was null");
      getCollisionObject()->getCollisionShape()->setLocalScaling(icl2bullet_unscaled(scale));
      m_stateChanged = true;
      activate(true);
      //remove the contactpoints so the collision will be updated
      if(m_world)m_world->removeContactPoints(this);
    }
    
    geom::Vec PhysicsObject::getScale() {
      return bullet2icl_unscaled(getCollisionObject()->getCollisionShape()->getLocalScaling());
    }
    
    void PhysicsObject::updateSceneObject()
    {
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::updateSceneObject: physical object was null");
      if(m_stateChanged) {
        geom::Vec scale(getScale());
        geom::Mat scaleMat(scale[0],0,0,0,
                     0,scale[1],0,0,
                     0,0,scale[2],0,
                     0,0,0       ,1);
        SceneObject::setTransformation(bullet2icl(getCollisionObject()->getWorldTransform()) * scaleMat);
        m_stateChanged = false;
      }
    }

    void PhysicsObject::setRestitution(float restitution)
    {
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::setFriction: physical object was null");
      m_physicalObject->setRestitution(restitution);
    }
    
    void PhysicsObject::setCollisionGroup(int group)
    {
      m_collisionGroup = group;
    }
    
    int PhysicsObject::getCollisionGroup()
    {
      return m_collisionGroup;
    }
    
    void PhysicsObject::setContactResponse(bool hasResponse){
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::setCollisionResponse: physical object was null");
      if(hasResponse)m_physicalObject->setCollisionFlags(0);
      else m_physicalObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
    
    bool PhysicsObject::hasContactResponse(){
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::hasCollisionResponse: physical object was null");
      return m_physicalObject->hasContactResponse();
    }
    
    void PhysicsObject::activate(bool forceActivation)
    {
      if(!getCollisionObject()) throw utils::ICLException("PhysicsObject::activate(): physical object was null");
      m_physicalObject->activate(forceActivation);
    }
    
    void PhysicsObject::stateChanged()
    {
      m_stateChanged = true;
    }
    
    void PhysicsObject::setCurrentPhysicsWorld(PhysicsWorld *world) {
      m_world = world;
    }

    void PhysicsObject::setCollisionCallback(utils::Function<void,PhysicsObject*,PhysicsObject*, geom::Vec> collisionCallback) {
      m_collisionCallback = collisionCallback;
    }

    void PhysicsObject::collisionCallback(PhysicsObject* self, PhysicsObject* other, geom::Vec pos) {
      m_collisionCallback(self, other,pos);
    }
  }
}
