
#include <Compare.h>
#include <Img.h>

namespace icl {

#define ICL_COMP_ZERO 0
#define ICL_COMP_NONZERO 255

  
   // {{{ C++ fallback CompareOp classes
   
   template <typename T> class CompareOpEq {
      // {{{ open
   public:
      inline icl8u operator()(T val1, T val2) const
         {
            return val1 == val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
         }
   };
   // }}}
  
   template <typename T> class CompareOpLess {
      // {{{ open
   public:
      inline icl8u operator()(T val1, T val2) const
         {
            return val1 < val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
         }
   };
   // }}}

   template <typename T> class CompareOpLessEq {
      // {{{ open
   public:
      inline icl8u operator()(T val1, T val2) const
         {
            return val1 <= val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
         }
   };
   // }}}

   template <typename T> class CompareOpGreater {
      // {{{ open
   public:
      inline icl8u operator()(T val1, T val2) const
         {
            return val1 > val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
         }
   };
   // }}}

   template <typename T> class CompareOpGreaterEq {
      // {{{ open
   public:
      inline icl8u operator()(T val1, T val2) const
         {
            return val1 >= val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
         }
   };
   // }}}

   template <typename T> class CompareOpEqEps {
      // {{{ open
   public:
      inline icl8u operator()(T val1, T val2, T eps) const {
         return (std::abs(val1-val2) <= eps) ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
      }
   };
   // }}}

   // }}}

   // {{{ C++ fallback compare functions as templates

   template <typename T, class CompareOp>
   inline void fallbackCompare(const Img<T> *src1, const Img<T> *src2, Img8u *dst, const CompareOp &compare)
      // {{{ open
   {
      ICLASSERT_RETURN( src1 && src2 && dst );
      ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
      ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
      ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );
      for(int c=src1->getChannels()-1; c >= 0; --c) {
         ConstImgIterator<T> itSrc1 = src1->getROIIterator(c);
         ConstImgIterator<T> itSrc2 = src2->getROIIterator(c);
         ImgIterator<icl8u> itDst = dst->getROIIterator(c);
         for(;itSrc1.inRegion(); ++itSrc1, ++itSrc2, ++itDst){
            *itDst = compare(*itSrc1,*itSrc2);
         }
      }
   }

   // }}}

   template <typename T, class CompareOp>
   inline void fallbackCompareC(const Img<T> *src, T value, Img8u *dst, const CompareOp &compare)
      // {{{ open
   {
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

      for(int c=src->getChannels()-1; c >= 0; --c) {
         ConstImgIterator<T> itSrc = src->getROIIterator(c);
         ImgIterator<icl8u>  itDst = dst->getROIIterator(c);
         for(;itSrc.inRegion(); ++itSrc, ++itDst){
            *itDst = compare(*itSrc,value);
         }
      }
   }
   // }}}

   template <typename T>
   inline void fallbackEqualEps(const Img<T> *src1, const Img<T> *src2, Img8u *dst, T eps)
      // {{{ open
   {
      ICLASSERT_RETURN( src1 && src2 && dst );
      ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
      ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
      ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );

      CompareOpEqEps<T> compare;
      for(int c=src1->getChannels()-1; c >= 0; --c) {
         ConstImgIterator<T> itSrc1 = src1->getROIIterator(c);
         ConstImgIterator<T> itSrc2 = src2->getROIIterator(c);
         ImgIterator<icl8u>  itDst = dst->getROIIterator(c);
         for(;itSrc1.inRegion(); ++itSrc1, ++itSrc2, ++itDst){
            *itDst = compare(*itSrc1,*itSrc2,eps);
         }
      }
   }
   // }}}
  
   template <typename T>
   inline void fallbackEqualEpsC(const Img<T> *src, T value, Img8u *dst,T eps)
      // {{{ open
   {
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

      CompareOpEqEps<T> compare;
      for(int c=src->getChannels()-1; c >= 0; --c) {
         ConstImgIterator<T> itSrc = src->getROIIterator(c);
         ImgIterator<icl8u>  itDst = dst->getROIIterator(c);
         for(;itSrc.inRegion(); ++itSrc, ++itDst){
            *itDst = compare(*itSrc,value,eps);
         }
      }
   }
   // }}}

   // }}}
  
    
