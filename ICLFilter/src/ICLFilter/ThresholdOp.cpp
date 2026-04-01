// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/ThresholdOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

    const char* toString(ThresholdOp::Op op) {
      switch(op) {
        case ThresholdOp::Op::ltVal:   return "ltVal";
        case ThresholdOp::Op::gtVal:   return "gtVal";
        case ThresholdOp::Op::ltgtVal: return "ltgtVal";
      }
      return "?";
    }

    core::ImageBackendDispatching& ThresholdOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.addSelector<ThreshSig>(Op::ltVal);
        proto.addSelector<ThreshSig>(Op::gtVal);
        proto.addSelector<ThreshDualSig>(Op::ltgtVal);
        return true;
      }();
      (void)init;
      return proto;
    }

    // Constructor — clones selectors from the class prototype
    ThresholdOp::ThresholdOp(optype ttype, float lowThreshold,
                                   float highThreshold, float lowVal, float highVal)
      : ImageBackendDispatching(prototype()),
        m_eType(ttype), m_fLowThreshold(lowThreshold),
        m_fHighThreshold(highThreshold), m_fLowVal(lowVal), m_fHighVal(highVal)
    {}

    void ThresholdOp::apply(const Image &src, Image &dst) {
      if(!prepare(dst, src)) return;

      auto& swLT   = getSelector<ThreshSig>(Op::ltVal);
      auto& swGT   = getSelector<ThreshSig>(Op::gtVal);
      auto& swLTGT = getSelector<ThreshDualSig>(Op::ltgtVal);

      switch(m_eType) {
        case lt:
          swLT.resolve(src)->apply(src, dst, m_fLowThreshold, m_fLowThreshold);
          break;
        case gt:
          swGT.resolve(src)->apply(src, dst, m_fHighThreshold, m_fHighThreshold);
          break;
        case ltgt:
          swLTGT.resolve(src)->apply(src, dst,
                        m_fLowThreshold, m_fLowThreshold,
                        m_fHighThreshold, m_fHighThreshold);
          break;
        case ltVal:
          swLT.resolve(src)->apply(src, dst, m_fLowThreshold, m_fLowVal);
          break;
        case gtVal:
          swGT.resolve(src)->apply(src, dst, m_fHighThreshold, m_fHighVal);
          break;
        case ltgtVal:
          swLTGT.resolve(src)->apply(src, dst,
                        m_fLowThreshold, m_fLowVal,
                        m_fHighThreshold, m_fHighVal);
          break;
      }
    }

  } // namespace filter
} // namespace icl
