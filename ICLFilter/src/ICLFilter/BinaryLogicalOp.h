// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/BinaryOp.h>
#include <ICLCore/ImageBackendDispatching.h>

namespace icl {
  namespace filter {

    /// Class for logical operations on two images (and, or, xor). \ingroup BINARY
    /// Only supports integer types (icl8u, icl16s, icl32s).
    class ICLFilter_API BinaryLogicalOp : public BinaryOp, public core::ImageBackendDispatching {
      public:

      enum optype { andOp, orOp, xorOp };

      /// Backend selector keys. Values must match addSelector() order.
      enum class Op : int { apply };

      BinaryLogicalOp(optype t);

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

    /// ADL-visible toString for BinaryLogicalOp::Op → registry name (defined in BinaryLogicalOp.cpp)
    ICLFilter_API const char* toString(BinaryLogicalOp::Op op);

  } // namespace filter
} // namespace icl
