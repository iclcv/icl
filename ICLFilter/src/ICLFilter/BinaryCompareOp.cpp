/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryCompareOp.cpp            **
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

#include <ICLFilter/BinaryCompareOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

    const char* toString(BinaryCompareOp::Op op) {
      switch(op) {
        case BinaryCompareOp::Op::compare: return "compare";
        case BinaryCompareOp::Op::compareEqTol: return "compareEqTol";
      }
      return "?";
    }

    core::ImageBackendDispatching& BinaryCompareOp::prototype() {
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

  } // namespace filter
} // namespace icl
