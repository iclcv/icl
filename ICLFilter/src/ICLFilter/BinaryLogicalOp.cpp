// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/BinaryLogicalOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
    const char* toString(BinaryLogicalOp::Op op) {
      switch(op) {
        case BinaryLogicalOp::Op::apply: return "apply";
      }
      return "?";
    }

    core::ImageBackendDispatching& BinaryLogicalOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.addSelector<Sig>(Op::apply);
        return true;
      }();
      (void)init;
      return proto;
    }

    BinaryLogicalOp::BinaryLogicalOp(optype t)
      : ImageBackendDispatching(prototype()),
        m_eOpType(t)
    {}

    void BinaryLogicalOp::apply(const Image &src1, const Image &src2, Image &dst) {
      if(!check(src1, src2)) return;
      if(!prepare(dst, src1)) return;
      getSelector<Sig>(Op::apply).resolve(src1)->apply(src1, src2, dst, static_cast<int>(m_eOpType));
    }

  } // namespace icl::filter