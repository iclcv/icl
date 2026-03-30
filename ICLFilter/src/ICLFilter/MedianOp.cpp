/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/MedianOp.cpp                   **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke, Andre Justus,     **
**          Sergius Gaulik                                         **
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

#include <ICLFilter/MedianOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    const char* toString(MedianOp::Op op) {
      switch(op) {
        case MedianOp::Op::fixed: return "fixed";
        case MedianOp::Op::generic: return "generic";
      }
      return "?";
    }

    core::ImageBackendDispatching& MedianOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.initDispatching("MedianOp");
        proto.addSelector<MedianFixedSig>(Op::fixed);
        proto.addSelector<MedianGenericSig>(Op::generic);
        return true;
      }();
      (void)init;
      return proto;
    }

    // Constructor — clones selectors from the class prototype
    MedianOp::MedianOp(const Size &maskSize)
      : NeighborhoodOp(adaptSize(maskSize)),
        ImageBackendDispatching(prototype())
    {}

    void MedianOp::apply(const core::Image &src, core::Image &dst) {
      if (!prepare(dst, src)) return;
      const Size &ms = getMaskSize();
      if (ms == Size(3,3) || ms == Size(5,5)) {
        getSelector<MedianFixedSig>(Op::fixed).resolve(src)->apply(
          src, dst, ms.width, getROIOffset());
      } else {
        getSelector<MedianGenericSig>(Op::generic).resolve(src)->apply(
          src, dst, ms, getROIOffset(), getAnchor());
      }
    }

  } // namespace filter
}
