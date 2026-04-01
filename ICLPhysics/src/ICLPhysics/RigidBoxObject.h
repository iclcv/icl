// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLPhysics/RigidObject.h>

namespace icl{
  namespace physics{
    /// A Box that can with RigidObject features
    class ICLPhysics_API RigidBoxObject : public RigidObject{
      protected:
      SceneObject *so;

      public:
      /// Constructor that takes the position and  dimensions, as well as mass of the box
      RigidBoxObject(float x, float y, float z, float dx, float dy, float dz, float mass=1.0);
    };
  }
}
