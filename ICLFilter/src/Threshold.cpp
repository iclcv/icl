#include "Threshold.h"
#include "Macros.h"

namespace icl{

  // {{{ C++ fallback "ThreshFunc"-classes
  
  template <class S, class D> class ThreshFunc{
    // {{{ open

  public:
    ThreshFunc(S s1, S s2=0, S s3=0, S s4=0): 
      s1(s1),s2(s2),s3(s3),s4(s4),
      d1(Cast<S,D>::cast(s1)),d2(Cast<S,D>::cast(s2)),
      d3(Cast<S,D>::cast(s3)),d4(Cast<S,D>::cast(s4)){}
    virtual ~ThreshFunc(){}
    virtual inline D threshold(S val) const {return D(0);}
  protected:
    S s1,s2,s3,s4;
    D d1,d2,d3,d4;
  };

  // }}}
  template <class S, class D> class ThreshFuncLT : public ThreshFunc<S,D>{ 
    // {{{ open

  public:
    ThreshFuncLT(S t):ThreshFunc<S,D>(t){}
    virtual inline D threshold(S val) const { 
      return val< ThreshFunc<S,D>::s1 ? ThreshFunc<S,D>::d1 : Cast<S,D>::cast(val) ; 
    }
  };

  // }}}
  template <class S, class D> struct ThreshFuncGT : public ThreshFunc<S,D>{
    // {{{ open

    ThreshFuncGT(S t):ThreshFunc<S,D>(t){};
    virtual inline D threshold(S val) const{ 
      return val>ThreshFunc<S,D>::s1 ? ThreshFunc<S,D>::d1 : Cast<S,D>::cast(val) ; 
    }
  };

  // }}}
  template <class S, class D> struct ThreshFuncLTGT : public ThreshFunc<S,D>{
    // {{{ open

    ThreshFuncLTGT(S t1,S t2):ThreshFunc<S,D>(t1,t2){};
    virtual inline D threshold(S val) const{ 
      return val<ThreshFunc<S,D>::s1 ? ThreshFunc<S,D>::d1 : val>ThreshFunc<S,D>::s2 ? ThreshFunc<S,D>::d2 : Cast<S,D>::cast(val) ; 
    }
  };

  // }}}
  template <class S, class D> struct ThreshFuncLTVal : public ThreshFunc<S,D>{
    // {{{ open

    ThreshFuncLTVal(S t, S v):ThreshFunc<S,D>(t,v){};
    virtual inline D threshold(S val) const{ 
      return val<ThreshFunc<S,D>::s1 ? ThreshFunc<S,D>::d2 : Cast<S,D>::cast(val) ; 
    }
  };

  // }}}
  template <class S, class D> struct ThreshFuncGTVal : public ThreshFunc<S,D>{
    // {{{ open

    ThreshFuncGTVal(S t, S v):ThreshFunc<S,D>(t,v){};
    virtual inline D threshold(S val) const {
      return val> ThreshFunc<S,D>::s1 ? ThreshFunc<S,D>::d2 : Cast<S,D>::cast(val) ; 
    }
  };

  // }}}
  template <class S, class D> struct ThreshFuncLTGTVal : public ThreshFunc<S,D>{
    // {{{ open

    ThreshFuncLTGTVal(S t1, S v1, S t2, S v2):ThreshFunc<S,D>(t1,v1,t2,v2){};
    virtual inline D threshold(S val)const { 
      return val<ThreshFunc<S,D>::s1 ? ThreshFunc<S,D>::d2 : val>ThreshFunc<S,D>::s3 ? ThreshFunc<S,D>::d4 : Cast<S,D>::cast(val) ; 
    }
  };

  // }}}


  // }}}

  // {{{ arbitrary-type C++ fallback function
  
