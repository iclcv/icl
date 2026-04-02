// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/UnaryCompareOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(UnaryCompareOp::Op op) {
    switch(op) {
      case UnaryCompareOp::Op::compare: return "compare";
      case UnaryCompareOp::Op::compareEqTol: return "compareEqTol";
    }
    return "?";
  }

  core::ImageBackendDispatching& UnaryCompareOp::prototype() {
    static core::ImageBackendDispatching proto;
    static bool init = [&] {
      proto.addSelector<CmpSig>(Op::compare);
      proto.addSelector<CmpEqtSig>(Op::compareEqTol);
      return true;
    }();
    (void)init;
    return proto;
  }

  // Constructor — clones selectors from the class prototype
  UnaryCompareOp::UnaryCompareOp(optype ot, icl64f value, icl64f tolerance)
    : ImageBackendDispatching(prototype()),
      m_eOpType(ot), m_dValue(value), m_dTolerance(tolerance)
  {}

  UnaryCompareOp::UnaryCompareOp(const std::string &op, icl64f value, icl64f tolerance)
    : UnaryCompareOp(translate_op_type(op), value, tolerance) {}

  // ================================================================
  // apply()
  // ================================================================

  void UnaryCompareOp::apply(const Image &src, Image &dst) {
    if(!prepare(dst, src, depth8u)) return;

    if(m_eOpType == eqt) {
      auto& sel = getSelector<CmpEqtSig>(Op::compareEqTol);
      sel.resolve(src)->apply(src, dst, m_dValue, m_dTolerance);
    } else {
      auto& sel = getSelector<CmpSig>(Op::compare);
      sel.resolve(src)->apply(src, dst, m_dValue, static_cast<int>(m_eOpType));
    }
  }

  } // namespace icl::filter