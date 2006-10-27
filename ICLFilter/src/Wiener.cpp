#include "Wiener.h"
#include "Img.h"

namespace icl {

  // {{{ Constructor / Destructor

  Wiener::Wiener (const Size& maskSize) {
    if (maskSize.width <= 0 || maskSize.height<=0) {
      ERROR_LOG("illegal width/height: " << maskSize.width << "/" << maskSize.height);
      setMask (Size(3,3));
    } else setMask (maskSize);
  }
  // }}}
  void Wiener::setMask (Size maskSize) {
    // make maskSize odd:
    maskSize.width  = (maskSize.width/2)*2 + 1;
    maskSize.height = (maskSize.height/2)*2 + 1;
    FilterMask::setMask (maskSize);
  }

#ifdef WITH_IPP_OPTIMIZATION
  template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, IppiSize, IppiPoint, icl32f, icl8u*)>
  void Wiener::ippiWienerCall (const Img<T> *src, Img<T> *dst,icl32f* noise) {
    int pBufferSize;
    ippiFilterWienerGetBufferSize(dst->getROISize(), oMaskSize, 1, &pBufferSize);
    m_oBuf = new icl8u[pBufferSize];
    for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset),
                 src->getLineStep(),
                 dst->getROIData (c),
                 dst->getLineStep(),
                 dst->getROISize(), oMaskSize, oAnchor,
                 noise,
                 m_oBuf);
    };
    delete[] m_oBuf;
  }

  void Wiener::FilterWiener(const Img8u *src, Img8u *dst, icl32f *noise){
    ippiWienerCall<icl8u,ippiFilterWiener_8u_C1R> (src,dst,noise);
  };
  void Wiener::FilterWiener(const Img32f *src, Img32f *dst, icl32f *noise){
    ippiWienerCall<icl32f,ippiFilterWiener_32f_C1R> (src,dst,noise);
  };
  // {{{ ImgI* version

   void Wiener::FilterWiener (const ImgI *poSrc, ImgI **ppoDst, icl32f *noise)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       FilterWiener(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),noise);
     else
       FilterWiener(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),noise);
   }
   // }}}

  // }}}
#endif

}
