// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/BinaryArithmeticalOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

    const char* toString(BinaryArithmeticalOp::Op op) {
      switch(op) {
        case BinaryArithmeticalOp::Op::apply: return "apply";
      }
      return "?";
    }

    core::ImageBackendDispatching& BinaryArithmeticalOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.addSelector<Sig>(Op::apply);
        return true;
      }();
      (void)init;
      return proto;
    }

    // Constructor — clones selectors from the class prototype
    BinaryArithmeticalOp::BinaryArithmeticalOp(optype t)
      : ImageBackendDispatching(prototype()),
        m_eOpType(t)
    {}

    void BinaryArithmeticalOp::apply(const Image &src1, const Image &src2, Image &dst) {
      if(!check(src1, src2)) return;
      if(!prepare(dst, src1)) return;
      getSelector<Sig>(Op::apply).resolve(src1)->apply(src1, src2, dst, static_cast<int>(m_eOpType));
    }

  } // namespace filter
} // namespace icl