  template <class S, class D>
  inline void fallbackThreshold(Img<S> *src, Img<D> *dst,const ThreshFunc<S,D> &f){
    // {{{ open
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    for(int c=std::min(src->getChannels(), dst->getChannels()) - 1; c >= 0; --c) {
      ImgIterator<S> itSrc = src->getROIIterator(c);
      ImgIterator<D> itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = f.threshold(*itSrc);
      }
    }
  }

  // }}}
  
  // }}}

#ifdef WITH_IPP_OPTIMIZATION
  // {{{ ippi-function call templates (sit down before open)

  template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T)>
  inline void ippiThresholdCall_1T(Img<T> *src, Img<T> *dst, T t){
    // {{{ open

    for (int c=std::min(src->getChannels(), dst->getChannels()) - 1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(), t);
    }
  }

  // }}}

  template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T, T)>
  inline void ippiThresholdCall_2T(Img<T> *src, Img<T> *dst, T t1, T t2){  
    // {{{ open
    for (int c=std::min(src->getChannels(), dst->getChannels()) - 1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(), t1, t2);
    }
  }

  // }}}
 
 template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T, T, T, T)>
  inline void ippiThresholdCall_4T(Img<T> *src, Img<T> *dst, T t1,T t2, T t3, T t4){
    // {{{ open
    for (int c=std::min(src->getChannels(), dst->getChannels()) - 1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(), t1,t2,t3,t4);
    }
  }

  // }}}

 // }}}
  
  // {{{ "Direct" template specializations without Val postfix

  template <>
  void Threshold::Direct::lt<icl8u>(Img8u *src,Img8u *dst, icl8u t){
    ippiThresholdCall_1T<icl8u, ippiThreshold_LT_8u_C1R> (src, dst, t);
  }
  template <>
  void Threshold::Direct::lt<icl32f>(Img32f *src,Img32f *dst, icl32f t){
    ippiThresholdCall_1T<icl32f, ippiThreshold_LT_32f_C1R> (src, dst, t);
  }
  template <>
  void Threshold::Direct::gt<icl8u>(Img8u *src,Img8u *dst, icl8u t){
    ippiThresholdCall_1T<icl8u, ippiThreshold_GT_8u_C1R> (src, dst, t);
  }
  template <>
  void Threshold::Direct::gt<icl32f>(Img32f *src,Img32f *dst, icl32f t){
    ippiThresholdCall_1T<icl32f, ippiThreshold_GT_32f_C1R> (src, dst, t);
  }
  template <>
  void Threshold::Direct::ltgt<icl8u>(Img8u *src,Img8u *dst, icl8u tMin, icl8u tMax){
    ippiThresholdCall_1T<icl8u, ippiThreshold_LT_8u_C1R> (src, dst, tMin);
    ippiThresholdCall_1T<icl8u, ippiThreshold_GT_8u_C1R> (src, dst, tMax);
  }
  template <>
  void Threshold::Direct::ltgt<icl32f>(Img32f *src,Img32f *dst, icl32f tMin, icl32f tMax){
    ippiThresholdCall_1T<icl32f, ippiThreshold_LT_32f_C1R> (src, dst, tMin);
    ippiThresholdCall_1T<icl32f, ippiThreshold_GT_32f_C1R> (src, dst, tMax);
  }
  // function with val

  // }}}
 
  // {{{ "Direct" template specialization with Val postfix

  template <>
  void Threshold::Direct::ltVal<icl8u>(Img8u *src,Img8u *dst, icl8u t, icl8u val){
    ippiThresholdCall_2T<icl8u, ippiThreshold_LTVal_8u_C1R> (src, dst, t, val);
  }
  template <>
  void Threshold::Direct::ltVal<icl32f>(Img32f *src,Img32f *dst, icl32f t, icl32f val){
    ippiThresholdCall_2T<icl32f, ippiThreshold_LTVal_32f_C1R> (src, dst, t, val);
  }
  template <>
  void Threshold::Direct::gtVal<icl8u>(Img8u *src,Img8u *dst, icl8u t, icl8u val){
    ippiThresholdCall_2T<icl8u, ippiThreshold_GTVal_8u_C1R> (src, dst, t,val);
  }
  template <>
  void Threshold::Direct::gtVal<icl32f>(Img32f *src,Img32f *dst, icl32f t, icl32f val){
    ippiThresholdCall_2T<icl32f, ippiThreshold_GTVal_32f_C1R> (src, dst, t,val);
  }
  template <>
  void Threshold::Direct::ltgtVal<icl8u>(Img8u *src,Img8u *dst, icl8u tMin,icl8u minVal, icl8u tMax,icl8u maxVal){
    ippiThresholdCall_4T<icl8u, ippiThreshold_LTValGTVal_8u_C1R> (src, dst, tMin, minVal, tMax, maxVal);
  }
  template <>
  void Threshold::Direct::ltgtVal<icl32f>(Img32f *src,Img32f *dst, icl32f tMin,icl32f minVal, icl32f tMax, icl32f maxVal){
    ippiThresholdCall_4T<icl32f, ippiThreshold_LTValGTVal_32f_C1R> (src, dst, tMin, minVal, tMax, maxVal);

  }

  // }}}