#ifdef WITH_IPP_OPTIMIZATION

   // {{{ ippi-function call templates

   template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, const T*, int, icl8u*, int, IppiSize, IppCmpOp)>
   inline void ippiCompare(const Img<T> *src1, const Img<T> *src2, Img8u *dst, Compare::op cmpOp)
      // {{{ open
   {  
      ICLASSERT_RETURN( src1 && src2 && dst );
      ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
      ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
      ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );

      for (int c=src1->getChannels()-1; c >= 0; --c) {
         ippiFunc (src1->getROIData (c), src1->getLineStep(),
                   src2->getROIData (c), src2->getLineStep(),
                   dst->getROIData (c), dst->getLineStep(),
                   dst->getROISize(), (IppCmpOp) cmpOp);
      }
   }
   // }}}

   template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T, icl8u*, int, IppiSize, IppCmpOp)>
   inline void ippiCompareC(const Img<T> *src, T value, Img8u *dst, Compare::op cmpOp)
      // {{{ open
   {
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

      for (int c=src->getChannels()-1; c >= 0; --c) {
         ippiFunc (src->getROIData (c), src->getLineStep(), value,
                   dst->getROIData (c), dst->getLineStep(),
                   dst->getROISize(),(IppCmpOp) cmpOp);
      }
   }
   // }}}

   // }}}

   // {{{ compare 

  void Compare::compare(const Img8u *src1,const Img8u *src2,Img8u *dst, Compare::op cmpOp){ 
    ippiCompare<icl8u, ippiCompare_8u_C1R> (src1,src2, dst, cmpOp);
  }
  void Compare::compare(const Img16s *src1,const Img16s *src2,Img8u *dst, Compare::op cmpOp){
    ippiCompare<icl16s, ippiCompare_16s_C1R> (src1,src2, dst, cmpOp);
  }
   void Compare::compare(const Img32s *src1, const Img32s *src2, Img8u *dst, Compare::op cmpOp)
      // {{{ open
   {
      switch (cmpOp){
         case Compare::compareEq:
            fallbackCompare (src1, src2, dst, CompareOpEq<icl32s>()); break;
         case Compare::compareLess:
            fallbackCompare (src1, src2, dst, CompareOpLess<icl32s>()); break;
         case Compare::compareLessEq:
            fallbackCompare (src1, src2, dst, CompareOpLessEq<icl32s>()); break;
         case Compare::compareGreater:
            fallbackCompare (src1, src2, dst, CompareOpGreater<icl32s>()); break;
         case Compare::compareGreaterEq:
            fallbackCompare (src1, src2, dst, CompareOpGreaterEq<icl32s>()); break;
      }
   }
   // }}}
  void Compare::compare(const Img32f *src1,const Img32f *src2,Img8u *dst, Compare::op cmpOp){
    ippiCompare<icl32f, ippiCompare_32f_C1R> (src1,src2, dst, cmpOp);
  }
   void Compare::compare(const Img64f *src1, const Img64f *src2, Img8u *dst, Compare::op cmpOp)
      // {{{ open
   {
      switch (cmpOp){
         case Compare::compareEq:
            fallbackCompare (src1, src2, dst, CompareOpEq<icl64f>()); break;
         case Compare::compareLess:
            fallbackCompare (src1, src2, dst, CompareOpLess<icl64f>()); break;
         case Compare::compareLessEq:
            fallbackCompare (src1, src2, dst, CompareOpLessEq<icl64f>()); break;
         case Compare::compareGreater:
            fallbackCompare (src1, src2, dst, CompareOpGreater<icl64f>()); break;
         case Compare::compareGreaterEq:
            fallbackCompare (src1, src2, dst, CompareOpGreaterEq<icl64f>()); break;
      }
   }
   // }}}

  
  
  
   void Compare::equalEps(const Img8u *src1,const Img8u *src2,Img8u *dst, icl8u eps)
      // {{{ open
   {
      ICLASSERT_RETURN( src1 && src2 && dst );
      ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
      ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
      ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );

      for (int c=src1->getChannels()-1; c >= 0; --c) {
         ippiAbsDiff_8u_C1R (src1->getROIData (c), src1->getLineStep(),
                             src2->getROIData (c), src2->getLineStep(),
                             dst->getROIData (c), dst->getLineStep(),
                             dst->getROISize());
         ippiThreshold_LTValGTVal_8u_C1IR (dst->getROIData (c), dst->getLineStep(),
                                           dst->getROISize(), 
                                           eps, ICL_COMP_NONZERO, eps, ICL_COMP_ZERO);
      }
   }
   // }}}
   void Compare::equalEps(const Img32f *src1,const Img32f *src2,Img8u *dst, icl32f eps)
      // {{{ open
   {
      ICLASSERT_RETURN( src1 && src2 && dst );
      ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
      ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
      ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );

      for (int c=src1->getChannels()-1; c >= 0; --c) {
         ippiCompareEqualEps_32f_C1R (src1->getROIData (c), src1->getLineStep(),
                                      src2->getROIData (c), src2->getLineStep(),
                                      dst->getROIData (c), dst->getLineStep(),
                                      dst->getROISize(), eps);
      }
   }
   // }}}

   // }}}

   // {{{ compareC

  void Compare::compareC(const Img8u *src, icl8u value, Img8u *dst, Compare::op cmpOp){
    ippiCompareC<icl8u, ippiCompareC_8u_C1R> (src, value, dst, cmpOp);
  }
  void Compare::compareC(const Img16s *src, icl16s value, Img8u *dst, Compare::op cmpOp){
    ippiCompareC<icl16s, ippiCompareC_16s_C1R> (src, value, dst, cmpOp);
  }
   void Compare::compareC(const Img32s *src, icl32s value, Img8u *dst, Compare::op cmpOp)
      // {{{ open
   {
      switch (cmpOp){
         case Compare::compareEq:
            fallbackCompareC (src, value, dst, CompareOpEq<icl32s>()); break;
         case Compare::compareLess:
            fallbackCompareC (src, value, dst, CompareOpLess<icl32s>()); break;
         case Compare::compareLessEq:
            fallbackCompareC (src, value, dst, CompareOpLessEq<icl32s>()); break;
         case Compare::compareGreater:
            fallbackCompareC (src, value, dst, CompareOpGreater<icl32s>()); break;
         case Compare::compareGreaterEq:
            fallbackCompareC (src, value, dst, CompareOpGreaterEq<icl32s>()); break;
      }
   }
   // }}}

  void Compare::compareC(const Img32f *src, icl32f value, Img8u *dst, Compare::op cmpOp){
    ippiCompareC<icl32f, ippiCompareC_32f_C1R> (src, value, dst, cmpOp);
  }

   void Compare::compareC(const Img64f *src, icl64f value, Img8u *dst, Compare::op cmpOp)
      // {{{ open
   {
      switch (cmpOp){
         case Compare::compareEq:
            fallbackCompareC (src, value, dst, CompareOpEq<icl64f>()); break;
         case Compare::compareLess:
            fallbackCompareC (src, value, dst, CompareOpLess<icl64f>()); break;
         case Compare::compareLessEq:
            fallbackCompareC (src, value, dst, CompareOpLessEq<icl64f>()); break;
         case Compare::compareGreater:
            fallbackCompareC (src, value, dst, CompareOpGreater<icl64f>()); break;
         case Compare::compareGreaterEq:
            fallbackCompareC (src, value, dst, CompareOpGreaterEq<icl64f>()); break;
      }
   }
   // }}}




  
   void Compare::equalEpsC(const Img8u *src, icl8u value, Img8u *dst, icl8u eps)
      // {{{ open
   {
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

      for (int c=src->getChannels()-1; c >= 0; --c) {
         ippiAbsDiffC_8u_C1R (src->getROIData (c), src->getLineStep(),
                              dst->getROIData (c), dst->getLineStep(),
                              dst->getROISize(), value);
         ippiThreshold_LTValGTVal_8u_C1IR (dst->getROIData (c), dst->getLineStep(),
                                           dst->getROISize(), 
                                           eps, ICL_COMP_NONZERO, eps, ICL_COMP_ZERO);
      }
   }
   // }}}
   void Compare::equalEpsC(const Img32f *src, icl32f value, Img8u *dst, icl32f eps)
      // {{{ open
   {

      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

      for (int c=src->getChannels()-1; c >= 0; --c) {
         ippiCompareEqualEpsC_32f_C1R (src->getROIData (c), src->getLineStep(), value,
                                       dst->getROIData (c), dst->getLineStep(),
                                       dst->getROISize(), eps);
      }
   }
   // }}}

  //No IPP Functions available -> Fallback
  void Compare::equalEps(const Img16s *src1, const Img16s *src2,Img8u *dst, icl16s eps){
    fallbackEqualEps<icl16s>(src1, src2, dst, eps);
  }
  void Compare::equalEps(const Img32s *src1, const Img32s *src2,Img8u *dst, icl32s eps){
    fallbackEqualEps<icl32s>(src1, src2, dst, eps);
  }
  void Compare::equalEps(const Img64f *src1, const Img64f *src2,Img8u *dst, icl64f eps){
    fallbackEqualEps<icl64f>(src1, src2, dst, eps);
  }

  void Compare::equalEpsC(const Img16s *src1, icl16s value,Img8u *dst, icl16s eps)
  {
    fallbackEqualEpsC<icl16s>(src1, value, dst, eps);
  }
  void Compare::equalEpsC(const Img32s *src1, icl32s value,Img8u *dst, icl32s eps)
  {
    fallbackEqualEpsC<icl32s>(src1, value, dst, eps);
  }
  void Compare::equalEpsC(const Img64f *src1, icl64f value,Img8u *dst, icl64f eps)
  {
    fallbackEqualEpsC<icl64f>(src1, value, dst, eps);
  }
   
   // }}}
  
