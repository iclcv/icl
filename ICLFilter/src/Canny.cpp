#include "Canny.h"
#include "Img.h"
#include "Convolution.h"

namespace icl {

#ifdef WITH_IPP_OPTIMIZATION
  // without ipp non of the function is implemented
  Canny::Canny(icl32f lowThresh, icl32f highThresh):
    m_pucBuffer8u(0),m_poSobelx(0),m_poSobely(0),
    m_fLowThresh(lowThresh), m_fHighThresh(highThresh)
  {
    FUNCTION_LOG("");
  }
  
  Canny::~Canny(){
    FUNCTION_LOG("");
    if(m_pucBuffer8u){
      delete [] m_pucBuffer8u;
    }
    if (m_poSobelx){delete m_poSobelx;}
    if (m_poSobely){delete m_poSobely;}
  }

  void Canny::enshureBufferSize(const Size &s){
    FUNCTION_LOG("");
    int bufferSizeNeeded;
    ippiCannyGetSize(s, &bufferSizeNeeded);
    if(bufferSizeNeeded != m_iBufferSize){
      if(m_pucBuffer8u) delete m_pucBuffer8u;
      m_pucBuffer8u = new icl8u[bufferSizeNeeded];
      m_iBufferSize = bufferSizeNeeded;
    }
  }

void Canny::apply (const Img32f *srcDx, const Img32f *srcDy, Img8u *dst, icl32f lowThresh, icl32f highThresh){
      // {{{ open
  FUNCTION_LOG("");
  ICLASSERT_RETURN(srcDx->getROISize() == srcDy->getROISize());
  ICLASSERT_RETURN(srcDx->getROISize() == dst->getROISize());
  ICLASSERT_RETURN(srcDx->getChannels() == srcDy->getChannels());
  ICLASSERT_RETURN(srcDx->getChannels() == dst->getChannels());
  
  enshureBufferSize(srcDx->getROISize());
  
  for (int c=srcDx->getChannels()-1; c >= 0; --c) {
         ippiCanny_32f8u_C1R (const_cast<Img32f*>(srcDx)->getROIData (c), srcDx->getLineStep(),
                              const_cast<Img32f*>(srcDy)->getROIData (c), srcDy->getLineStep(),
                              dst->getROIData (c), dst->getLineStep(),
                              dst->getROISize(),lowThresh,highThresh,m_pucBuffer8u);
      }
   }
  // }}}

  // {{{ ImgBase* version

  void Canny::apply (const ImgBase *poSrcDx, const ImgBase *poSrcDy, ImgBase **ppoDst, icl32f lowThresh, icl32f highThresh)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrcDx->getDepth() == depth32f);
    ICLASSERT_RETURN( poSrcDx->getDepth() == poSrcDy->getDepth());
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

  void Canny::apply (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    apply(poSrc, ppoDst, m_fLowThresh, m_fHighThresh);
  }
   // }}}

// }}}


#endif
}