#else
  // {{{ "Direct" fallback functions without Val postfix

  template <>
  void Threshold::Direct::lt<icl8u>(Img8u *src,Img8u *dst, icl8u t){
    fallbackThreshold<icl8u,icl8u>(src->asImg<icl8u>(), dst->asImg<icl8u>(),ThreshFuncLT<icl8u,icl8u>(t));
  }
  template <>
  void Threshold::Direct::lt<icl32f>(Img32f *src,Img32f *dst, icl32f t){
    fallbackThreshold(src->asImg<icl32f>(), dst->asImg<icl32f>(),ThreshFuncLT<icl32f,icl32f>(t));
  }
  template <>
  void Threshold::Direct::gt<icl8u>(Img8u *src,Img8u *dst, icl8u t){
    fallbackThreshold<icl8u,icl8u>(src->asImg<icl8u>(), dst->asImg<icl8u>(),ThreshFuncGT<icl8u,icl8u>(t));
  }
  template <>
  void Threshold::Direct::gt<icl32f>(Img32f *src,Img32f *dst, icl32f t){
    fallbackThreshold(src->asImg<icl32f>(), dst->asImg<icl32f>(),ThreshFuncLT<icl32f,icl32f>(t));
  }
  template <>
  void Threshold::Direct::ltgt<icl8u>(Img8u *src,Img8u *dst, icl8u tMin, icl8u tMax){
    fallbackThreshold<icl8u,icl8u>(src->asImg<icl8u>(), dst->asImg<icl8u>(),ThreshFuncLTGT<icl8u,icl8u>(tMin,tMax));
  }
  template <>
  void Threshold::Direct::ltgt<icl32f>(Img32f *src,Img32f *dst, icl32f tMin, icl32f tMax){
    fallbackThreshold<icl32f,icl32f>(src->asImg<icl32f>(), dst->asImg<icl32f>(),ThreshFuncLTGT<icl32f,icl32f>(tMin,tMax));
  }
  
  // }}}
  
  // {{{ "Direct" fallback functions with Val postfix

  template <>
  void Threshold::Direct::ltVal<icl8u>(Img8u *src,Img8u *dst, icl8u t, icl8u val){
    fallbackThreshold<icl8u,icl8u>(src->asImg<icl8u>(), dst->asImg<icl8u>(),ThreshFuncLTVal<icl8u,icl8u>(t,val));
  }
  template <>
  void Threshold::Direct::ltVal<icl32f>(Img32f *src,Img32f *dst, icl32f t, icl32f val){
    fallbackThreshold<icl32f,icl32f>(src->asImg<icl32f>(), dst->asImg<icl32f>(),ThreshFuncLTVal<icl32f,icl32f>(t,val));
  }
  template <>
  void Threshold::Direct::gtVal<icl8u>(Img8u *src,Img8u *dst, icl8u t, icl8u val){
    fallbackThreshold<icl8u,icl8u>(src->asImg<icl8u>(), dst->asImg<icl8u>(),ThreshFuncGTVal<icl8u,icl8u>(t,val));
  }
  template <>
  void Threshold::Direct::gtVal<icl32f>(Img32f *src,Img32f *dst, icl32f t, icl32f val){
    fallbackThreshold<icl32f,icl32f>(src->asImg<icl32f>(), dst->asImg<icl32f>(),ThreshFuncGTVal<icl32f,icl32f>(t,val));
  }
  template <>
  void Threshold::Direct::ltgtVal<icl8u>(Img8u *src,Img8u *dst, icl8u tMin,icl8u minVal, icl8u tMax,icl8u maxVal){
    fallbackThreshold<icl8u,icl8u>(src->asImg<icl8u>(), dst->asImg<icl8u>(),ThreshFuncLTGTVal<icl8u,icl8u>(tMin,minVal,tMax,maxVal));
  }
  template <>
  void Threshold::Direct::ltgtVal<icl32f>(Img32f *src,Img32f *dst, icl32f tMin,icl32f minVal, icl32f tMax, icl32f maxVal){
    fallbackThreshold<icl32f,icl32f>(src->asImg<icl32f>(), dst->asImg<icl32f>(),ThreshFuncLTGTVal<icl32f,icl32f>(tMin,minVal,tMax,maxVal));
  }
  // }}}
