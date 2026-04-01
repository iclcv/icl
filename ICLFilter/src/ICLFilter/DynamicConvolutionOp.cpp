// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/DynamicConvolutionOp.h>
#include <ICLCore/Img.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{

    DynamicConvolutionOp::DynamicConvolutionOp (const ImgBase* poKernel) :
       ConvolutionOp ()
    {
       poKernelBuf = new Img<icl32f>(Size(3,3), 1);
       if (poKernel) setKernel (poKernel);
    }

    DynamicConvolutionOp::~DynamicConvolutionOp () {
       delete poKernelBuf;
    }

    void DynamicConvolutionOp::setKernel (const ImgBase* poKernel) {
       ICLASSERT_RETURN(poKernel->getChannels() == 1);

       // resize kernel buffer if necessary
       if (poKernel->getROISize() != poKernelBuf->getSize())
          poKernelBuf->setSize (poKernel->getROISize());

       // copy data from poKernel's ROI to poKernelBuf
       poKernel->convertROI(poKernelBuf);

       // set ConvolutionOp kernel from float data
       ConvolutionOp::setKernel (ConvolutionKernel(poKernelBuf->getData(0), poKernelBuf->getSize(), false));
    }
  }
}
