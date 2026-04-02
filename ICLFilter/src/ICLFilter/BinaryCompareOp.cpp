// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/BinaryCompareOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(BinaryCompareOp::Op op) {
    switch(op) {
      case BinaryCompareOp::Op::compare: return "compare";
      case BinaryCompareOp::Op::compareEqTol: return "compareEqTol";
    }
    return "?";
  }

  core::ImageBackendDispatching& BinaryCompareOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<CmpSig>(Op::compare);
      proto.addSelector<CmpEqtSig>(Op::compareEqTol);
      return true;
    }();
    return proto;
  }

  // Constructor — clones selectors from the class prototype
  BinaryCompareOp::BinaryCompareOp(optype ot, icl64f tolerance)
    : ImageBackendDispatching(prototype()),
      m_eOpType(ot), m_dTolerance(tolerance)
  {}

  void BinaryCompareOp::apply(const Image &src1, const Image &src2, Image &dst) {
    if(!check(src1, src2)) return;
    if(!prepare(dst, src1, depth8u)) return;
    if(m_eOpType == eqt) {
      getSelector<CmpEqtSig>(Op::compareEqTol).resolve(src1)->apply(src1, src2, dst, m_dTolerance);
    } else {
      getSelector<CmpSig>(Op::compare).resolve(src1)->apply(src1, src2, dst, static_cast<int>(m_eOpType));
    }
  }

  } // namespace icl::filter