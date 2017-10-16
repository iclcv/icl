/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/RigidObject.h                **
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

			enum ActivationMode {
				DYNAMIC = 0,	// ACTIVE_TAG - Disables the object after a certain time interval. Use setActive() to make it active again and reset the time-counter
				ACTIVE_FOREVER = 1,	// DISABLE_DEACTIVATION
				INACTIVE_FOREVER = 2,	// DISABLE_SIMULATION
				BT_ISLAND_SLEEPING = 3,	// ISLAND_SLEEPING - (not recommented)
				TOWARDS_INACTIVE = 4	// WANTS_DEAKTIVATION - disables the object after each step directly
			};

			/**
			 * @brief RigidObject
			 */
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

			/// returns the linear velocity of that object
      geom::Vec getLinearVelocity();

			/// returns the angular velocity of that object
      geom::Vec getAngularVelocity();

			geom::Vec getTotalForce();

			geom::Vec getTotalTorque();

      ///apply a force at the point relPos
      void applyForce(geom::Vec force, geom::Vec relPos);

      ///apply a force to the center
      void applyCentralForce(geom::Vec force);

			///apply an impulse at the point relPos
			void applyImpulse(geom::Vec force, geom::Vec relPos);

			///apply an impulse to the center
			void applyCentralImpulse(geom::Vec force);

			/// apply a torque to the center
			void applyTorque(geom::Vec t);

			/// apply a torque impulse to the center
			void applyTorqueImpulse(geom::Vec t);

			void setDeactivationTime(float time);

      /// sets the angular and linear damping of that object
      void setDamping(float linear, float angular);

			/// sets the corresponding bullet activation state for this object
			void setActivationMode(ActivationMode const mode);

			/// activates this component regarding the current activation mode
			void setActive(bool force_active = false);

      virtual ~RigidObject();

      /// returns the mass of the object
      float getMass();
      /// sets the mass of the object. 0 weight makes the object static.
      void setMass(float mass);
    };
  }
}
