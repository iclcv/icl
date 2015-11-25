/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsWorld.h               **
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
      
		public:

			enum BulletSolverType {
				SequentialImpulseConstraintSolver,
				MLCP_Dantzig,
				NNCG,
				Lemke,
				Default = SequentialImpulseConstraintSolver
			};

		private:

      friend class PhysicsObject;
      /// internal data structure (hidden)
      struct Data;
      
      /// internal data pointer
      Data *data;
      
			void setSolver(BulletSolverType type);

		protected:

      /// removes contactpoints (used when the collisionshape of an object has changed)
      void removeContactPoints(PhysicsObject *obj);

		public:

      /// constructor with given config file name
			PhysicsWorld(BulletSolverType solver_type = Default);

      /// Destructor
      ~PhysicsWorld();
      
      /// adds a physics object to the world (ownership is not passed)
      void addObject(PhysicsObject *obj);
      
      /// removes the given physics object from the world
      void removeObject(PhysicsObject *obj);
      
      ///sets the Gravity of the World
      void setGravity(const geom::Vec &gravity);

      /// enables/disables gravity
      /** When enabling gravity (on = true), either the given gravity
          value can be used or if useThisGravityIfOn is null, the default gravity
          (0,0, -9810) is used */
      void setGravityEnabled(bool on, const geom::Vec *useThisGravityIfOn=0);

      ///enable splitImpulse
      void splitImpulseEnabled(bool enable);
      
      /// applies physical simulation for the given time step
      /** If the given time interval tdSeconds is < 0, the actual time interval since
          the last call of this method is used */
      void step(float dtSeconds=-1, int maxSubSteps=10, float fixedTimeStep=1.f/120.f);

			/// returns the last delta of time in seconds as a double value.
			double getLastTimeDelta();
      
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