#else // NO IPP_OPTIMIZATION

  
   // {{{ C++ compare

   // {{{ open

   #define ICL_INSTANTIATE_DEPTH(T) \
   void Compare::compare(const Img ## T *src1, const Img ## T *src2, Img8u *dst, Compare::op cmpOp){\
      switch (cmpOp){\
         case Compare::compareEq:\
            fallbackCompare (src1, src2, dst, CompareOpEq<icl ## T>()); break;\
         case Compare::compareLess:\
            fallbackCompare (src1, src2, dst, CompareOpLess<icl ## T>()); break;\
         case Compare::compareLessEq:\
            fallbackCompare (src1, src2, dst, CompareOpLessEq<icl ## T>()); break;\
         case Compare::compareGreater:\
            fallbackCompare (src1, src2, dst, CompareOpGreater<icl ## T>()); break;\
         case Compare::compareGreaterEq:\
            fallbackCompare (src1, src2, dst, CompareOpGreaterEq<icl ## T>()); break;\
      }\
   }

  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

   
   // }}}
   


   
#define ICL_INSTANTIATE_DEPTH(T) \
  void Compare::equalEps(const Img ## T *src1, const Img ## T *src2,Img8u *dst, icl ## T eps){\
    fallbackEqualEps<icl ## T>(src1, src2, dst, eps); }
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

  
   // }}}

   // {{{ C++ compareC

   // {{{ open
   #define ICL_INSTANTIATE_DEPTH(T) \
    void Compare::compareC(const Img ## T *src, icl ## T value, Img8u *dst, Compare::op cmpOp){\
      switch (cmpOp){\
         case Compare::compareEq:\
            fallbackCompareC (src, value, dst, CompareOpEq<icl ## T>()); break;\
         case Compare::compareLess:\
            fallbackCompareC (src, value, dst, CompareOpLess<icl ## T>()); break;\
         case Compare::compareLessEq:\
            fallbackCompareC (src, value, dst, CompareOpLessEq<icl ## T>()); break;\
         case Compare::compareGreater:\
            fallbackCompareC (src, value, dst, CompareOpGreater<icl ## T>()); break;\
         case Compare::compareGreaterEq:\
            fallbackCompareC (src, value, dst, CompareOpGreaterEq<icl ## T>()); break;\
      }\
   }
   // }}}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

   
   
#define ICL_INSTANTIATE_DEPTH(T) \
  void Compare::equalEpsC(const Img ## T *src1, icl ## T value,Img8u *dst, icl ## T eps){ \
    fallbackEqualEpsC<icl ## T>(src1, value, dst, eps);}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  

   // }}}
  
#endif

   // {{{ ImgBase* functions 

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: compare(poSrc1->asImg<icl ## T>(),poSrc2->asImg<icl ## T>(),(*ppoDst)->asImg<icl8u>(),cmpOp); break;
  void Compare::compare(const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst, Compare::op cmpOp)
    // {{{ open
  {
    ICLASSERT_RETURN( poSrc1 && poSrc2 && poSrc1->getDepth() == poSrc2->getDepth() );
    ICLASSERT_RETURN( poSrc1->getROISize() == poSrc2->getROISize() );
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    if (!Filter::prepare (ppoDst, poSrc1, depth8u)) return;
    switch (poSrc1->getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: equalEps(poSrc1->asImg<icl ## T>(),poSrc2->asImg<icl ## T>(), (*ppoDst)->asImg<icl8u>(),Cast<icl32f,icl ## T>::cast(eps)); break;
  void Compare::equalEps(const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst, icl32f eps)
    // {{{ open
  {
    ICLASSERT_RETURN( poSrc1 && poSrc2 && poSrc1->getDepth() == poSrc2->getDepth() );
    ICLASSERT_RETURN( poSrc1->getROISize() == poSrc2->getROISize() );
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );

    if (!Filter::prepare (ppoDst, poSrc1, depth8u)) return;
    switch (poSrc1->getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    }//??
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: compareC(poSrc->asImg<icl ## T>(),Cast<icl32f,icl ## T>::cast(value),(*ppoDst)->asImg<icl8u>(),cmpOp); break;
  void Compare::compareC(const ImgBase *poSrc, icl32f value, ImgBase **ppoDst, Compare::op cmpOp)
    // {{{ open
  {
    if (!Filter::prepare (ppoDst, poSrc, depth8u)) return;
    switch (poSrc->getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: equalEpsC(poSrc->asImg<icl ## T>(),Cast<icl32f,icl ## T>::cast(value),(*ppoDst)->asImg<icl8u>(),Cast<icl32f,icl ##T>::cast(eps)); break;
  void Compare::equalEpsC(const ImgBase *poSrc, icl32f value, ImgBase **ppoDst, icl32f eps)
    // {{{ open
  {
    if (!Filter::prepare (ppoDst, poSrc, depth8u)) return;
    switch (poSrc->getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}
    
// }}}
}
