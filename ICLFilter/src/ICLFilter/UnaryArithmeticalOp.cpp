/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryArithmeticalOp.cpp     **
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
