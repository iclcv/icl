// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/GeomDefs.h>
#include <iostream>

namespace icl::geom {
    /// Utility structure for calculation of view-ray / plane intersections
    struct ICLGeom_API PlaneEquation{

      /// Constructor with given offset and direction vector
      explicit PlaneEquation(const Vec &offset=Vec(), const Vec &normal=Vec());

        /// line offset
      Vec offset;

      /// line direction
      Vec normal;
    };

    /// ostream operator
    ICLGeom_API std::ostream &operator<<(std::ostream &s, const PlaneEquation &p);
  } // namespace icl::geom