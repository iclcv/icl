/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/Object2PointConstraint.h     **
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

#include <ICLGeom/GeomDefs.h>
#include <ICLPhysics/Constraint.h>

namespace icl {
  namespace physics{
    class RigidObject;

    /// This constraint binds an object to a point with a spring
    class ICLPhysics_API Object2PointConstraint: public Constraint{
      RigidObject* anchor;
      public:
      /// Constructor that takes all arguments
      Object2PointConstraint(RigidObject* obj, const geom::Vec& localOffset, const geom::Vec& point, float stiffness, float damping);
      /// Destructor
      virtual ~Object2PointConstraint();
      /// Sets the Point to which the object will be bound
      void setPoint(const geom::Vec& newPoint);
      /// Sets the offset of the constraint in local coordinates of the object
      void setLocalOffset(const geom::Vec& newLocalOffset);
      /// Sets the stiffness of the joint
      void setStiffness(float newStiffness);
      /// Sets the dampening of the joint
      void setDamping(float newDamping);
    };
  }
}
