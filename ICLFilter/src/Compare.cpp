#include <Compare.h>
#include <Img.h>

namespace icl {

#define ICL_COMP_ZERO 0
#define ICL_COMP_NONZERO 255

#ifdef WITH_IPP_OPTIMIZATION

   // {{{ ippi-function call templates

   template <typename T, IppStatus (*ippiFunc) (const T*, int, const T*, int, icl8u*, int, IppiSize, IppCmpOp)>
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

   template <typename T, IppStatus (*ippiFunc) (const T*, int, T, icl8u*, int, IppiSize, IppCmpOp)>
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

   void Compare::compare(const Img8u *src1,const Img8u *src2,Img8u *dst, Compare::op cmpOp)
   { ippiCompare<icl8u, ippiCompare_8u_C1R> (src1,src2, dst, cmpOp); }   
   void Compare::compare(const Img32f *src1,const Img32f *src2,Img8u *dst, Compare::op cmpOp)
   { ippiCompare<icl32f, ippiCompare_32f_C1R> (src1,src2, dst, cmpOp); }

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

   void Compare::compareC(const Img8u *src, icl8u value, Img8u *dst, Compare::op cmpOp)
   { ippiCompareC<icl8u, ippiCompareC_8u_C1R> (src, value, dst, cmpOp); }   
   void Compare::compareC(const Img32f *src, icl32f value, Img8u *dst, Compare::op cmpOp)
   { ippiCompareC<icl32f, ippiCompareC_32f_C1R> (src, value, dst, cmpOp); }
   
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

   // }}}
  
#else // NO IPP_OPTIMISATION

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
   void fallbackCompare(const Img<T> *src1, const Img<T> *src2, Img8u *dst, const CompareOp &compare)
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
   void fallbackCompareC(const Img<T> *src, T value, Img8u *dst, const CompareOp &compare)
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
   void fallbackEqualEps(const Img<T> *src1, const Img<T> *src2, Img8u *dst, T eps)
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
   void fallbackEqualEpsC(const Img<T> *src, T value, Img8u *dst,T eps)
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
  
   // {{{ C++ compare

   void Compare::compare(const Img8u *src1,const Img8u *src2,Img8u *dst, Compare::op cmpOp)
      // {{{ open
   {
      switch (cmpOp) {
         case Compare::compareEq:
            fallbackCompare (src1, src2, dst, CompareOpEq<icl8u>()); break;
         case Compare::compareLess:
            fallbackCompare (src1, src2, dst, CompareOpLess<icl8u>()); break;
         case Compare::compareLessEq:
            fallbackCompare (src1, src2, dst, CompareOpLessEq<icl8u>()); break;
         case Compare::compareGreater:
            fallbackCompare (src1, src2, dst, CompareOpGreater<icl8u>()); break;
         case Compare::compareGreaterEq:
            fallbackCompare (src1, src2, dst, CompareOpGreaterEq<icl8u>()); break;
      }
   }

   // }}}
   void Compare::compare(const Img32f *src1, const Img32f *src2, Img8u *dst, Compare::op cmpOp)
      // {{{ open
   {
      switch (cmpOp){
         case Compare::compareEq:
            fallbackCompare (src1, src2, dst, CompareOpEq<icl32f>()); break;
         case Compare::compareLess:
            fallbackCompare (src1, src2, dst, CompareOpLess<icl32f>()); break;
         case Compare::compareLessEq:
            fallbackCompare (src1, src2, dst, CompareOpLessEq<icl32f>()); break;
         case Compare::compareGreater:
            fallbackCompare (src1, src2, dst, CompareOpGreater<icl32f>()); break;
         case Compare::compareGreaterEq:
            fallbackCompare (src1, src2, dst, CompareOpGreaterEq<icl32f>()); break;
      }
   }

   // }}}

   void Compare::equalEps(const Img8u *src1, const Img8u *src2,Img8u *dst, icl8u eps)
   { fallbackEqualEps<icl8u>(src1, src2, dst, eps); }
   void Compare::equalEps(const Img32f *src1, const Img32f *src2,Img8u *dst, icl32f eps)
   { fallbackEqualEps<icl32f>(src1, src2, dst, eps); }

   // }}}

   // {{{ C++ compareC

