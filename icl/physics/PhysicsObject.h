// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom/SceneObject.h>
#include <icl/utils/Uncopyable.h>
#include <mutex>
#include <functional>
class btCollisionObject;

namespace icl::physics {
    class PhysicsWorld;

    /// This class combines an physical object and it's graphical representation in ICL
    class ICLPhysics_API PhysicsObject : public geom::SceneObject{
      ///physicsworld this object has been added to
      PhysicsWorld *m_world;
      /// internal physical object
      btCollisionObject *m_physicalObject;
      /// collisionGroup this object belongs to
      int m_collisionGroup;
      protected:
      /// is true if the physical state of the object has changed since the last updateSceneObject()
      bool m_stateChanged;

      std::function<void(PhysicsObject*,PhysicsObject*, geom::Vec)> m_collisionCallback;

			std::string m_id;

      public:
      PhysicsObject(const PhysicsObject&) = delete;
      PhysicsObject& operator=(const PhysicsObject&) = delete;

      /// Default constructor (initializint m_physicalObject with 0)
			PhysicsObject():m_world(0), m_physicalObject(0),m_collisionGroup(0), m_stateChanged(true), m_id("") {
        setLockingEnabled(true);
        //create an empty default callback
        struct defaultCallback {
          static void callback(PhysicsObject* self, PhysicsObject* other, geom::Vec pos) {}
        };
		m_collisionCallback = &defaultCallback::callback;
      }

      /// Destructor (freeing m_physicalObject if not 0)
      virtual ~PhysicsObject();

      /// sets the physical object (delets the old one of not 0)
      void setPhysicalObject(btCollisionObject *obj);

      /// sets the internal physical object o null, but wont delete it!
      void forgetPhysicalObject(bool removeFromWorld=true);

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

			/// sets the activation mode of this object
      void activate(bool forceActivation = false);

			int getCollisionObjectType();

			void setCollisionObjectType(int type);

			int getCollisionFlags();

			void setCollisionFlags(int flags);

      /// sets an internal flag to update the sceneobject
      void stateChanged();

      /// sets an internal pointer to the world the object is currently in
      void setCurrentPhysicsWorld(PhysicsWorld *world);

      /// sets the collision callback of the object
      void setCollisionCallback(std::function<void(PhysicsObject*,PhysicsObject*, geom::Vec)> collisionCallback);

			/// calls the collision callback of the object
      void collisionCallback(PhysicsObject* self, PhysicsObject* other, geom::Vec pos);

			/// sets the identifier string. There is no proof for beeing unique within the world
			void setObjectID(std::string const &id);

			/// returns the (not unique) object identifier
			std::string getObjectID();

			/// returns the (not unique) object identifier (const)
			std::string getObjectID() const;
    };
  }