#endif

  // {{{ "Direct "explicit template instantination

  template void Threshold::Direct::lt<icl8u>(Img8u*,Img8u*,icl8u);
  template void Threshold::Direct::gt<icl8u>(Img8u*,Img8u*,icl8u);
  template void Threshold::Direct::ltgt<icl8u>(Img8u*,Img8u*,icl8u,icl8u);
  template void Threshold::Direct::ltVal<icl8u>(Img8u*,Img8u*,icl8u,icl8u);
  template void Threshold::Direct::gtVal<icl8u>(Img8u*,Img8u*,icl8u,icl8u);
  template void Threshold::Direct::ltgtVal<icl8u>(Img8u*,Img8u*,icl8u,icl8u,icl8u,icl8u);

  template void Threshold::Direct::lt<icl32f>(Img32f*,Img32f*,icl32f);
  template void Threshold::Direct::gt<icl32f>(Img32f*,Img32f*,icl32f);
  template void Threshold::Direct::ltgt<icl32f>(Img32f*,Img32f*,icl32f,icl32f);
  template void Threshold::Direct::ltVal<icl32f>(Img32f*,Img32f*,icl32f,icl32f);
  template void Threshold::Direct::gtVal<icl32f>(Img32f*,Img32f*,icl32f,icl32f);
  template void Threshold::Direct::ltgtVal<icl32f>(Img32f*,Img32f*,icl32f,icl32f,icl32f,icl32f);

  // }}}

  // {{{ non-template functions

  void Threshold::lt(ImgI *src, ImgI *dst, icl32f t){
    // {{{ open

    icl8u t8u = Cast<icl32f,icl8u>::cast(t);
    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        Direct::lt(src->asImg<icl8u>(), dst->asImg<icl8u>(),t8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshFuncLT<icl8u,icl32f>(t8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshFuncLT<icl32f,icl8u>(t));
        break;
      case 11: // 32f->32f
        Direct::lt(src->asImg<icl32f>(), dst->asImg<icl32f>(),t);
        break;
      default: return;
    }
  }

  // }}}
  
  void Threshold::gt(ImgI *src, ImgI *dst, icl32f t){
    // {{{ open

    icl8u t8u = Cast<icl32f,icl8u>::cast(t);
    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        Direct::gt(src->asImg<icl8u>(), dst->asImg<icl8u>(),t8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshFuncGT<icl8u,icl32f>(t8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshFuncGT<icl32f,icl8u>(t));
        break;
      case 11: // 32f->32f
        Direct::gt(src->asImg<icl32f>(), dst->asImg<icl32f>(),t);
        break;
      default: return;
    }
  }

  // }}}
  
  void Threshold::ltgt(ImgI *src, ImgI *dst, icl32f tMin, icl32f tMax){
    // {{{ open

    icl8u tMin8u = Cast<icl32f,icl8u>::cast(tMin);
    icl8u tMax8u = Cast<icl32f,icl8u>::cast(tMax);
    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        Direct::ltgt(src->asImg<icl8u>(), dst->asImg<icl8u>(),tMin8u,tMax8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshFuncLTGT<icl8u,icl32f>(tMin8u,tMax8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshFuncLTGT<icl32f,icl8u>(tMin,tMax));
        break;
      case 11: // 32f->32f
        Direct::ltgt(src->asImg<icl32f>(), dst->asImg<icl32f>(),tMin,tMax);
        break;
      default: return;
    }
  }

  // }}}
   
  void Threshold::ltVal(ImgI *src, ImgI *dst, icl32f t, icl32f val){
    // {{{ open

    icl8u t8u = Cast<icl32f,icl8u>::cast(t);
    icl8u val8u = Cast<icl32f,icl8u>::cast(val);
    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        Direct::ltVal(src->asImg<icl8u>(), dst->asImg<icl8u>(),t8u,val8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshFuncLTVal<icl8u,icl32f>(t8u,val8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshFuncLTVal<icl32f,icl8u>(t,val));
        break;
      case 11: // 32f->32f
        Direct::ltVal(src->asImg<icl32f>(), dst->asImg<icl32f>(),t,val);
        break;
      default: return;
    }
  }

  // }}}
  
  void Threshold::gtVal(ImgI *src, ImgI *dst, icl32f t, icl32f val){
    // {{{ open

    icl8u t8u = Cast<icl32f,icl8u>::cast(t);
    icl8u val8u = Cast<icl32f,icl8u>::cast(val);
    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        Direct::gtVal(src->asImg<icl8u>(), dst->asImg<icl8u>(),t8u,val8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshFuncGTVal<icl8u,icl32f>(t8u,val8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshFuncGTVal<icl32f,icl8u>(t,val));
        break;
      case 11: // 32f->32f
        Direct::gtVal(src->asImg<icl32f>(), dst->asImg<icl32f>(),t,val);
        break;
      default: return;
    }
  }

  // }}}
 
  void Threshold::ltgtVal(ImgI *src, ImgI *dst, icl32f tMin, icl32f minVal, icl32f tMax, icl32f maxVal){
    // {{{ open

    icl8u tMin8u = Cast<icl32f,icl8u>::cast(tMin);
    icl8u minVal8u = Cast<icl32f,icl8u>::cast(minVal);
    icl8u tMax8u = Cast<icl32f,icl8u>::cast(tMax);
    icl8u maxVal8u = Cast<icl32f,icl8u>::cast(maxVal);


    switch(src->getDepth()+10*dst->getDepth()){
      case 0 : // 8u->8u
        Direct::ltgtVal(src->asImg<icl8u>(), dst->asImg<icl8u>(),tMin8u,minVal8u,tMax8u, maxVal8u);
        break;
      case 10: // 8u->32f
        fallbackThreshold<icl8u,icl32f>(src->asImg<icl8u>(), dst->asImg<icl32f>(),ThreshFuncLTGTVal<icl8u,icl32f>(tMin8u,minVal8u,tMax8u, maxVal8u));
        break;
      case 1: // 32f->8u
        fallbackThreshold<icl32f,icl8u>(src->asImg<icl32f>(), dst->asImg<icl8u>(),ThreshFuncLTGTVal<icl32f,icl8u>(tMin,minVal,tMax,maxVal));
        break;
      case 11: // 32f->32f
        Direct::ltgtVal(src->asImg<icl32f>(), dst->asImg<icl32f>(),tMin,minVal,tMax,maxVal);
        break;
      default: return;
    }
  }

  // }}}
  
// }}}
  
  
} // namespace icl



