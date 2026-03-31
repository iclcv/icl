/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryArithmeticalOp.cpp       **
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

#include <ICLFilter/BinaryArithmeticalOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

    const char* toString(BinaryArithmeticalOp::Op op) {
      switch(op) {
        case BinaryArithmeticalOp::Op::apply: return "apply";
      }
      return "?";
    }

    core::ImageBackendDispatching& BinaryArithmeticalOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.addSelector<Sig>(Op::apply);
        return true;
      }();
      (void)init;
      return proto;
    }

    // Constructor — clones selectors from the class prototype
    BinaryArithmeticalOp::BinaryArithmeticalOp(optype t)
      : ImageBackendDispatching(prototype()),
        m_eOpType(t)
    {}

    void BinaryArithmeticalOp::apply(const Image &src1, const Image &src2, Image &dst) {
      if(!check(src1, src2)) return;
      if(!prepare(dst, src1)) return;
      getSelector<Sig>(Op::apply).resolve(src1)->apply(src1, src2, dst, static_cast<int>(m_eOpType));
    }

  } // namespace filter
} // namespace icl
