// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLGeom/PlaneEquation.h>

namespace icl::geom {
    PlaneEquation::PlaneEquation(const Vec &offset, const Vec &normal):
      offset(offset),normal(normal){
    }

    std::ostream &operator<<(std::ostream &s, const PlaneEquation &p){
      return s << "PlaneEquation( <X- " << p.offset.transp() << "," << p.normal.transp() << "> = 0)";
    }

  } // namespace icl::geom