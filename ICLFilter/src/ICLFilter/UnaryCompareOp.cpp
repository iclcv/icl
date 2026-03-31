/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryCompareOp.cpp          **
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

#include <ICLFilter/UnaryCompareOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

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

  } // namespace filter
} // namespace icl
