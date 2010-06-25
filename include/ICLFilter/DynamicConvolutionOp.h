/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/DynamicConvolutionOp.h               **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef DYNAMIC_CONVOLUTION_OP_H
#define DYNAMIC_CONVOLUTION_OP_H

#include <ICLFilter/ConvolutionOp.h>

namespace icl{
  /// Convolution using the ROI of an ICL image as its kernel \ingroup UNARY \ingroup NBH
  /** Sometimes it is useful to use the ROI of an ICL image directly as the
      convolution kernel, e.g. for template matching. Because the ROI may be
      smaller than the image itself, the DynamicConvolution class maintains
      an internal buffer poKernelBuf of this ROI only. Its first channel is
      directly set as the (unbuffered) kernel data of the underlying Convolution
      class.
  */
  class DynamicConvolutionOp : public ConvolutionOp {
    public:
    /// Constructor
    /**
      @ param poKernel Kernel for the Convolution
    */
    DynamicConvolutionOp (const ImgBase* poKernel = 0);
    
    /// Destructor
    ~DynamicConvolutionOp ();
    
    /// sets the Kernel for the Convolution
    void setKernel (const ImgBase* poKernel);
    private:
    icl::Img<icl::icl32f> *poKernelBuf;
  };
  
}

#endif
