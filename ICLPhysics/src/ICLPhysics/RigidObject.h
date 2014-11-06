#pragma once

#include <ICLPhysics/PhysicsObject.h>
#include <ICLGeom/GeomDefs.h>

class btRigidBody;

namespace icl{
  namespace physics{
    /// This class combines a movable but rigid physics object and it's graphical representation in ICL
    /** Actually, this sub-class is just an explicit restriction of the general PhysicalObject class,
        that can represent:
        - collision objects 
        - rigid object
        - soft body objects 
    */
    class ICLPhysics_API RigidObject : public PhysicsObject{
      
      public:
      RigidObject();
      
      virtual btRigidBody *getRigidBody();
      /// returns internal physical object as rigidBody (const)
      /** Rigid bodys are movable object that are rigid */
      virtual const btRigidBody *getRigidBody() const { 
        return this->getRigidBody(); 
      }
      
      /// sets the linear velocity of that object
      void setLinearVelocity(geom::Vec velocity);
      
      /// sets the angular velocity of that object
      void setAngularVelocity(geom::Vec velocity);
      
      /// sets the linear velocity of that object
      geom::Vec getLinearVelocity();
      
      /// sets the angular velocity of that object
      geom::Vec getAngularVelocity();
      
      ///apply a force at the point relPos
      void applyForce(geom::Vec force, geom::Vec relPos);
      
      ///apply a force to the center
      void applyCentralForce(geom::Vec force);
      
      /// sets the angular and linear damping of that object
      void setDamping(float linear, float angular);
      
      virtual ~RigidObject();
      
      /// returns the mass of the object
      float getMass();
      /// sets the mass of the object. 0 weight makes the object static.
      void setMass(float mass);
    };
  }
}
