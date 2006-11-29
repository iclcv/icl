#include "Canny.h"
#include "Img.h"
#include "Convolution.h"

namespace icl {


#ifdef WITH_IPP_OPTIMIZATION

Canny::Canny(const Img32f *src){
  int bufSize;
  ippiCannyGetSize(src->getROISize(), &bufSize);
  m_oBuffer8u = new icl8u[bufSize];
    m_poSobelx=0; //sobel-x => y-derivation
    m_poSobely=0; //sobel-y => x-derivation
}


Canny::~Canny(){
  FUNCTION_LOG("");
  delete [] m_oBuffer8u;
  if (m_poSobelx){delete m_poSobelx;}
  if (m_poSobely){delete m_poSobely;}
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

   
  Canny::Canny(const Img32f *src){
     #warning "Canny Edge Detector is not implemented without IPP optimization";
  }
Canny::~Canny(){}
   
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
    Convolution oConvX(Convolution::kernelSobelX3x3);
    oConvX.setClipToROI (true);
    oConvX.apply (poSrc->asImg<icl32f>(), &m_poSobelx);
    Convolution oConvY(Convolution::kernelSobelY3x3);
    oConvY.setClipToROI (true);
    oConvY.apply (poSrc->asImg<icl32f>(), &m_poSobely);
    if (!prepare (ppoDst, m_poSobelx,depth8u)) return;
    apply(m_poSobely->asImg<icl32f>(),m_poSobelx->asImg<icl32f>(),(*ppoDst)->asImg<icl8u>(),lowThresh,highThresh);
    //apply(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl8u>(),lowThresh,highThresh);
  }
   // }}}


// }}}

}
