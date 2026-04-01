// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/BinaryOp.h>
#include <ICLCore/ImageBackendDispatching.h>

namespace icl {
  namespace filter {

    /// Class for arithmetic operations performed on two images. \ingroup BINARY
    class ICLFilter_API BinaryArithmeticalOp : public BinaryOp, public core::ImageBackendDispatching {
      public:

      enum optype { addOp, subOp, mulOp, divOp, absSubOp };

      /// Backend selector key (single operation dispatched by optype argument)
      enum class Op : int { apply };

      BinaryArithmeticalOp(optype t);

      void apply(const core::Image &src1, const core::Image &src2, core::Image &dst) override;
      using BinaryOp::apply;

      void setOpType(optype t) { m_eOpType = t; }
      optype getOpType() const { return m_eOpType; }

      using Sig = void(const core::Image&, const core::Image&, core::Image&, int);

      /// Class-level prototype — owns selectors, populated during static init
      static core::ImageBackendDispatching& prototype();

      private:
      optype m_eOpType;
    };

    /// ADL-visible toString for BinaryArithmeticalOp::Op → registry name
    ICLFilter_API const char* toString(BinaryArithmeticalOp::Op op);

  } // namespace filter
} // namespace icl
