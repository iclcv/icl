#ifndef DYNAMIC_CONVOLUTION_OP_H
#define DYNAMIC_CONVOLUTION_OP_H

#include <iclConvolutionOp.h>

namespace icl{
  /// Convolution using the ROI of an ICL image as its kernel
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
