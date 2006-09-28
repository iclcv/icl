#include "Compare.h"
#include "Macros.h"

namespace icl {
#define ICL_COMP_ZERO 0
#define ICL_COMP_NONZERO 255

// {{{ C++ fallback for 8u EPS  needed with and without ipp optimization
  template <typename T> class CompareOpEqEps {
    // {{{ open
  public:
    inline T operator()(T val1, T val2, T eps) const {
      return (val1+eps >= val2 && val1-eps <= val2) ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
    }
  };
  // }}}

template <typename T>
  void fallbackCompareEps(const Img<T> *src1, const Img<T> *src2, Img8u *dst,
                       T eps) {
    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
    ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );

    CompareOpEqEps<T> compare;
    for(int c=src1->getChannels()-1; c >= 0; --c) {
      ImgIterator<T> itSrc1 = const_cast<Img<T>*>(src1)->getROIIterator(c);
      ImgIterator<T> itSrc2 = const_cast<Img<T>*>(src2)->getROIIterator(c);
      ImgIterator<icl8u> itDst = dst->getROIIterator(c);
      for(;itSrc1.inRegion(); ++itSrc1, ++itSrc2, ++itDst){
        *itDst = Cast<T,icl8u>::cast(compare(*itSrc1,*itSrc2,eps));
      }
    }
  }
  template <typename T>
  void fallbackCompareEpsC(const Img<T> *src, T value, Img8u *dst,
                        T eps) {
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    CompareOpEqEps<T> compare;
    for(int c=src->getChannels()-1; c >= 0; --c) {
      ImgIterator<T> itSrc = const_cast<Img<T>*>(src)->getROIIterator(c);
      ImgIterator<icl8u> itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
          *itDst = Cast<T,icl8u>::cast(compare(*itSrc,value,eps));
      }
    }
  }


  void Compare::compEqualEps(const Img8u *src1, const Img8u *src2,Img8u *dst, icl8u eps)
  {
    fallbackCompareEps<icl8u>(src1, src2, dst, eps);
  }
  void Compare::compEqualEpsC(const Img8u *src1, icl8u value,Img8u *dst, icl8u eps)
  {
    fallbackCompareEpsC<icl8u>(src1, value, dst, eps);
  }  
// }}}

