// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/filter/UnaryOp.h>
#include <icl/core/ImageBackendDispatching.h>
#include <icl/core/Image.h>

namespace icl::filter {
  /// UnaryCompareOp using the BackendDispatch architecture.
  class ICLFilter_API UnaryCompareOp : public UnaryOp, public core::ImageBackendDispatching {
    public:

    enum optype { lt, lteq, eq, gteq, gt, eqt };

    /// Backend selector keys (2 distinct operations). Values must match addSelector() order.
    enum class Op : int { compare, compareEqTol };

    /// Translate a string relation to an optype
    static optype translate_op_type(const std::string &s) {
      if(s == "<") return lt;
      if(s == ">") return gt;
      if(s == "<=") return lteq;
      if(s == ">=") return gteq;
      if(s == "==") return eq;
      if(s == "~=") return eqt;
      throw utils::ICLException("UnaryCompareOp::translate_op_type(" + s + "): invalid optype string!");
      return lt;
    }

    UnaryCompareOp(optype ot = gt, icl64f value = 128, icl64f tolerance = 0);

    /// String-based constructor (e.g. ">", "<=", "==", "~=")
    UnaryCompareOp(const std::string &op, icl64f value = 128, icl64f tolerance = 0);

    void apply(const core::Image &src, core::Image &dst) override;
    using UnaryOp::apply;

    void setOpType(optype ot) { m_eOpType = ot; }
    optype getOpType() const { return m_eOpType; }
    void setValue(icl64f value) { m_dValue = value; }
    icl64f getValue() const { return m_dValue; }
    void setTolerance(icl64f tolerance) { m_dTolerance = tolerance; }
    void setTollerance(icl64f tolerance) { m_dTolerance = tolerance; }
    icl64f getTolerance() const { return m_dTolerance; }

    // Sub-op signatures for backend dispatch
    using CmpSig    = void(const core::Image&, core::Image&, double, int);
    using CmpEqtSig = void(const core::Image&, core::Image&, double, double);

    /// Class-level prototype — owns selectors, populated during static init
    static core::ImageBackendDispatching& prototype();

    private:
    optype m_eOpType;
    icl64f m_dValue;
    icl64f m_dTolerance;
  };

  /// ADL-visible toString for UnaryCompareOp::Op → registry name (defined in UnaryCompareOp.cpp)
  ICLFilter_API const char* toString(UnaryCompareOp::Op op);

  } // namespace icl::filter