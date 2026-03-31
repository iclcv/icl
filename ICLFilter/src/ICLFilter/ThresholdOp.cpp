/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ThresholdOp.cpp             **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

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
