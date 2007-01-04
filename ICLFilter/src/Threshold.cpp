#include <Threshold.h>
#include <Img.h>

namespace icl {

  
  
  
   // {{{ C++ fallback ThreshOp classes
  
   template <typename T> class ThreshOpLTVal {
      // {{{ open

   public:
      ThreshOpLTVal (T t, T v) : threshold(t), value(v) {}
      inline T operator()(T val) const { 
         if (val < threshold) return value;
         return val;
      }
   private:
      T threshold;
      T value;
   };

   // }}}
   template <typename T> class ThreshOpGTVal {
      // {{{ open

   public:
      ThreshOpGTVal (T t, T v) : threshold(t), value(v) {}
      inline T operator()(T val) const { 
         if (val > threshold) return value;
         return val;
      }
   private:
      T threshold;
      T value;
   };

   // }}}
   template <typename T> class ThreshOpLTGTVal {
      // {{{ open
   public:
      ThreshOpLTGTVal(T tLow, T vLow, T tUp, T vUp) : 
         tLow(tLow), tUp(tUp), vLow(vLow), vUp(vUp) {}
      inline T operator()(T val) const { 
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
  
   template <typename T, class ThresholdOp>
   inline void fallbackThreshold(const Img<T> *src, Img<T> *dst, 
                          const ThresholdOp &threshold) {
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

      for(int c=src->getChannels()-1; c >= 0; --c) {
         ConstImgIterator<T> itSrc = src->getROIIterator(c);
         ImgIterator<T> itDst = dst->getROIIterator(c);
         for(;itSrc.inRegion(); ++itSrc, ++itDst){
            *itDst = threshold(*itSrc);
         }
      }
   }

   // }}}

#define ICL_INSTANTIATE_ALL_IPP_DEPTHS \
  ICL_INSTANTIATE_DEPTH(8u)  \
  ICL_INSTANTIATE_DEPTH(16s) \
  ICL_INSTANTIATE_DEPTH(32f)

#define ICL_INSTANTIATE_ALL_FB_DEPTHS \
  ICL_INSTANTIATE_DEPTH(32s)  \
  ICL_INSTANTIATE_DEPTH(64f)


   
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
  
   // {{{ function specializations without Val postfix
 
#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::lt(const Img ## T *src,Img ## T *dst, icl ## T t){\
    ippiThresholdCall_1T<icl ## T, ippiThreshold_LT_ ## T ## _C1R> (src, dst, t);}

  ICL_INSTANTIATE_ALL_IPP_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::gt(const Img ## T *src,Img ## T *dst, icl ## T t){\
    ippiThresholdCall_1T<icl ## T, ippiThreshold_GT_ ## T ## _C1R> (src, dst, t);}

  ICL_INSTANTIATE_ALL_IPP_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::ltgt(const Img ## T *src,Img ## T *dst, icl ## T tMin, icl ## T tMax){\
      ippiThresholdCall_4T<icl ## T, ippiThreshold_LTValGTVal_ ## T ## _C1R> (src, dst, tMin,tMin, tMax,tMax);}

  ICL_INSTANTIATE_ALL_IPP_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

   // }}}
 
   // {{{ function specializations with Val postfix

#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::ltVal(const Img ## T *src,Img ## T *dst, icl ## T t, icl ## T val){\
    ippiThresholdCall_2T<icl ## T , ippiThreshold_LTVal_ ## T ## _C1R> (src, dst, t, val);}

  ICL_INSTANTIATE_ALL_IPP_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::gtVal(const Img ## T *src,Img ## T *dst, icl ## T t, icl ## T val){\
    ippiThresholdCall_2T<icl ## T , ippiThreshold_GTVal_ ## T ## _C1R> (src, dst, t, val);}

  ICL_INSTANTIATE_ALL_IPP_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::ltgtVal(const Img ## T *src,Img ## T *dst, icl ## T tMin,icl ## T minVal, icl ## T tMax,icl ## T maxVal){\
      ippiThresholdCall_4T<icl ## T, ippiThreshold_LTValGTVal_ ## T ##_C1R> (src, dst, tMin, minVal, tMax, maxVal);}

  ICL_INSTANTIATE_ALL_IPP_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

   // }}}
#endif 

   // {{{ function specializations without Val postfix (fallback)
   /* We just use the appropriate *Val functions, because there is no performance
      gain implementing a specialised variant */


#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::lt(const Img ## T *src, Img ## T *dst, icl ## T t){\
    Threshold::ltVal(src, dst, t, t);}
#ifdef WITH_IPP_OPTIMIZATION
  ICL_INSTANTIATE_ALL_FB_DEPTHS
#else
  ICL_INSTANTIATE_ALL_DEPTHS
#endif
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::gt(const Img ## T *src, Img ## T *dst, icl ## T t){\
    Threshold::gtVal(src, dst, t, t);}
#ifdef WITH_IPP_OPTIMIZATION
  ICL_INSTANTIATE_ALL_FB_DEPTHS
#else
  ICL_INSTANTIATE_ALL_DEPTHS
#endif
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::ltgt(const Img ## T *src, Img ## T *dst, icl ## T tMin, icl ## T tMax){\
    Threshold::ltgtVal(src, dst, tMin,tMin, tMax,tMax);}
#ifdef WITH_IPP_OPTIMIZATION
  ICL_INSTANTIATE_ALL_FB_DEPTHS
#else
  ICL_INSTANTIATE_ALL_DEPTHS
#endif
#undef ICL_INSTANTIATE_DEPTH

   // }}}
  
   // {{{ function specializations with Val postfix (fallback)

#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::ltVal(const Img ##T *src, Img ## T *dst, icl ## T t, icl ## T val){\
    fallbackThreshold (src, dst, ThreshOpLTVal<icl ## T>(t,val));}
#ifdef WITH_IPP_OPTIMIZATION
  ICL_INSTANTIATE_ALL_FB_DEPTHS
#else
  ICL_INSTANTIATE_ALL_DEPTHS
#endif
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::gtVal(const Img ##T *src, Img ## T *dst, icl ## T t, icl ## T val){\
    fallbackThreshold (src, dst, ThreshOpGTVal<icl ## T>(t,val));}
#ifdef WITH_IPP_OPTIMIZATION
  ICL_INSTANTIATE_ALL_FB_DEPTHS
#else
  ICL_INSTANTIATE_ALL_DEPTHS
#endif
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Threshold::ltgtVal(const Img ## T *src, Img ## T *dst, icl ## T tMin, icl ## T minVal, icl ##T tMax, icl ## T maxVal){\
      fallbackThreshold (src, dst, ThreshOpLTGTVal<icl ## T>(tMin,minVal,tMax,maxVal));}
#ifdef WITH_IPP_OPTIMIZATION
  ICL_INSTANTIATE_ALL_FB_DEPTHS
#else
  ICL_INSTANTIATE_ALL_DEPTHS
#endif
#undef ICL_INSTANTIATE_DEPTH

   // }}}

#undef ICL_INSTANTIATE_ALL_IPP_DEPTHS

#undef ICL_INSTANTIATE_ALL_FB_DEPTHS


   // {{{ ImgBase* versions

#define ICL_INSTANTIATE_DEPTH(T) \
    case depth ## T: lt(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), Cast<icl32f,icl ## T>::cast(t)); break;
  void Threshold::lt(const ImgBase *poSrc, ImgBase **ppoDst, icl32f t)
    // {{{ open
  {
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_DEPTH; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}
  
#define ICL_INSTANTIATE_DEPTH(T) \
    case depth ## T: gt(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), Cast<icl32f,icl ## T>::cast(t)); break;
  void Threshold::gt(const ImgBase *poSrc, ImgBase **ppoDst, icl32f t)
    // {{{ open
  {
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_DEPTH; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}
  
#define ICL_INSTANTIATE_DEPTH(T) \
    case depth ## T: ltgt(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), Cast<icl32f,icl ## T>::cast(tMin), Cast<icl32f,icl ## T>::cast(tMax)); break;
  void Threshold::ltgt(const ImgBase *poSrc, ImgBase **ppoDst, icl32f tMin, icl32f tMax)
    // {{{ open
  {
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_DEPTH; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}
  
#define ICL_INSTANTIATE_DEPTH(T) \
    case depth ## T: ltVal(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), Cast<icl32f,icl ## T>::cast(t), Cast<icl32f,icl ## T>::cast(val)); break;
  void Threshold::ltVal(const ImgBase *poSrc, ImgBase **ppoDst, icl32f t, icl32f val)
    // {{{ open
  {
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_DEPTH; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}
  
#define ICL_INSTANTIATE_DEPTH(T) \
    case depth ## T: gtVal(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), Cast<icl32f,icl ## T>::cast(t), Cast<icl32f,icl ## T>::cast(val)); break;
  void Threshold::gtVal(const ImgBase *poSrc, ImgBase **ppoDst, icl32f t, icl32f val)
    // {{{ open
  {
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_DEPTH; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}
  
#define ICL_INSTANTIATE_DEPTH(T) \
    case depth ## T: ltgtVal(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), Cast<icl32f,icl ## T>::cast(tMin),\
                Cast<icl32f,icl ##T>::cast(minVal), Cast<icl32f,icl ## T>::cast(tMax), Cast<icl32f,icl ## T>::cast(maxVal));break;
  void Threshold::ltgtVal(const ImgBase *poSrc, ImgBase **ppoDst, 
                           icl32f tMin, icl32f minVal, icl32f tMax, icl32f maxVal)
    // {{{ open
  {
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_DEPTH; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
// }}}  
  
} // namespace icl
