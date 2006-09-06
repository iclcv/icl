#include "Threshold.h"
#include "Macros.h"

namespace icl{

  // {{{ C++ fallback ThreshOp classes
  
  template <class S, class D> class ThreshOpLT {
    // {{{ open

  public:
    ThreshOpLT (S t) : threshold(t) {}
    inline D operator()(S val) const { 
      return Cast<S,D>::cast(val < threshold ? threshold : val);
    }
  private:
     S threshold;
  };

  // }}}
  template <class S, class D> class ThreshOpGT {
    // {{{ open

  public:
    ThreshOpGT (S t) : threshold(t) {}
    inline D operator()(S val) const { 
      return Cast<S,D>::cast(val > threshold ? threshold : val);
    }
  private:
     S threshold;
  };

  // }}}
  template <class S, class D> class ThreshOpLTGT {
    // {{{ open
  public:
    ThreshOpLTGT(S tLow, S tUp) : tLow(tLow), tUp(tUp) {}
    inline D operator()(S val) const { 
       if (val < tLow) return Cast<S,D>::cast(tLow);
       if (val > tUp)  return Cast<S,D>::cast(tUp);
       return Cast<S,D>::cast(val);
    }
  private:
    S tLow, tUp;
  };

  // }}}

  template <class S, class D> class ThreshOpLTVal {
    // {{{ open

  public:
    ThreshOpLTVal (S t, D v) : threshold(t), value(v) {}
    inline D operator()(S val) const { 
      if (val < threshold) return value;
      return Cast<S,D>::cast(val);
    }
  private:
     S threshold;
     D value;
  };

  // }}}
  template <class S, class D> class ThreshOpGTVal {
    // {{{ open

  public:
    ThreshOpGTVal (S t, D v) : threshold(t), value(v) {}
    inline D operator()(S val) const { 
      if (val > threshold) return value;
      return Cast<S,D>::cast(val);
    }
  private:
     S threshold;
     D value;
  };

  // }}}
  template <class S, class D> class ThreshOpLTGTVal {
    // {{{ open
  public:
    ThreshOpLTGTVal(S tLow, D vLow, S tUp, D vUp) : 
       tLow(tLow), tUp(tUp), vLow(vLow), vUp(vUp) {}
    inline D operator()(S val) const { 
       if (val < tLow) return vLow;
       if (val > tUp)  return vUp;
       return Cast<S,D>::cast(val);
    }
  private:
    S tLow, tUp;
    D vLow, vUp;
  };

  // }}}

  // }}}

  // {{{ C++ fallback threshold function for all threshold operations
  
  template <class S, class D, class ThresholdOp>
  void fallbackThreshold(Img<S> *src, Img<D> *dst, const ThresholdOp &threshold) {
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    for(int c=std::min(src->getChannels(), dst->getChannels()) - 1; c >= 0; --c) {
      ImgIterator<S> itSrc = src->getROIIterator(c);
      ImgIterator<D> itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = threshold(*itSrc);
      }
    }
  }

  // }}}

