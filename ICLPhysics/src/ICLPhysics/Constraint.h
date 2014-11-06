#pragma once

#include <ICLUtils/Uncopyable.h>
#include <BulletDynamics/ConstraintSolver/btTypedConstraint.h>
#include <ICLPhysics/RigidObject.h>
#include <vector>

namespace icl {
  namespace physics{
    
    /// Base Class for constraints
    class ICLPhysics_API Constraint:public utils::Uncopyable{
      
      protected:
      /// internal constraint
      btTypedConstraint *m_constraint;
      std::vector<RigidObject*> m_objects;
      bool m_collide;

      void initUserPointer();

      public:
      /// Base cosntructor
      Constraint();

      /// Destructor
      virtual ~Constraint();
      
      /// Getter for the internal constraint
      btTypedConstraint* getConstraint();
      
      /// Getter for the objects involved in the constraint
      std::vector<RigidObject*>& getObjects();
    };
  }
}
