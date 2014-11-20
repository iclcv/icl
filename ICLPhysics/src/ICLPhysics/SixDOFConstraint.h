/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/SixDOFConstraint.h           **
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

#include <ICLPhysics/Constraint.h>
#include <ICLGeom/GeomDefs.h>

namespace icl {
  namespace physics{
    class RigidObject;
  
    /// This is a generic constraint that allows to free configuration of the limits on all 6 degrees of freedom.
    class ICLPhysics_API SixDOFConstraint: public Constraint{
      void init(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, const bool useLinearReferenceFrameA);
      public:
      /// The frames describe the position and orientation of the pivot in the objectspaces of the 2 objects.
      SixDOFConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, const bool useLinearReferenceFrameA = true);
      SixDOFConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB,bool useLinearReferenceFrameA = true);
      /// This constructor uses the identity geom::Matrices as frames.
      SixDOFConstraint(RigidObject* a, RigidObject* b, const bool useLinearReferenceFrameA = true);
      /// The frames describe the position and orientation of the constraint pivot in the objectspace of the 2 objects.
      void setFrames(const geom::Mat &frameA, const geom::Mat &frameB);
      void setPivot(const geom::Vec &pivotInA, const geom::Vec &pivotInB);
      
      /* Set lower linear limits of the constraint pivot.
       * Setting the lower limit to higher than the upper limit unlocks the axis.
       */
      void setLinearLowerLimit(const geom::Vec &lower);
      
      /* Set upper linear limits of the constraint pivot.
       * Setting the lower limit to higher than the upper limit unlocks the axis.
       */
      void setLinearUpperLimit(const geom::Vec &upper);
      
      /// Return the lower linear limit.
      geom::Vec getLinearLowerLimit();
      
      /// Return the upper linear limit.
      geom::Vec getLinearUpperLimit();
      
      
      
      /* Set lower angular limits of the constraint pivot.
       * Setting the lower limit to higher than the upper limit unlocks the axis.
       */
      void setAngularLowerLimit(const geom::Vec &lower);
      
      /* Set upper linear limits of the constraint pivot.
       * Setting the lower limit to higher than the upper limit unlocks the axis.
       */
      void setAngularUpperLimit(const geom::Vec &upper);
      
      /// Return the lower angular limit.
      geom::Vec getAngularLowerLimit();
      
      /// Return the upper angular limit.
      geom::Vec getAngularUpperLimit();
      
      /// Set the angular motor settings.
      void setLinearMotor(int index, bool enableMotor, float targetVelocity, float maxMotorForce);
      
      /// Set the linear motor settings.
      void setAngularMotor(int index, bool enableMotor, float targetVelocity, float maxMotorForce);
    };
  }
}
