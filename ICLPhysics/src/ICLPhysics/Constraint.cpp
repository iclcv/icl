#include <ICLPhysics/Constraint.h>
namespace icl {
  namespace physics{

    void Constraint::initUserPointer() {
      m_constraint->setUserConstraintPtr(this);
    }

    Constraint::Constraint(){
    }

    Constraint::~Constraint(){
      delete m_constraint;
    }
    
    btTypedConstraint* Constraint::getConstraint(){
      return m_constraint;
    }
    
    std::vector<RigidObject*>& Constraint::getObjects() {
      return m_objects;
    }
  }
}
