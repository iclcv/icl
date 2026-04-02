// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/BinaryOp.h>
#include <ICLCore/ImageBackendDispatching.h>

namespace icl::filter {
  /// Class for comparing two images pixel-wise \ingroup BINARY
  class ICLFilter_API BinaryCompareOp : public BinaryOp, public core::ImageBackendDispatching {
    public:

    enum optype { lt, lteq, eq, gteq, gt, eqt };

    /// Backend selector keys. Values must match addSelector() order.
    enum class Op : int { compare, compareEqTol };

    BinaryCompareOp(optype ot, icl64f tolerance = 0);

    void apply(const core::Image &src1, const core::Image &src2, core::Image &dst) override;
    using BinaryOp::apply;

    optype getOpType() const { return m_eOpType; }
    void setOpType(optype ot) { m_eOpType = ot; }
    icl64f getTolerance() const { return m_dTolerance; }
    void setTolerance(icl64f tolerance) { m_dTolerance = tolerance; }

    using CmpSig    = void(const core::Image&, const core::Image&, core::Image&, int);
    using CmpEqtSig = void(const core::Image&, const core::Image&, core::Image&, double);

    /// Class-level prototype — owns selectors, populated during static init
    static core::ImageBackendDispatching& prototype();

    private:
    optype m_eOpType;
    icl64f m_dTolerance;
  };

  /// ADL-visible toString for BinaryCompareOp::Op → registry name (defined in BinaryCompareOp.cpp)
  ICLFilter_API const char* toString(BinaryCompareOp::Op op);

  } // namespace icl::filter