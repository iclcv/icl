#include "Canny.h"
#include "Img.h"
#include "Convolution.h"

namespace icl {


#ifdef WITH_IPP_OPTIMIZATION

void Canny::createBuffer(const Img32f *src){
  int bufSize;
  ippiCannyGetSize(src->getROISize(), &bufSize);
  m_oBuffer8u = new icl8u [bufSize];
  
  
}
void Canny::deleteBuffer(){
  delete m_oBuffer8u;
}
void Canny::apply (const Img32f *srcDx, const Img32f *srcDy, Img8u *dst, icl32f lowThresh, icl32f highThresh){
      // {{{ open
      for (int c=srcDx->getChannels()-1; c >= 0; --c) {
         ippiCanny_32f8u_C1R (srcDx->getROIData (c), srcDx->getLineStep(),
                   srcDy->getROIData (c), srcDy->getLineStep(),
                   dst->getROIData (c), dst->getLineStep(),
                   dst->getROISize(),lowThresh,highThresh,m_oBuffer8u);
      }
   }
  // }}}

  #else
  // {{{ C++ fallback 

   void Canny::apply (const Img32f *srcDx, const Img32f *srcDy, Img8u *dst, icl32f lowThresh, icl32f highThresh){
     #warning "Canny Edge Detector is not implemented without IPP optimization";
    }

  // }}}
  #endif
  // {{{ ImgBase* version

  void Canny::apply (const ImgBase *poSrcDx, const ImgBase *poSrcDy, ImgBase **ppoDst, icl32f lowThresh, icl32f highThresh)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrcDx->getDepth() == poSrcDy->getDepth() == depth32f);
    ICLASSERT_RETURN( poSrcDx->getChannels() == poSrcDy->getChannels());
    if (!prepare (ppoDst, poSrcDx,depth8u)) return;
       apply(poSrcDx->asImg<icl32f>(),poSrcDy->asImg<icl32f>(),(*ppoDst)->asImg<icl8u>(),lowThresh,highThresh);
  }
   // }}}

  
  void Canny::apply (const ImgBase *poSrc, ImgBase **ppoDst, icl32f lowThresh, icl32f highThresh)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f);
    

    ImgBase *sobelx=0; //sobel-x => y-derivation
    ImgBase *sobely=0; //sobel-y => x-derivation
    Convolution* pConv = new Convolution(Convolution::kernelSobelX3x3);
    pConv->setClipToROI (true);
    pConv->apply (poSrc->asImg<icl32f>(), &sobelx);
    pConv = new Convolution(Convolution::kernelSobelY3x3);
    pConv->setClipToROI (true);
    pConv->apply (poSrc->asImg<icl32f>(), &sobely);
    if (!prepare (ppoDst, sobelx,depth8u)) return;
    apply(sobely->asImg<icl32f>(),sobelx->asImg<icl32f>(),(*ppoDst)->asImg<icl8u>(),lowThresh,highThresh);
    //apply(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl8u>(),lowThresh,highThresh);
  }
   // }}}


// }}}

}