#ifdef WITH_IPP_OPTIMIZATION

  // {{{ ippi-function call templates


  template <typename T, IppStatus (*ippiFunc) (const T*, int, const T*, int, icl8u*, int, IppiSize, IppCmpOp)>
  inline void ippiCompareCall(const Img<T> *src1, const Img<T> *src2, Img8u *dst, Compare::compareop cmpop)
  {
    // {{{ open
    
    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
    ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );

    for (int c=src1->getChannels()-1; c >= 0; --c) {
      ippiFunc (src1->getROIData (c), src1->getLineStep(),
                src2->getROIData (c), src2->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(), (IppCmpOp) cmpop);
    }
  }
  // }}}
  template <IppStatus (*ippiFunc) (const icl32f*, int, const icl32f*, int, icl8u*, int, IppiSize, icl32f)>
  inline void ippiCompareCallEps(const Img32f *src1, const Img32f *src2, Img8u *dst, icl32f eps)
  {
    // {{{ open

    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
    ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );

    for (int c=src1->getChannels()-1; c >= 0; --c) {
      ippiFunc (src1->getROIData (c), src1->getLineStep(),
                src2->getROIData (c), src2->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(), eps);
    }
  }
  // }}}
 
  template <typename T, IppStatus (*ippiFunc) (const T*, int, T, icl8u*, int, IppiSize, IppCmpOp)>
  inline void ippiCompareCCall(const Img<T> *src, T value, Img8u *dst, Compare::compareop cmpop)
  {
    // {{{ open

    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

    for (int c=src->getChannels()-1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                value,
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(),(IppCmpOp) cmpop);
    }
  }
  //}}}
  template <IppStatus (*ippiFunc) (const icl32f*, int, icl32f, icl8u*, int, IppiSize, icl32f)>
  inline void ippiCompareCallEpsC(const Img32f *src, icl32f value, Img8u *dst, icl32f eps)
  {
    // {{{ open

    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

    for (int c=src->getChannels()-1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                value,
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(),eps);
    }
  }


  // }}}
  // }}}
  // {{{ function specializations without Val postfix

  
  void Compare::comp(const Img8u *src1,const Img8u *src2,Img8u *dst, Compare::compareop cmpop)
  {
    ippiCompareCall<icl8u, ippiCompare_8u_C1R> (src1,src2, dst, cmpop );
  }
   
  void Compare::comp(const Img32f *src1,const Img32f *src2,Img8u *dst, Compare::compareop cmpop)
  {
    ippiCompareCall<icl32f, ippiCompare_32f_C1R> (src1,src2, dst, cmpop);
  }
  void Compare::compEqualEps(const Img32f *src1,const Img32f *src2,Img8u *dst, icl32f eps)
  {
    ippiCompareCallEps<ippiCompareEqualEps_32f_C1R> (src1,src2, dst, eps);
  }
	// }}}
  // {{{ function specializations with Val postfix
   
  void Compare::compC(const Img8u *src, icl8u value, Img8u *dst, Compare::compareop cmpop)
  {
    ippiCompareCCall<icl8u, ippiCompareC_8u_C1R> (src, value, dst, cmpop);
  }
   
  void Compare::compC(const Img32f *src, icl32f value, Img8u *dst, Compare::compareop cmpop)
  {
    ippiCompareCCall<icl32f, ippiCompareC_32f_C1R> (src, value, dst, cmpop);
  }
   
  void Compare::compEqualEpsC(const Img32f *src,icl32f value,Img8u *dst, icl32f eps)
  {
    ippiCompareCallEpsC<ippiCompareEqualEpsC_32f_C1R> (src,value, dst, eps);
  }
  // }}}

