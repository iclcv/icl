// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/la/FixedMatrix.h>

namespace icl::math {
  /// A column vector is just a single-column FixedMatrix (DIM rows, 1 col)
  template<class T, int DIM>
  using FixedColVector = FixedMatrix<T, DIM, 1>;

  /// A row vector is just a single-row FixedMatrix (1 row, DIM cols)
  template<class T, int DIM>
  using FixedRowVector = FixedMatrix<T, 1, DIM>;

  } // namespace icl::math