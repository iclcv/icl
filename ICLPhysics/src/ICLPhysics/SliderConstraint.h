/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/SliderConstraint.h           **
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

#include <ICLPhysics/SixDOFConstraint.h>

namespace icl {
  namespace physics{
    /// This constraint simulates a Slider joint.
    class ICLPhysics_API SliderConstraint: public SixDOFConstraint{
      int m_rotationAxis;
      void init();
      public:
      /// Creates the Slider, with the specified frame along the rotationAxis(0 = x, 1 = y, 2 = z).
      SliderConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, int rotationAxis, const bool useLinearReferenceFrameA = true);
      /// Creates the Slider, with the specified frame along the rotationAxis(0 = x, 1 = y, 2 = z).
      SliderConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB, int rotationAxis, const bool useLinearReferenceFrameA = true);

      /* This sets the angular limit, for the specified axis.
       * Possible ranges:
       * X-Axis: lower = -PI, upper = PI
       * Y-Axis: lower = -PI/2, upper = PI/2
       * Z-Axis: lower = -PI, upper = PI
       *
       * Setting the lower limit to be higher than the upper limit unlocks rotation on this axis.
       */
      void setAngularLimits(float lower, float upper);
      /* This sets the linear limit, for the specified axis.
       * Setting the lower limit to be higher than the upper limit unlocks translation along this Axis.
       */
      void setLinearLimits(float lower, float upper);
    };
  }
}
