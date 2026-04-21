// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/geom/GeomDefs.h>
#include <iostream>

namespace icl::geom {
  /** \cond */
  class SceneObject;
  /** \endcond */

  /// utility structure that defines a hit between a ViewRay and SceneObjects
  struct ICLGeom_API Hit{
    /// constructor (initializes obj with 0 and dist with -1)
    Hit():obj(0),dist(-1){}

    /// hit SceneObject
    SceneObject *obj;

    /// exact position in the world where it was hit
    Vec pos;

    /// distance to the originating viewrays origin
    float dist;

    /// for sorting by closest distance ot viewray origin
    inline bool operator<(const Hit &h) const { return dist < h.dist; }

    /// can be used to check wheter there was a hit at all
    operator bool() const { return obj; }

    /// friendly implemented ostream operator ...
    friend std::ostream &operator<<(std::ostream &str, const Hit &h){
      return (h ? (str << "Hit(obj=" << static_cast<void*>(h.obj) << ", dist=" << h.dist
                   << ", pos=" << h.pos.transp() << ")" )
              : (str << "Hit(NULL)"));
    }
  };



  } // namespace icl::geom