// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

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
