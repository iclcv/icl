#include "LUT.h"

namespace icl{
  
  void LUT::simple(Img8u *src, Img8u *dst, const std::vector<icl8u>& lut){
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    ICLASSERT_RETURN( lut.size() >= 256 );
    
    for(int c=src->getChannels()-1; c >= 0; --c) {
      ImgIterator<icl8u> itSrc = src->getROIIterator(c);
      ImgIterator<icl8u> itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = lut[*itSrc];
      }
    }    
  }

  void LUT::reduceBits(Img8u *src, Img8u *dst, icl8u n){
#ifdef WITH_IPP_OPTIMIZATION
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    ICLASSERT_RETURN( n > 0 );
    for(int c= src->getChannels()-1 ; c >= 0 ; --c){
      ippiReduceBits_8u_C1R(src->getROIData(c),src->getLineStep(),dst->getROIData(c),dst->getLineStep(),
                          src->getROISize(),0, ippDitherNone, n);
    }
#else
    std::vector<icl8u> lut(256),lv;
    float range = 256.0/n;
    for(int i=0;i<n;i++)lv.push_back((int)round(i*range));
    for(int i=0;i<256;i++){
      lut[i]=lv[(int)round((float)i/n)];
    }
    // calculate table
    simple(src,dst,lut);
#endif
  }
  
}
