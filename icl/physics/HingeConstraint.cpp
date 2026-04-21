// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics/HingeConstraint.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>
#include <icl/physics/PhysicsDefs.h>
#include <icl/physics/RigidBoxObject.h>

namespace icl::physics {
    HingeConstraint::HingeConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, int rotationAxis, const bool useLinearReferenceFrameA):
      SliderConstraint(a,b,frameInA,frameInB,rotationAxis,useLinearReferenceFrameA) {
      setLinearLowerLimit(geom::Vec(0,0,0));
      setLinearUpperLimit(geom::Vec(0,0,0));
    }

    HingeConstraint::HingeConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB, int rotationAxis, const bool useLinearReferenceFrameA):
      SliderConstraint(a,b,pivotInA,pivotInB,rotationAxis,useLinearReferenceFrameA) {
      setLinearLowerLimit(geom::Vec(0,0,0));
      setLinearUpperLimit(geom::Vec(0,0,0));
    }
  }