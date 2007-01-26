#include <Wiener.h>
#include <Img.h>

namespace icl {

   // Constructor
   Wiener::Wiener (const Size& maskSize) {
#ifdef WITH_IPP_OPTIMIZATION
      if (maskSize.width <= 0 || maskSize.height<=0) {
         ERROR_LOG("illegal width/height: " << maskSize.width << "/" << maskSize.height);
         setMask (Size(3,3));
      } else setMask (maskSize);
#else
      throw ("Wiener Filter only implemented with IPP usage.");
#endif
   }

#ifdef WITH_IPP_OPTIMIZATION
// {{{ IPP version

   template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiSize, IppiPoint, float[], icl8u*)>
   void Wiener::ippiWienerCall (const ImgBase *poSrc, ImgBase *poDst, float fNoise) {
      Img<T> *src = (Img<T>*) poSrc;
      Img<T> *dst = (Img<T>*) poDst;

      int iBufferSize;
      ippiFilterWienerGetBufferSize(dst->getROISize(), oMaskSize, 1, &iBufferSize);
      vBuffer.reserve (iBufferSize);

      for(int c=src->getChannels()-1; c>=0; --c) {
         ippiFunc(src->getROIData (c, this->oROIoffset), src->getLineStep(),
                  dst->getROIData (c), dst->getLineStep(),
                  dst->getROISize(), oMaskSize, oAnchor, &fNoise, &vBuffer[0]);
      };
   }

// }}}
#endif

   void (Wiener::*Wiener::aMethods[depthLast+1])(const ImgBase *poSrc, ImgBase *poDst, float) = {
#ifdef WITH_IPP_OPTIMIZATION 
      &Wiener::ippiWienerCall<icl8u,ippiFilterWiener_8u_C1R>,   // 8u
      &Wiener::ippiWienerCall<icl16s,ippiFilterWiener_16s_C1R>, // 16s
      0, // 32s
      &Wiener::ippiWienerCall<icl32f,ippiFilterWiener_32f_C1R>, // 32f
      0 // 64f
#else
      0
#endif     
   };

   // {{{ Wiener::apply (ImgBase *poSrc, ImgBase **ppoDst, float fNoise)

   void Wiener::apply (const ImgBase *poSrc, ImgBase **ppoDst, float fNoise) {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     void (Wiener::*pMethod)(const ImgBase*, ImgBase*, float) 
        = this->aMethods[poSrc->getDepth()];
     if (!pMethod) ICL_INVALID_FORMAT;
     (this->*pMethod)(poSrc, *ppoDst, fNoise);
   }
   
  // }}}

}
