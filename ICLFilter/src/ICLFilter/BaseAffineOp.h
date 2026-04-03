// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/UnaryOp.h>

namespace icl::filter {
  /// Abtract base class for arbitrary affine operation classes \ingroup AFFINE \ingroup UNARY
  /** The Base affine class complies an abtract interface class
      for all Filter classes implementing affine operations:
      - Affine (General Affine Transformations using 3x2 Matrix)
      - Rotate
      - Translate
      - Mirror
      - Scale
  */
  class ICLFilter_API BaseAffineOp : public UnaryOp{
    public:
    /// Destructor
    virtual ~BaseAffineOp(){}

    /// import from super class
    using UnaryOp::apply;
  };

  } // namespace icl::filter