// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke, Andre Justus

#include <icl/filter/ConvolutionOp.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(ConvolutionOp::Op op) {
    switch(op) {
      case ConvolutionOp::Op::apply: return "apply";
    }
    return "?";
  }

  core::ImageBackendDispatching& ConvolutionOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<ConvSig>(Op::apply);
      return true;
    }();
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

  } // namespace icl::filter