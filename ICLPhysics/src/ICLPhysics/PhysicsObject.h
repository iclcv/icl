/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsObject.h              **
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

#include <ICLGeom/SceneObject.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Function.h>
class btCollisionObject;

namespace icl {
  namespace physics{
    
    class PhysicsWorld;
    
    /// This class combines an physical object and it's graphical representation in ICL
    class ICLPhysics_API PhysicsObject : public geom::SceneObject, public utils::Uncopyable{
      ///physicsworld this object has been added to
      PhysicsWorld *m_world;
      /// internal physical object
      btCollisionObject *m_physicalObject;
      /// collisionGroup this object belongs to
      int m_collisionGroup;
      protected:
      /// is true if the physical state of the object has changed since the last updateSceneObject()
      bool m_stateChanged;

      utils::Function<void,PhysicsObject*,PhysicsObject*, geom::Vec> m_collisionCallback;

      public:
      /// Default constructor (initializint m_physicalObject with 0)
      PhysicsObject():m_world(0), m_physicalObject(0),m_collisionGroup(0), m_stateChanged(true){
        setLockingEnabled(true);
        //create an empty default callback
        struct defaultCallback {
          static void callback(PhysicsObject* self, PhysicsObject* other, geom::Vec pos) {}
        };
		m_collisionCallback = utils::function(&defaultCallback::callback);
      }

      /// Destructor (freeing m_physicalObject if not 0)
      virtual ~PhysicsObject();
      
      /// sets the physical object (delets the old one of not 0)
      void setPhysicalObject(btCollisionObject *obj);

      
      /// returns internal physical object as collision object
      /** Collision objects are like rigid objects, but they cannot be moved */
      virtual btCollisionObject *getCollisionObject() {
        return m_physicalObject;
      }

      /** Collision objects are like rigid objects, but they cannot be moved */
      virtual const btCollisionObject *getCollisionObject() const{
        return m_physicalObject;
      }

      /// this method is used to update the visualization object from the physical object
      virtual void updateSceneObject();
      
      /// called by the scene before it's rendered (calling purely virtual updateSceneObject)
      virtual void prepareForRendering() { 
        lock();
        updateSceneObject();
        unlock();
      }
      
      /// Sets the transform of this object. Scaling or shearing is not supported: Use setScale instead.
      virtual void setTransformation(const geom::Mat &m);
      /// applies the given transformation matrix. Scaling or shearing is not supported: Use setScale instead.
      virtual void transform(const geom::Mat &m);
      /// returns the transformation of this object
      geom::Mat getTransformation();
      
      void setScale(geom::Vec scale);
      geom::Vec getScale();
      
      /// sets the collision margin for this object
      void setCollisionMargin(float margin);
      
      /// sets the friction of this object
      void setFriction(float friction);
      
      ///sets the rolling friction of this obect
      void setRollingFriction(float friction);
      
      /// sets the collision group of this object
      void setCollisionGroup(int group);
      
      /// returns the collision group of this object
      int getCollisionGroup();
      
      /// enables or disable collision with other objects
      void setContactResponse(bool response);
      
      /// Checks if the object has a collisionresponse
      bool hasContactResponse();
      
      /// sets the restitution of this object
      void setRestitution(float restitution);
      
      /// sets the restitution of this object
      void activate(bool forceActivation = false);
      
      /// sets an internal flag to update the sceneobject
      void stateChanged();
      
      /// sets an internal pointer to the world the object is currently in
      void setCurrentPhysicsWorld(PhysicsWorld *world);

      /// sets the collision callback of the object
      void setCollisionCallback(utils::Function<void,PhysicsObject*,PhysicsObject*, geom::Vec> collisionCallback);

      /// sets the collision callback of the object
      void collisionCallback(PhysicsObject* self, PhysicsObject* other, geom::Vec pos);
    };
  }
}