   void Compare::compareC(const Img8u *src, icl8u value, Img8u *dst, Compare::op cmpOp)
      // {{{ open
   {
      switch (cmpOp){
         case Compare::compareEq:
            fallbackCompareC (src, value, dst, CompareOpEq<icl8u>()); break;
         case Compare::compareLess:
            fallbackCompareC (src, value, dst, CompareOpLess<icl8u>()); break;
         case Compare::compareLessEq:
            fallbackCompareC (src, value, dst, CompareOpLessEq<icl8u>()); break;
         case Compare::compareGreater:
            fallbackCompareC (src, value, dst, CompareOpGreater<icl8u>()); break;
         case Compare::compareGreaterEq:
            fallbackCompareC (src, value, dst, CompareOpGreaterEq<icl8u>()); break;
      }
   }
   // }}}
   void Compare::compareC(const Img32f *src, icl32f value, Img8u *dst, Compare::op cmpOp)
      // {{{ open
   {
      switch (cmpOp){
         case Compare::compareEq:
            fallbackCompareC (src, value, dst, CompareOpEq<icl32f>()); break;
         case Compare::compareLess:
            fallbackCompareC (src, value, dst, CompareOpLess<icl32f>()); break;
         case Compare::compareLessEq:
            fallbackCompareC (src, value, dst, CompareOpLessEq<icl32f>()); break;
         case Compare::compareGreater:
            fallbackCompareC (src, value, dst, CompareOpGreater<icl32f>()); break;
         case Compare::compareGreaterEq:
            fallbackCompareC (src, value, dst, CompareOpGreaterEq<icl32f>()); break;
      }
   }
   // }}}

   void Compare::equalEpsC(const Img8u *src1, icl8u value,Img8u *dst, icl8u eps)
   { fallbackEqualEpsC<icl8u>(src1, value, dst, eps); }  
   void Compare::equalEpsC(const Img32f *src1, icl32f value,Img8u *dst, icl32f eps)
   { fallbackEqualEpsC<icl32f>(src1, value, dst, eps); }

   // }}}
  
#endif

   // {{{ ImgBase* functions 
  
  void Compare::compare(const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst, Compare::op cmpOp)
    // {{{ open
  {
    ICLASSERT_RETURN( poSrc1 && poSrc2 && poSrc1->getDepth() == poSrc2->getDepth() );
    ICLASSERT_RETURN( poSrc1->getROISize() == poSrc2->getROISize() );
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    if (!Filter::prepare (ppoDst, poSrc1, depth8u)) return;
    switch (poSrc1->getDepth()){
      case depth8u: compare(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),cmpOp); break;
      case depth32f: compare(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl8u>(),cmpOp); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}
  
  void Compare::equalEps(const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst, icl32f eps)
    // {{{ open
  {
    ICLASSERT_RETURN( poSrc1 && poSrc2 && poSrc1->getDepth() == poSrc2->getDepth() );
    ICLASSERT_RETURN( poSrc1->getROISize() == poSrc2->getROISize() );
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );

    if (!Filter::prepare (ppoDst, poSrc1, depth8u)) return;
    switch (poSrc1->getDepth()){
      case depth8u: equalEps(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(), (*ppoDst)->asImg<icl8u>(),Cast<icl32f,icl8u>::cast(eps)); break;
      case depth32f: equalEps(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(), (*ppoDst)->asImg<icl8u>(),eps); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}

  void Compare::compareC(const ImgBase *poSrc, icl32f value, ImgBase **ppoDst, Compare::op cmpOp)
    // {{{ open
  {
    if (!Filter::prepare (ppoDst, poSrc, depth8u)) return;
    switch (poSrc->getDepth()){
      case depth8u: compareC(poSrc->asImg<icl8u>(),Cast<icl32f,icl8u>::cast(value),(*ppoDst)->asImg<icl8u>(),cmpOp); break;
      case depth32f: compareC(poSrc->asImg<icl32f>(),value,(*ppoDst)->asImg<icl8u>(),cmpOp); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}
  
  void Compare::equalEpsC(const ImgBase *poSrc, icl32f value, ImgBase **ppoDst, icl32f eps)
    // {{{ open
  {
    if (!Filter::prepare (ppoDst, poSrc, depth8u)) return;
    switch (poSrc->getDepth()){
      case depth8u: equalEpsC(poSrc->asImg<icl8u>(),Cast<icl32f,icl8u>::cast(value),(*ppoDst)->asImg<icl8u>(),Cast<icl32f,icl8u>::cast(eps)); break;
      case depth32f: equalEpsC(poSrc->asImg<icl32f>(),value,(*ppoDst)->asImg<icl8u>(),eps); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}
    
// }}}
}