#ifdef WITH_IPP_OPTIMIZATION
  // {{{ ippi-function call templates

  template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T)>
  inline void ippiThresholdCall_1T(const Img<T> *src, Img<T> *dst, T t){
    // {{{ open

    for (int c=std::min(src->getChannels(), dst->getChannels()) - 1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(), t);
    }
  }

  // }}}

  template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T, T)>
  inline void ippiThresholdCall_2T(const Img<T> *src, Img<T> *dst, T t1, T t2){  
    // {{{ open
    for (int c=std::min(src->getChannels(), dst->getChannels()) - 1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(), t1, t2);
    }
  }

  // }}}
 
 template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T, T, T, T)>
  inline void ippiThresholdCall_4T(const Img<T> *src, Img<T> *dst, T t1,T t2, T t3, T t4){
    // {{{ open
    for (int c=std::min(src->getChannels(), dst->getChannels()) - 1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(), t1,t2,t3,t4);
    }
  }

  // }}}

 // }}}
  
  // {{{ template specializations without Val postfix

  template <>
  void Threshold::lt<icl8u>(const Img8u *src,Img8u *dst, icl8u t){
    ippiThresholdCall_1T<icl8u, ippiThreshold_LT_8u_C1R> (src, dst, t);
  }
  template <>
  void Threshold::lt<icl32f>(const Img32f *src,Img32f *dst, icl32f t){
    ippiThresholdCall_1T<icl32f, ippiThreshold_LT_32f_C1R> (src, dst, t);
  }
  template <>
  void Threshold::gt<icl8u>(const Img8u *src,Img8u *dst, icl8u t){
    ippiThresholdCall_1T<icl8u, ippiThreshold_GT_8u_C1R> (src, dst, t);
  }
  template <>
  void Threshold::gt<icl32f>(const Img32f *src,Img32f *dst, icl32f t){
    ippiThresholdCall_1T<icl32f, ippiThreshold_GT_32f_C1R> (src, dst, t);
  }
  template <>
  void Threshold::ltgt<icl8u>(const Img8u *src,Img8u *dst, icl8u tMin, icl8u tMax){
    ippiThresholdCall_1T<icl8u, ippiThreshold_LT_8u_C1R> (src, dst, tMin);
    ippiThresholdCall_1T<icl8u, ippiThreshold_GT_8u_C1R> (src, dst, tMax);
  }
  template <>
  void Threshold::ltgt<icl32f>(const Img32f *src,Img32f *dst, icl32f tMin, icl32f tMax){
    ippiThresholdCall_1T<icl32f, ippiThreshold_LT_32f_C1R> (src, dst, tMin);
    ippiThresholdCall_1T<icl32f, ippiThreshold_GT_32f_C1R> (src, dst, tMax);
  }

  // }}}
 
  // {{{ template specializations with Val postfix

  template <>
  void Threshold::ltVal<icl8u>(const Img8u *src,Img8u *dst, icl8u t, icl8u val){
    ippiThresholdCall_2T<icl8u, ippiThreshold_LTVal_8u_C1R> (src, dst, t, val);
  }
  template <>
  void Threshold::ltVal<icl32f>(const Img32f *src,Img32f *dst, icl32f t, icl32f val){
    ippiThresholdCall_2T<icl32f, ippiThreshold_LTVal_32f_C1R> (src, dst, t, val);
  }
  template <>
  void Threshold::gtVal<icl8u>(const Img8u *src,Img8u *dst, icl8u t, icl8u val){
    ippiThresholdCall_2T<icl8u, ippiThreshold_GTVal_8u_C1R> (src, dst, t,val);
  }
  template <>
  void Threshold::gtVal<icl32f>(const Img32f *src,Img32f *dst, icl32f t, icl32f val){
    ippiThresholdCall_2T<icl32f, ippiThreshold_GTVal_32f_C1R> (src, dst, t,val);
  }
  template <>
  void Threshold::ltgtVal<icl8u>(const Img8u *src,Img8u *dst, icl8u tMin,icl8u minVal, icl8u tMax,icl8u maxVal){
    ippiThresholdCall_4T<icl8u, ippiThreshold_LTValGTVal_8u_C1R> (src, dst, tMin, minVal, tMax, maxVal);
  }
  template <>
  void Threshold::ltgtVal<icl32f>(const Img32f *src,Img32f *dst, icl32f tMin,icl32f minVal, icl32f tMax, icl32f maxVal){
    ippiThresholdCall_4T<icl32f, ippiThreshold_LTValGTVal_32f_C1R> (src, dst, tMin, minVal, tMax, maxVal);

  }

  // }}}
#else
  // {{{ template specializations without Val postfix (fallback)

  template <>
  void Threshold::lt<icl8u>(Img8u *src,Img8u *dst, icl8u t){
    fallbackThreshold (src, dst, ThreshOpLT<icl8u,icl8u>(t));
  }
  template <>
  void Threshold::lt<icl32f>(Img32f *src,Img32f *dst, icl32f t){
    fallbackThreshold (src, dst, ThreshOpLT<icl32f,icl32f>(t));
  }
  template <>
  void Threshold::gt<icl8u>(Img8u *src,Img8u *dst, icl8u t){
    fallbackThreshold (src, dst, ThreshOpGT<icl8u,icl8u>(t));
  }
  template <>
  void Threshold::gt<icl32f>(Img32f *src,Img32f *dst, icl32f t){
    fallbackThreshold (src, dst, ThreshOpLT<icl32f,icl32f>(t));
  }
  template <>
  void Threshold::ltgt<icl8u>(Img8u *src,Img8u *dst, icl8u tMin, icl8u tMax){
    fallbackThreshold (src, dst, ThreshOpLTGT<icl8u,icl8u>(tMin,tMax));
  }
  template <>
  void Threshold::ltgt<icl32f>(Img32f *src,Img32f *dst, icl32f tMin, icl32f tMax){
    fallbackThreshold (src, dst, ThreshOpLTGT<icl32f,icl32f>(tMin,tMax));
  }

  // }}}
  
  // {{{ template specializations with Val postfix (fallback)

  template <>
  void Threshold::ltVal<icl8u>(Img8u *src,Img8u *dst, icl8u t, icl8u val){
    fallbackThreshold (src, dst, ThreshOpLTVal<icl8u,icl8u>(t,val));
  }
  template <>
  void Threshold::ltVal<icl32f>(Img32f *src,Img32f *dst, icl32f t, icl32f val){
    fallbackThreshold (src, dst, ThreshOpLTVal<icl32f,icl32f>(t,val));
  }
  template <>
  void Threshold::gtVal<icl8u>(Img8u *src,Img8u *dst, icl8u t, icl8u val){
    fallbackThreshold (src, dst, ThreshOpGTVal<icl8u,icl8u>(t,val));
  }
  template <>
  void Threshold::gtVal<icl32f>(Img32f *src,Img32f *dst, icl32f t, icl32f val){
    fallbackThreshold (src, dst, ThreshOpGTVal<icl32f,icl32f>(t,val));
  }
  template <>
  void Threshold::ltgtVal<icl8u>(Img8u *src,Img8u *dst, icl8u tMin,icl8u minVal, icl8u tMax,icl8u maxVal){
    fallbackThreshold (src, dst, ThreshOpLTGTVal<icl8u,icl8u>(tMin,minVal,tMax,maxVal));
  }
  template <>
  void Threshold::ltgtVal<icl32f>(Img32f *src,Img32f *dst, icl32f tMin,icl32f minVal, icl32f tMax, icl32f maxVal){
    fallbackThreshold (src, dst, ThreshOpLTGTVal<icl32f,icl32f>(tMin,minVal,tMax,maxVal));
  }
  // }}}
