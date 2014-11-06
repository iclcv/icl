#pragma once

#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Lockable.h>
#include <ICLMath/DynMatrix.h>
#include <ICLGeom/Geom.h>
#include <ICLPhysics/PhysicsDefs.h>

#include <vector>


/** \cond */
class btSoftBodyWorldInfo;
/** \endcond */

namespace icl{
  namespace geom{
    class ViewRay;
  }
  namespace physics{
    
    class PhysicsObject;
    class RigidObject;
    class Constraint;

    /// A physical world that handles physical objects
    class ICLPhysics_API PhysicsWorld : public utils::Lockable, public utils::Uncopyable{
      
      friend class PhysicsObject;
      /// internal data structure (hidden)
      struct Data;
      
      /// internal data pointer
      Data *data;
      
      protected:
      
      /// removes contactpoints (used when the collisionshape of an object has changed)
      void removeContactPoints(PhysicsObject *obj);
      
      public:

      /// constructor with given config file name
      PhysicsWorld();

      /// Destructor
      ~PhysicsWorld();
      
      /// adds a physics object to the world (ownership is not passed)
      void addObject(PhysicsObject *obj);
      
      /// removes the given physics object from the world
      void removeObject(PhysicsObject *obj);
      
      ///sets the Gravity of the World
      void setGravity(geom::Vec gravity);
      
      ///enable splitImpulse
      void splitImpulseEnabled(bool enable);
      
      /// applies physical simulation for the given time step
      /** If the given time interval tdSeconds is < 0, the actual time interval since
          the last call of this method is used */
      void step(float dtSeconds=-1, int maxSubSteps=10, float fixedTimeStep=1.f/120.f);
      
      ///check collision of an object with the world
      bool collideWithWorld(RigidObject* obj, bool ignoreJoints = true);
      
      /// enables/disables collision between the group0 and group1
      void setGroupCollision(int group0, int group1, bool collides);
      
      /// returns wether group0 and group1 collide
      bool getGroupCollision(int group0, int group1);
      
      /// Return true if the ray hit and sets the pointer to the first object that was hit as well as the hit normal and hit point
      bool rayCast(const geom::ViewRay& ray, float rayLength, PhysicsObject*& obj, geom::Vec &normal, geom::Vec &hitPoint);
      
      /// adds a constraint to the world
      void addConstraint(Constraint* constraint, bool disableCollisionWithLinkedBodies = false, bool passOwnerShip=false);
      
      /// removes a cosntraint from the world
      void removeConstraint(Constraint* constraint);

      /// returns an internal world-info struct
      const btSoftBodyWorldInfo *getWorldInfo() const;

      /// returns an internal world-info struct
      btSoftBodyWorldInfo *getWorldInfo();
    };
  }
}
