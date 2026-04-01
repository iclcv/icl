// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/UnaryArithmeticalOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

    const char* toString(UnaryArithmeticalOp::Op op) {
      switch(op) {
        case UnaryArithmeticalOp::Op::withVal: return "withVal";
        case UnaryArithmeticalOp::Op::noVal: return "noVal";
      }
      return "?";
    }

    core::ImageBackendDispatching& UnaryArithmeticalOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.addSelector<ArithValSig>(Op::withVal);
        proto.addSelector<ArithNoValSig>(Op::noVal);
        return true;
      }();
      (void)init;
      return proto;
    }

    // Constructor — clones selectors from the class prototype
    UnaryArithmeticalOp::UnaryArithmeticalOp(optype t, icl64f val)
      : ImageBackendDispatching(prototype()),
        m_eOpType(t), m_dValue(val)
    {}

    // ================================================================
    // apply()
    // ================================================================

    void UnaryArithmeticalOp::apply(const Image &src, Image &dst) {
      if(!prepare(dst, src)) return;

      switch(m_eOpType) {
        case addOp: case subOp: case mulOp: case divOp: {
          auto& sel = getSelector<ArithValSig>(Op::withVal);
          sel.resolve(src)->apply(src, dst, m_dValue, static_cast<int>(m_eOpType));
          break;
        }
        default: {
          auto& sel = getSelector<ArithNoValSig>(Op::noVal);
          sel.resolve(src)->apply(src, dst, static_cast<int>(m_eOpType));
          break;
        }
      }
    }

  } // namespace filter
} // namespace icl
