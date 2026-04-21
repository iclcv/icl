// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/physics/SixDOFConstraint.h>

namespace icl::physics {
    class RigidObject;

    /// This constraint simulates a ballsocket joint.
    class ICLPhysics_API BallSocketConstraint: public SixDOFConstraint{
      public:
      BallSocketConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, const bool useLinearReferenceFrameA = true);
      BallSocketConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB, const bool useLinearReferenceFrameA = true);
    };
  }