/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ConvolutionOp.cpp              **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke, Andre Justus      **
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

#include <ICLFilter/ConvolutionOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{

    const char* toString(ConvolutionOp::Op op) {
      switch(op) {
        case ConvolutionOp::Op::apply: return "apply";
      }
      return "?";
    }

    core::ImageBackendDispatching& ConvolutionOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.addSelector<ConvSig>(Op::apply);
        return true;
      }();
      (void)init;
      return proto;
    }

    ConvolutionOp::ConvolutionOp(const ConvolutionKernel &kernel):
      NeighborhoodOp(kernel.getSize()),
      ImageBackendDispatching(prototype()),
      m_forceUnsignedOutput(false){
      setKernel(kernel);
    }
    ConvolutionOp::ConvolutionOp(const ConvolutionKernel &kernel, bool forceUnsignedOutput):
      NeighborhoodOp(kernel.getSize()),
      ImageBackendDispatching(prototype()),
      m_forceUnsignedOutput(forceUnsignedOutput){
      setKernel(kernel);
    }

    void ConvolutionOp::apply(const core::Image &src, core::Image &dst) {
      ICLASSERT_RETURN(!src.isNull());
      ICLASSERT_RETURN(!m_kernel.isNull());

      depth dstDepth = m_forceUnsignedOutput ? src.getDepth()
                       : (src.getDepth() == depth8u ? depth16s : src.getDepth());
      if(!prepare(dst, src, dstDepth)) return;

      if(src.getDepth() >= depth32f){
        m_kernel.toFloat();
      }else if(m_kernel.isFloat()){
        WARNING_LOG("convolution of non-float images with float kernels is not supported\n"
                    "use an int-kernel instead. For now, the kernel is casted to int-type");
        m_kernel.toInt(true);
      }

      getSelector<ConvSig>(Op::apply).resolve(src)->apply(src, dst, *this);
    }

  } // namespace filter
}
