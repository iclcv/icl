/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryLogicalOp.cpp             **
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

#include <ICLFilter/UnaryLogicalOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    const char* toString(UnaryLogicalOp::Op op) {
      switch(op) {
        case UnaryLogicalOp::Op::withVal: return "withVal";
        case UnaryLogicalOp::Op::noVal: return "noVal";
      }
      return "?";
    }

    core::ImageBackendDispatching& UnaryLogicalOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.initDispatching("UnaryLogicalOp");
        proto.addSelector<WithValSig>(Op::withVal);
        proto.addSelector<NoValSig>(Op::noVal);
        return true;
      }();
      (void)init;
      return proto;
    }

    // Constructor — clones selectors from the class prototype
    UnaryLogicalOp::UnaryLogicalOp(optype t, icl32s val)
      : ImageBackendDispatching(prototype()),
        m_eOpType(t), m_dValue(val)
    {}

    void UnaryLogicalOp::apply(const Image &src, Image &dst) {
      ICLASSERT_RETURN(src.getDepth() == depth8u || src.getDepth() == depth16s || src.getDepth() == depth32s);
      if(!prepare(dst, src)) return;

      if(m_eOpType == notOp) {
        getSelector<NoValSig>(Op::noVal).resolve(src)->apply(src, dst);
      } else {
        getSelector<WithValSig>(Op::withVal).resolve(src)->apply(src, dst, m_dValue, (int)m_eOpType);
      }
    }

  } // namespace filter
}
