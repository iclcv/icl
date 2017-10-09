/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/DynamicConvolutionOp.h         **
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

