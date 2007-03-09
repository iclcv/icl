#include <iclDynamicConvolutionOp.h>
#include <iclImg.h>

namespace icl{

  DynamicConvolutionOp::DynamicConvolutionOp (const ImgBase* poKernel) : 
     ConvolutionOp ()
  {
     poKernelBuf = new icl::Img<icl32f>(Size(3,3), 1);
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
     ConvolutionOp::setKernel (poKernelBuf->getData(0), poKernelBuf->getSize(), false);
  }

 }
