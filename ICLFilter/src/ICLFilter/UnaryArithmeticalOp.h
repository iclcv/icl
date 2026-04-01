// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/UnaryOp.h>
#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>

namespace icl {
  namespace filter {

    /// UnaryArithmeticalOp using the BackendDispatch architecture.
    class ICLFilter_API UnaryArithmeticalOp : public UnaryOp, public core::ImageBackendDispatching {
      public:

      enum optype {
        addOp=0, subOp=1, mulOp=2, divOp=3,
        sqrOp=10, sqrtOp=11, lnOp=12, expOp=13, absOp=14
      };

      /// Backend selector keys. Values must match addSelector() order in prototype().
      enum class Op : int { withVal = 0, noVal = 1 };

      UnaryArithmeticalOp(optype t = addOp, icl64f val = 0);

      void apply(const core::Image &src, core::Image &dst) override;
      using UnaryOp::apply;

      void setValue(icl64f value) { m_dValue = value; }
      icl64f getValue() const { return m_dValue; }
      void setOpType(optype t) { m_eOpType = t; }
      optype getOpType() const { return m_eOpType; }

      // Sub-op signatures for backend dispatch
      using ArithValSig   = void(const core::Image&, core::Image&, double, int);
      using ArithNoValSig = void(const core::Image&, core::Image&, int);

      /// Class-level prototype — owns selectors, populated during static init
      static core::ImageBackendDispatching& prototype();

      private:
      optype m_eOpType;
      icl64f m_dValue;
    };

    /// ADL-visible toString for UnaryArithmeticalOp::Op (defined in UnaryArithmeticalOp.cpp)
    ICLFilter_API const char* toString(UnaryArithmeticalOp::Op op);

  } // namespace filter
} // namespace icl
