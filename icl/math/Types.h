// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/math/la/FixedMatrix.h>
#include <icl/math/la/FixedVector.h>

namespace icl::math {

  /// Canonical fixed-size vector/matrix aliases for 3D geometry.
  /** This is the single source of truth for the small homogeneous types used
      across the framework. Downstream modules (cv3d, viz3d, physics2, markers,
      …) alias their local `Vec`/`Mat` to these instead of re-declaring
      `FixedColVector`/`FixedMatrix` themselves, so there is exactly one place
      to change if the representation ever evolves (e.g. SIMD alignment). */

  /// 3D column vector (float)
  using Vec3 = FixedColVector<icl32f, 3>;
  /// homogeneous 3D column vector (float)
  using Vec4 = FixedColVector<icl32f, 4>;
  /// 3x3 fixed matrix (float)
  using Mat3 = FixedMatrix<icl32f, 3, 3>;
  /// 4x4 homogeneous fixed matrix (float)
  using Mat4 = FixedMatrix<icl32f, 4, 4>;

  /// 3D column vector (double)
  using Vec3d = FixedColVector<icl64f, 3>;
  /// homogeneous 3D column vector (double)
  using Vec4d = FixedColVector<icl64f, 4>;
  /// 3x3 fixed matrix (double)
  using Mat3d = FixedMatrix<icl64f, 3, 3>;
  /// 4x4 homogeneous fixed matrix (double)
  using Mat4d = FixedMatrix<icl64f, 4, 4>;

} // namespace icl::math
