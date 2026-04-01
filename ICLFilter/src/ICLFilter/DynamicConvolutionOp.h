// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/ConvolutionOp.h>

namespace icl{
  namespace filter{
    /// Convolution using the ROI of an ICL image as its kernel \ingroup UNARY \ingroup NBH
    /** Sometimes it is useful to use the ROI of an ICL image directly as the
        convolution kernel, e.g. for template matching. Because the ROI may be
        smaller than the image itself, the DynamicConvolution class maintains
        an internal buffer poKernelBuf of this ROI only. Its first channel is
        directly set as the (unbuffered) kernel data of the underlying Convolution
        class.
    */
    class ICLFilter_API DynamicConvolutionOp : public ConvolutionOp {
      public:
      /// Constructor
      /**
        @ param poKernel Kernel for the Convolution
      */
      DynamicConvolutionOp (const core::ImgBase* poKernel = 0);

      /// Destructor
      ~DynamicConvolutionOp ();

      /// sets the Kernel for the Convolution
      void setKernel (const core::ImgBase* poKernel);
      private:
      core::Img<icl32f> *poKernelBuf;
    };

  } // namespace filter
}