#else
  // {{{ C++ fallback CompareOp classes
   
  template <typename T> class CompareOpEq {
    // {{{ open
  public:
    inline T operator()(T val1, T val2) const
    {
      return val1 == val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
    }
  };
  // }}}

  
  template <typename T> class CompareOpLess {
    // {{{ open
  public:
    inline T operator()(T val1, T val2) const
    {
      return val1 < val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
    }
  };
  // }}}

  template <typename T> class CompareOpLessEq 
  {
    // {{{ open
  public:
    inline T operator()(T val1, T val2) const
    {
      return val1 <= val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
    }
  };
  // }}}

  template <typename T> class CompareOpGreater 
  {
    // {{{ open
  public:
    inline T operator()(T val1, T val2) const
    {
      return val1 > val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
    }
  };
  // }}}

  template <typename T> class CompareOpGreaterEq {
    // {{{ open
  public:
    inline T operator()(T val1, T val2) const
    {
      return val1 >= val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
    }
  };
  // }}}
  // }}}
	// {{{ bla
  template <typename T, class CompareOps>
  void fallbackCompare(const Img<T> *src1, const Img<T> *src2, Img8u *dst,
                       const CompareOps &compare)
  {
    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
    ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );
    for(int c=src1->getChannels()-1; c >= 0; --c) {
      ImgIterator<T> itSrc1 = const_cast<Img<T>*>(src1)->getROIIterator(c);
      ImgIterator<T> itSrc2 = const_cast<Img<T>*>(src2)->getROIIterator(c);
      ImgIterator<icl8u> itDst = dst->getROIIterator(c);
      for(;itSrc1.inRegion(); ++itSrc1, ++itSrc2, ++itDst){
        *itDst = Cast<T,icl8u>::cast(compare(*itSrc1,*itSrc2));
      }
    }
  }
  template <typename T, class CompareOps>
  void fallbackCompareC(const Img<T> *src, T value, Img8u *dst,
                        const CompareOps &compare)
  {
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

    for(int c=src->getChannels()-1; c >= 0; --c) {
      ImgIterator<T> itSrc = const_cast<Img<T>*>(src)->getROIIterator(c);
      ImgIterator<icl8u> itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
          *itDst = Cast<T,icl8u>::cast(compare(*itSrc,value));
          //*itDst = Cast<T,icl8u>::cast(CompareOpEq<T>(*itSrc,value));
      }
    }
  }
  
  // }}} bla
	// {{{ lba
  void Compare::comp(const Img8u *src1,const Img8u *src2,Img8u *dst, compareop cmpop)
  {
    switch (cmpop){
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
  void Compare::comp(const Img32f *src1, const Img32f *src2, Img8u *dst, compareop cmpop)
  {
    switch (cmpop){
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
  void Compare::compC(const Img8u *src, icl8u value, Img8u *dst, compareop cmpop)
  {
    switch (cmpop){
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
  void Compare::compC(const Img32f *src, icl32f value, Img8u *dst, compareop cmpop)
  {
    switch (cmpop){
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
  /// short im header
  /** extejkfjd
      dfd
      sdfsd
  */

  void Compare::compEqualEps(const Img32f *src1, const Img32f *src2,Img8u *dst, icl32f eps)
  {
    fallbackCompareEps<icl32f>(src1, src2, dst, eps);
  }
  void Compare::compEqualEpsC(const Img32f *src1, icl32f value,Img8u *dst, icl32f eps)
  {
    fallbackCompareEpsC<icl32f>(src1, value, dst, eps);
  }
  

  // }}} lba

#endif

  // {{{ ImgI* versions
  void Compare::comp(const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst, Compare::compareop cmpop)
  {

    // {{{ open
    ICLASSERT_RETURN(poSrc1 && poSrc2);
    ICLASSERT_RETURN( poSrc1->getROISize() == poSrc2->getROISize() );
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    ensureCompatibleROI (ppoDst, poSrc1);
    if (poSrc1->getDepth () == depth8u)
      comp(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),cmpop);
    else
      comp(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl8u>(),cmpop);
  }
  // }}}

  void Compare::compC(const ImgI *poSrc, icl32f value, ImgI **ppoDst, Compare::compareop cmpop)
  {

    // {{{ open
    ensureCompatibleROI (ppoDst, poSrc);
    if (poSrc->getDepth () == depth8u)
      compC(poSrc->asImg<icl8u>(),Cast<icl32f,icl8u>::cast(value),(*ppoDst)->asImg<icl8u>(),cmpop);
    else
      compC(poSrc->asImg<icl32f>(),value,(*ppoDst)->asImg<icl8u>(),cmpop);
  }

  // }}}

   
  void Compare::compEqualEpsC(const ImgI *poSrc, icl32f value, ImgI **ppoDst, icl32f eps)
  {

    // {{{ open
    ensureCompatibleROI (ppoDst, poSrc);
    if (poSrc->getDepth () == depth8u)
      compEqualEpsC(poSrc->asImg<icl8u>(),Cast<icl32f,icl8u>::cast(value),(*ppoDst)->asImg<icl8u>(),Cast<icl32f,icl8u>::cast(eps));
    else
    compEqualEpsC(poSrc->asImg<icl32f>(),value,(*ppoDst)->asImg<icl8u>(),eps);
  }
  // }}}
  void Compare::compEqualEps(const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst, icl32f eps)
  {

    // {{{ open
    ICLASSERT_RETURN(poSrc1 && poSrc2);
    ICLASSERT_RETURN( poSrc1->getROISize() == poSrc2->getROISize() );
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    ensureCompatibleROI (ppoDst, poSrc1);
    if (poSrc1->getDepth () == depth8u)
       compEqualEps(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),Cast<icl32f,icl8u>::cast(eps));
    else
      compEqualEps(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl8u>(),eps);
  }
  // }}}
// }}}
}
