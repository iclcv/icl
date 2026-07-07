// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/cv3d/Types.h>
#include <algorithm>
#include <cmath>

namespace icl {
namespace viz3d {

  /// Canonical procedural-sky gradient shared by every render backend.
  /** A three-band vertical gradient (ground → horizon → zenith) evaluated purely
      from the elevation `t = dir·up ∈ [-1,1]`. It is the single source of truth
      so the Filament backend (IBL cubemap + drawn skybox) and the Cycles world
      background render the *same* sky — matching reflections, ambient and
      background across the realtime preview and the path-traced view.

      The formula matches the Cycles background node graph exactly:
      - upper hemisphere (t>0):  mix(horizon, zenith, pow(t, horizonSharpness))
      - lower hemisphere (t<=0): mix(horizon, ground, clamp(-t*groundSharpness,0,1))
      All colours are LINEAR RGB. */
  struct Sky {
    cv3d::GeomColor zenith {0.55f, 0.65f, 0.85f, 1.0f};  ///< straight up
    cv3d::GeomColor horizon{0.95f, 0.93f, 0.90f, 1.0f};  ///< at the horizon
    cv3d::GeomColor ground {0.30f, 0.27f, 0.25f, 1.0f};  ///< straight down
    float horizonSharpness = 0.4f;   ///< pow() exponent for the upper hemisphere
    float groundSharpness  = 3.0f;   ///< linear falloff for the lower hemisphere
    float intensity        = 1.0f;   ///< scalar multiplier on the result

    /// Linear RGB along a direction, given its elevation cosine t = dir·up.
    cv3d::GeomColor colorForElevation(float t) const {
      const float k = (t > 0.0f)
          ? std::pow(std::min(t, 1.0f), horizonSharpness)
          : std::min(std::max(-t * groundSharpness, 0.0f), 1.0f);
      const cv3d::GeomColor &a = horizon;
      const cv3d::GeomColor &b = (t > 0.0f) ? zenith : ground;
      return cv3d::GeomColor{(a[0] + (b[0] - a[0]) * k) * intensity,
                             (a[1] + (b[1] - a[1]) * k) * intensity,
                             (a[2] + (b[2] - a[2]) * k) * intensity, 1.0f};
    }
  };

} // namespace viz3d
} // namespace icl