#endif

  // {{{ ImgI* versions

  void Threshold::lt(const ImgI *src, ImgI *dst, icl32f t){
    // {{{ open

    icl8u t8u = Cast<icl32f,icl8u>::cast(t);
    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        lt(src->asImg<icl8u>(), dst->asImg<icl8u>(),t8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshOpLT<icl8u,icl32f>(t8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshOpLT<icl32f,icl8u>(t));
        break;
      case 11: // 32f->32f
        lt(src->asImg<icl32f>(), dst->asImg<icl32f>(),t);
        break;
      default: return;
    }
  }

  // }}}
  
  void Threshold::gt(const ImgI *src, ImgI *dst, icl32f t){
    // {{{ open

    icl8u t8u = Cast<icl32f,icl8u>::cast(t);
    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        gt(src->asImg<icl8u>(), dst->asImg<icl8u>(),t8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshOpGT<icl8u,icl32f>(t8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshOpGT<icl32f,icl8u>(t));
        break;
      case 11: // 32f->32f
        gt(src->asImg<icl32f>(), dst->asImg<icl32f>(),t);
        break;
      default: return;
    }
  }

  // }}}
  
  void Threshold::ltgt(const ImgI *src, ImgI *dst, icl32f tMin, icl32f tMax){
    // {{{ open

    icl8u tMin8u = Cast<icl32f,icl8u>::cast(tMin);
    icl8u tMax8u = Cast<icl32f,icl8u>::cast(tMax);
    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        ltgt(src->asImg<icl8u>(), dst->asImg<icl8u>(),tMin8u,tMax8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshOpLTGT<icl8u,icl32f>(tMin8u,tMax8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshOpLTGT<icl32f,icl8u>(tMin,tMax));
        break;
      case 11: // 32f->32f
        ltgt(src->asImg<icl32f>(), dst->asImg<icl32f>(),tMin,tMax);
        break;
      default: return;
    }
  }

  // }}}
   
  void Threshold::ltVal(const ImgI *src, ImgI *dst, icl32f t, icl32f val){
    // {{{ open

    icl8u t8u = Cast<icl32f,icl8u>::cast(t);
    icl8u val8u = Cast<icl32f,icl8u>::cast(val);
    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        ltVal(src->asImg<icl8u>(), dst->asImg<icl8u>(),t8u,val8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshOpLTVal<icl8u,icl32f>(t8u,val8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshOpLTVal<icl32f,icl8u>(t,val8u));
        break;
      case 11: // 32f->32f
        ltVal(src->asImg<icl32f>(), dst->asImg<icl32f>(),t,val);
        break;
      default: return;
    }
  }

  // }}}
  
  void Threshold::gtVal(const ImgI *src, ImgI *dst, icl32f t, icl32f val){
    // {{{ open

    icl8u t8u = Cast<icl32f,icl8u>::cast(t);
    icl8u val8u = Cast<icl32f,icl8u>::cast(val);
    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        gtVal(src->asImg<icl8u>(), dst->asImg<icl8u>(),t8u,val8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshOpGTVal<icl8u,icl32f>(t8u,val8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshOpGTVal<icl32f,icl8u>(t,val8u));
        break;
      case 11: // 32f->32f
        gtVal(src->asImg<icl32f>(), dst->asImg<icl32f>(),t,val);
        break;
      default: return;
    }
  }

  // }}}
 
  void Threshold::ltgtVal(const ImgI *src, ImgI *dst, icl32f tMin, icl32f minVal, icl32f tMax, icl32f maxVal){
    // {{{ open

    icl8u tMin8u = Cast<icl32f,icl8u>::cast(tMin);
    icl8u minVal8u = Cast<icl32f,icl8u>::cast(minVal);
    icl8u tMax8u = Cast<icl32f,icl8u>::cast(tMax);
    icl8u maxVal8u = Cast<icl32f,icl8u>::cast(maxVal);


    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        ltgtVal(src->asImg<icl8u>(), dst->asImg<icl8u>(),tMin8u,minVal8u,tMax8u, maxVal8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshOpLTGTVal<icl8u,icl32f>(tMin8u,minVal8u,tMax8u, maxVal8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshOpLTGTVal<icl32f,icl8u>(tMin,minVal8u,tMax,maxVal8u));
        break;
      case 11: // 32f->32f
        ltgtVal(src->asImg<icl32f>(), dst->asImg<icl32f>(),tMin,minVal,tMax,maxVal);
        break;
      default: return;
    }
  }

  // }}}
  
// }}}
  
  
} // namespace icl
