#include <DynamicConvolution.h>
#include <Img.h>

namespace icl{

  DynamicConvolution::DynamicConvolution (const ImgBase* poKernel) : 
     Convolution ()
  {
     poKernelBuf = new icl::Img<icl32f>(Size(3,3), 1);
     if (poKernel) setKernel (poKernel);
  }
  
  DynamicConvolution::~DynamicConvolution () {
     delete poKernelBuf;
  }

  void DynamicConvolution::setKernel (const ImgBase* poKernel) {
     ICLASSERT_RETURN(poKernel->getChannels() == 1);

     // resize kernel buffer if necessary
     if (poKernel->getROISize() != poKernelBuf->getSize())
        poKernelBuf->setSize (poKernel->getROISize());

     // copy data from poKernel's ROI to poKernelBuf
     poKernel->deepCopyROI (poKernelBuf);

     // set Convolution kernel from float data
     Convolution::setKernel (poKernelBuf->getData(0), poKernelBuf->getSize(), false);
  }

 }
