/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/WienerOp.cpp                   **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#include <ICLFilter/WienerOp.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    const char* toString(WienerOp::Op op) {
      switch(op) {
        case WienerOp::Op::apply: return "apply";
      }
      return "?";
    }

    core::ImageBackendDispatching& WienerOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.initDispatching("WienerOp");
        proto.addSelector<WienerSig>(Op::apply);
        return true;
      }();
      (void)init;
      return proto;
    }

    WienerOp::WienerOp(const Size &maskSize, icl32f noise)
      : NeighborhoodOp(maskSize), ImageBackendDispatching(prototype()),
        m_fNoise(noise)
    {}

    void WienerOp::apply(const Image &src, Image &dst) {
      if(!prepare(dst, src)) return;
      getSelector<WienerSig>(Op::apply).resolve(src)->apply(
        src, dst, getMaskSize(), getAnchor(), getROIOffset(), m_fNoise);
    }

  } // namespace filter
}
