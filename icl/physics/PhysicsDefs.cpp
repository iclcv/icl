// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics/PhysicsDefs.h>

#include <icl/markers/QuadDetector.h>

float icl::physics::ICL_UNIT_TO_METER = 0.001f;
float icl::physics::METER_TO_BULLET_UNIT = 0.1f;

void ensure_correct_recursive_linkage_against_ICLMarkers(){
  icl::markers::QuadDetector qd;
}
