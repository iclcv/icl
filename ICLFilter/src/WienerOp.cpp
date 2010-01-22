#include <ICLFilter/WienerOp.h>
#include <ICLCore/Img.h>

namespace icl {

  
#ifdef HAVE_IPP
  
  namespace{
    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiSize, IppiPoint, float[], icl8u*)>
    void ippiWienerCall (const Img<T> *src, 
                         Img<T> *dst,
                         const Size &oMaskSize, 
                         const Point &oAnchor,
                         const Point &roiOffset,
                         std::vector<icl8u> &vBuffer, 
                         icl32f fNoise) {
      // {{{ open
      int iBufferSize;
      ippiFilterWienerGetBufferSize(dst->getROISize(), oMaskSize, 1, &iBufferSize);
      vBuffer.reserve (iBufferSize);
      
      for(int c=src->getChannels()-1; c>=0; --c) {
        ippiFunc(src->getROIData (c, roiOffset), src->getLineStep(),
                 dst->getROIData (c), dst->getLineStep(),
                 dst->getROISize(), oMaskSize, oAnchor, &fNoise, &vBuffer[0]);
      };
    }
  
// }}}
  } // end of anonymous namespace   


  void WienerOp::apply (const ImgBase *poSrc, ImgBase **ppoDst) {
    // {{{ open
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    
    switch(poSrc->getDepth()){
#define ARGLIST(D) poSrc->asImg<icl##D>(),(*ppoDst)->asImg<icl##D>(),getMaskSize(),getAnchor(),getROIOffset(),m_vecBuffer,m_fNoise
      case depth8u: ippiWienerCall<icl8u,ippiFilterWiener_8u_C1R>(ARGLIST(8u)); break;
      case depth16s: ippiWienerCall<icl16s,ippiFilterWiener_16s_C1R>(ARGLIST(16s)); break;
      case depth32f: ippiWienerCall<icl32f,ippiFilterWiener_32f_C1R>(ARGLIST(32f)); break;
      default: ICL_INVALID_DEPTH;
#undef ARGLIST
    }
  }
  
  // }}}


#endif     
}
