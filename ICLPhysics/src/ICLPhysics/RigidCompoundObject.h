// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLPhysics/RigidObject.h>

namespace icl{
  namespace physics{
    /// A Box that can with RigidObject features
    class ICLPhysics_API RigidCompoundObject : public RigidObject{

      public:
      /// Constructor that takes the position and  dimensions, as well as mass of the box
      RigidCompoundObject(float x, float y, float z, float mass=1.0);
      void addChild(RigidObject* obj, bool passOwnership=true);
      void removeChild(RigidObject* obj);
      void removeAllChildren();
    };
  }
}
