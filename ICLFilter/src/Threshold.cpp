#include "Threshold.h"
#include "Macros.h"

namespace icl {

#ifdef WITH_IPP_OPTIMIZATION
   // {{{ ippi-function call templates

   template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T)>
   inline void ippiThresholdCall_1T(const Img<T> *src, Img<T> *dst, T t){
      // {{{ open

      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

      for (int c=src->getChannels()-1; c >= 0; --c) {
         ippiFunc (src->getROIData (c), src->getLineStep(),
                   dst->getROIData (c), dst->getLineStep(),
                   dst->getROISize(), t);
      }
   }

   // }}}

   template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T, T)>
   inline void ippiThresholdCall_2T(const Img<T> *src, Img<T> *dst, T t1, T t2){  
      // {{{ open

      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

      for (int c=src->getChannels()-1; c >= 0; --c) {
         ippiFunc (src->getROIData (c), src->getLineStep(),
                   dst->getROIData (c), dst->getLineStep(),
                   dst->getROISize(), t1, t2);
      }
   }

   // }}}
 
   template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T, T, T, T)>
   inline void ippiThresholdCall_4T(const Img<T> *src, Img<T> *dst, T t1,T t2, T t3, T t4){
      // {{{ open

      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

      for (int c=src->getChannels()-1; c >= 0; --c) {
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
   // {{{ C++ fallback ThreshOp classes
  
  template <typename T> class ThresholdOp {
  public:
     virtual ~ThresholdOp () {}
     virtual T operator()(T val) const = 0;
  };

  template <typename T> class ThreshOpLT : public ThresholdOp<T> {
    // {{{ open

  public:
    ThreshOpLT (T t) : threshold(t) {}
    virtual T operator()(T val) const { 
       if (val < threshold) return threshold;
       else return val;
    }
  private:
     T threshold;
  };

  // }}}
  template <typename T> class ThreshOpGT : public ThresholdOp<T> {
    // {{{ open

  public:
    ThreshOpGT (T t) : threshold(t) {}
    virtual T operator()(T val) const { 
       if (val > threshold) return threshold;
       else return val;
    }
  private:
     T threshold;
  };

  // }}}
  template <typename T> class ThreshOpLTGT : public ThresholdOp<T> {
    // {{{ open
  public:
    ThreshOpLTGT(T tLow, T tUp) : tLow(tLow), tUp(tUp) {}
    virtual T operator()(T val) const { 
       if (val < tLow) return tLow;
       if (val > tUp)  return tUp;
       return val;
    }
  private:
    T tLow, tUp;
  };

  // }}}

  template <typename T> class ThreshOpLTVal : public ThresholdOp<T> {
    // {{{ open

  public:
    ThreshOpLTVal (T t, T v) : threshold(t), value(v) {}
    virtual T operator()(T val) const { 
      if (val < threshold) return value;
      return val;
    }
  private:
     T threshold;
     T value;
  };

  // }}}
  template <typename T> class ThreshOpGTVal : public ThresholdOp<T> {
    // {{{ open

  public:
    ThreshOpGTVal (T t, T v) : threshold(t), value(v) {}
    virtual T operator()(T val) const { 
      if (val > threshold) return value;
      return val;
    }
  private:
     T threshold;
     T value;
  };

  // }}}
  template <typename T> class ThreshOpLTGTVal : public ThresholdOp<T> {
    // {{{ open
  public:
    ThreshOpLTGTVal(T tLow, T vLow, T tUp, T vUp) : 
       tLow(tLow), tUp(tUp), vLow(vLow), vUp(vUp) {}
    virtual T operator()(T val) const { 
       if (val < tLow) return vLow;
       if (val > tUp)  return vUp;
       return val;
    }
  private:
    T tLow, tUp;
    T vLow, vUp;
  };

  // }}}

  // }}}

   // {{{ C++ fallback threshold function for all threshold operations
  
   template <typename T>
   void fallbackThreshold(const Img<T> *src, Img<T> *dst, const ThresholdOp<T> &threshold) {
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

      for(int c=src->getChannels()-1; c >= 0; --c) {
         ImgIterator<T> itSrc = const_cast<Img<T>*>(src)->getROIIterator(c);
         ImgIterator<T> itDst = dst->getROIIterator(c);
         for(;itSrc.inRegion(); ++itSrc, ++itDst){
            *itDst = threshold(*itSrc);
         }
      }
   }

   // }}}

   // {{{ template specializations without Val postfix (fallback)

   template <>
   void Threshold::lt<icl8u>(const Img8u *src, Img8u *dst, icl8u t){
      fallbackThreshold (src, dst, ThreshOpLT<icl8u>(t));
   }
   template <>
   void Threshold::lt<icl32f>(const Img32f *src, Img32f *dst, icl32f t){
      fallbackThreshold (src, dst, ThreshOpLT<icl32f>(t));
   }
   template <>
   void Threshold::gt<icl8u>(const Img8u *src, Img8u *dst, icl8u t){
      fallbackThreshold (src, dst, ThreshOpGT<icl8u>(t));
   }
   template <>
   void Threshold::gt<icl32f>(const Img32f *src, Img32f *dst, icl32f t){
      fallbackThreshold (src, dst, ThreshOpLT<icl32f>(t));
   }
   template <>
   void Threshold::ltgt<icl8u>(const Img8u *src, Img8u *dst, icl8u tMin, icl8u tMax){
      fallbackThreshold (src, dst, ThreshOpLTGT<icl8u>(tMin,tMax));
   }
   template <>
   void Threshold::ltgt<icl32f>(const Img32f *src, Img32f *dst, icl32f tMin, icl32f tMax){
      fallbackThreshold (src, dst, ThreshOpLTGT<icl32f>(tMin,tMax));
   }

   // }}}
  
   // {{{ template specializations with Val postfix (fallback)

   template <>
   void Threshold::ltVal<icl8u>(const Img8u *src, Img8u *dst, icl8u t, icl8u val){
      fallbackThreshold (src, dst, ThreshOpLTVal<icl8u>(t,val));
   }
   template <>
   void Threshold::ltVal<icl32f>(const Img32f *src, Img32f *dst, icl32f t, icl32f val){
      fallbackThreshold (src, dst, ThreshOpLTVal<icl32f>(t,val));
   }
   template <>
   void Threshold::gtVal<icl8u>(const Img8u *src, Img8u *dst, icl8u t, icl8u val){
      fallbackThreshold (src, dst, ThreshOpGTVal<icl8u>(t,val));
   }
   template <>
   void Threshold::gtVal<icl32f>(const Img32f *src, Img32f *dst, icl32f t, icl32f val){
      fallbackThreshold (src, dst, ThreshOpGTVal<icl32f>(t,val));
   }
   template <>
   void Threshold::ltgtVal<icl8u>(const Img8u *src, Img8u *dst, 
                                  icl8u tMin, icl8u minVal, icl8u tMax, icl8u maxVal){
      fallbackThreshold (src, dst, ThreshOpLTGTVal<icl8u>(tMin,minVal,tMax,maxVal));
   }
   template <>
   void Threshold::ltgtVal<icl32f>(const Img32f *src, Img32f *dst, 
                                   icl32f tMin, icl32f minVal, icl32f tMax, icl32f maxVal){
      fallbackThreshold (src, dst, ThreshOpLTGTVal<icl32f>(tMin,minVal,tMax,maxVal));
   }
   // }}}
#endif

   // {{{ ImgI* versions

   void Threshold::lt(const ImgI *poSrc, ImgI **ppoDst, icl32f t) {
      // {{{ open

      ensureCompatibleROI (ppoDst, poSrc);
      if (poSrc->getDepth () == depth8u)
         lt(poSrc->asImg<icl8u>(), (*ppoDst)->asImg<icl8u>(), Cast<icl32f,icl8u>::cast(t));
      else 
         lt(poSrc->asImg<icl32f>(), (*ppoDst)->asImg<icl32f>(), t);
   }

   // }}}
  
   void Threshold::gt(const ImgI *poSrc, ImgI **ppoDst, icl32f t) {
      // {{{ open

      ensureCompatibleROI (ppoDst, poSrc);
      if (poSrc->getDepth () == depth8u)
         gt(poSrc->asImg<icl8u>(), (*ppoDst)->asImg<icl8u>(), Cast<icl32f,icl8u>::cast(t));
      else 
         gt(poSrc->asImg<icl32f>(), (*ppoDst)->asImg<icl32f>(), t);
   }

   // }}}
  
   void Threshold::ltgt(const ImgI *poSrc, ImgI **ppoDst, icl32f tMin, icl32f tMax) {
      // {{{ open

      ensureCompatibleROI (ppoDst, poSrc);
      if (poSrc->getDepth () == depth8u)
         ltgt(poSrc->asImg<icl8u>(), (*ppoDst)->asImg<icl8u>(), 
              Cast<icl32f,icl8u>::cast(tMin), Cast<icl32f,icl8u>::cast(tMax));
      else 
         ltgt(poSrc->asImg<icl32f>(), (*ppoDst)->asImg<icl32f>(), tMin, tMax);
   }

   // }}}
  
   void Threshold::ltVal(const ImgI *poSrc, ImgI **ppoDst, icl32f t, icl32f val) {
      // {{{ open

      ensureCompatibleROI (ppoDst, poSrc);
      if (poSrc->getDepth () == depth8u)
         ltVal(poSrc->asImg<icl8u>(), (*ppoDst)->asImg<icl8u>(), 
               Cast<icl32f,icl8u>::cast(t), Cast<icl32f,icl8u>::cast(val));
      else 
         ltVal(poSrc->asImg<icl32f>(), (*ppoDst)->asImg<icl32f>(), t, val);
   }

   // }}}
  
   void Threshold::gtVal(const ImgI *poSrc, ImgI **ppoDst, icl32f t, icl32f val) {
      // {{{ open

      ensureCompatibleROI (ppoDst, poSrc);
      if (poSrc->getDepth () == depth8u)
         gtVal(poSrc->asImg<icl8u>(), (*ppoDst)->asImg<icl8u>(), 
               Cast<icl32f,icl8u>::cast(t), Cast<icl32f,icl8u>::cast(val));
      else 
         gtVal(poSrc->asImg<icl32f>(), (*ppoDst)->asImg<icl32f>(), t, val);
   }

   // }}}
  
   void Threshold::ltgtVal(const ImgI *poSrc, ImgI **ppoDst, 
                           icl32f tMin, icl32f minVal, icl32f tMax, icl32f maxVal) {
      // {{{ open

      ensureCompatibleROI (ppoDst, poSrc);
      if (poSrc->getDepth () == depth8u)
         ltgtVal(poSrc->asImg<icl8u>(), (*ppoDst)->asImg<icl8u>(), 
                 Cast<icl32f,icl8u>::cast(tMin), Cast<icl32f,icl8u>::cast(minVal), 
                 Cast<icl32f,icl8u>::cast(tMax), Cast<icl32f,icl8u>::cast(maxVal));
      else 
         ltgtVal(poSrc->asImg<icl32f>(), (*ppoDst)->asImg<icl32f>(), tMin, minVal, tMax, maxVal);
   }

   // }}}

// }}}
  
  
} // namespace icl
