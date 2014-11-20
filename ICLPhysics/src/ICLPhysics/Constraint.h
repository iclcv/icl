/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/Constraint.h                 **
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
