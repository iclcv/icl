#include <Arithmetic.h>
#include <Img.h>
#include <math.h>
namespace icl {

#ifdef WITH_IPP_OPTIMIZATION

  // {{{ ippi-function call templates

  template <typename T, IppStatus (*ippiFunc) (T*, int, IppiSize)>
  inline void ippi1srcInplaceCall(const Img<T> *srcdst)
  {
    // {{{ open
    ICLASSERT_RETURN( srcdst );
    for (int c=srcdst->getChannels()-1; c >= 0; --c) {
      ippiFunc (srcdst->getROIData (c), srcdst->getLineStep(),
                srcdst->getROISize());
    }
  }
  // }}}
	
	
  template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize)>
  inline void ippi1srcCall(const Img<T> *src, Img<T> *dst)
  {
    // {{{ open
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels()==dst->getChannels());
    for (int c=src->getChannels()-1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize());
    }
  }
  // }}}

  template <typename T, IppStatus (*ippiFunc) (const T*, int, const T*, int, T*, int, IppiSize)>
  inline void ippi2srcCall(const Img<T> *src1, const Img<T> *src2, Img<T> *dst)
  {
    // {{{ open
    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels()==src2->getChannels());
    ICLASSERT_RETURN( src1->getChannels()==dst->getChannels());
    for (int c=src1->getChannels()-1; c >= 0; --c) {
      ippiFunc (src1->getROIData (c), src1->getLineStep(),
                src2->getROIData (c), src2->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize());
    }
  }
  // }}}


  template <typename T, IppStatus (*ippiFunc) (const T*, int, const T, T*, int, IppiSize)>
  inline void ippiCallC(const Img<T> *src, T value, Img<T> *dst)
  {
    // {{{ open
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels()==dst->getChannels());
    for (int c=src->getChannels()-1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                value,
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize());
    }
  }
  template <typename T,typename R, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, R)>
  inline void ippiCallAbsDiffC(const Img<T> *src, Img<T> *dst, R value)
  {
    // {{{ open
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels()==dst->getChannels());
    for (int c=src->getChannels()-1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(),
                value);
    }
  }
  // }}}

  // {{{ function specializations
  void Arithmetic::Add (const Img32f *src1, const Img32f *src2, Img32f *dst){
    ippi2srcCall<icl32f,ippiAdd_32f_C1R>(src1,src2,dst);
  }
  void Arithmetic::Sub (const Img32f *src1, const Img32f *src2, Img32f *dst){
    ippi2srcCall<icl32f,ippiSub_32f_C1R>(src1,src2,dst);
  }
  void Arithmetic::Mul (const Img32f *src1, const Img32f *src2, Img32f *dst){
    ippi2srcCall<icl32f,ippiMul_32f_C1R>(src1,src2,dst);
  }
  void Arithmetic::MulScale (const Img8u  *src1, const Img8u  *src2, Img8u  *dst){
    ippi2srcCall<icl8u,ippiMulScale_8u_C1R>(src1,src2,dst);
  }
  void Arithmetic::Div (const Img32f *src1, const Img32f *src2, Img32f *dst){
    ippi2srcCall<icl32f,ippiDiv_32f_C1R>(src1,src2,dst);
  }
  void Arithmetic::AbsDiff (const Img32f *src1, const Img32f *src2, Img32f *dst){
    ippi2srcCall<icl32f,ippiAbsDiff_32f_C1R>(src1,src2,dst);
  }
  void Arithmetic::AbsDiff (const Img8u *src1, const Img8u *src2, Img8u *dst){
    ippi2srcCall<icl8u,ippiAbsDiff_8u_C1R>(src1,src2,dst);
  }
  void Arithmetic::Sqr (const Img32f *src, Img32f *dst){
    ippi1srcCall<icl32f,ippiSqr_32f_C1R>(src,dst);
  }
  void Arithmetic::Sqrt (const Img32f *src, Img32f *dst){
    ippi1srcCall<icl32f,ippiSqrt_32f_C1R>(src,dst);
  }
  void Arithmetic::Ln (const Img32f *src, Img32f *dst){
    ippi1srcCall<icl32f,ippiLn_32f_C1R>(src,dst);
  }
  void Arithmetic::Exp (const Img32f *src, Img32f *dst){
    ippi1srcCall<icl32f,ippiLn_32f_C1R>(src,dst);
  }
  void Arithmetic::Abs (const Img32f *src, Img32f *dst){
    ippi1srcCall<icl32f,ippiAbs_32f_C1R>(src,dst);
  }
  void Arithmetic::Abs (Img32f *srcdst){
    ippi1srcInplaceCall<icl32f,ippiAbs_32f_C1IR>(srcdst);
  }
  void Arithmetic::AddC (const Img32f *src, const icl32f value, Img32f *dst){
    ippiCallC<icl32f,ippiAddC_32f_C1R>(src,value,dst);
  }
  void Arithmetic::SubC (const Img32f *src, const icl32f value, Img32f *dst){
    ippiCallC<icl32f,ippiSubC_32f_C1R>(src,value,dst);
  }
  void Arithmetic::MulC (const Img32f *src, const icl32f value, Img32f *dst){
    ippiCallC<icl32f,ippiMulC_32f_C1R>(src,value,dst);
  }
  void Arithmetic::MulCScale (const Img8u *src, const icl8u value, Img8u *dst){
    ippiCallC<icl8u,ippiMulCScale_8u_C1R>(src,value,dst);
  }
  void Arithmetic::DivC (const Img32f *src, const icl32f value, Img32f *dst){
    ippiCallC<icl32f,ippiDivC_32f_C1R>(src,value,dst);
  }
  void Arithmetic::AbsDiffC (const Img8u *src, int value, Img8u *dst){
    ippiCallAbsDiffC<icl8u,int,ippiAbsDiffC_8u_C1R>(src,dst,value);
  }
  void Arithmetic::AbsDiffC (const Img32f *src, icl32f value, Img32f *dst){
    ippiCallAbsDiffC<icl32f,icl32f,ippiAbsDiffC_32f_C1R>(src,dst,value);
  }
  // }}}

#else

  // not implemented functions

  void Arithmetic::MulCScale (const Img8u *src, const icl8u value, Img8u *dst){
    #warning "MulCScale is not yet implemented without IPP optimization";
  }
  void Arithmetic::MulScale (const Img8u  *src1, const Img8u  *src2, Img8u  *dst){
    #warning "MulScale is not yet implemented without IPP optimization";
  }
  // {{{ C++ fallback functions

  template <typename T,class ArithmeticOp>
  void fallbackArithmetic2T(const Img<T> *src1, const Img<T> *src2, Img<T> *dst,const ArithmeticOp &op)
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
      ImgIterator<T> itDst = dst->getROIIterator(c);
      for(;itSrc1.inRegion(); ++itSrc1, ++itSrc2, ++itDst){
        *itDst = op(*itSrc1,*itSrc2);
      }
    }
  }
  // }}}

  template <typename T,class ArithmeticOp>
  void fallbackArithmetic2TC(const Img<T> *src, const T value, Img<T> *dst,const ArithmeticOp &op)
    // {{{ open
  {
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    for(int c=src->getChannels()-1; c >= 0; --c) {
      ConstImgIterator<T> itSrc = src->getROIIterator(c);
      ImgIterator<T>      itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = op(*itSrc,value);
      }
    }
  }
  // }}}


  template <typename T,class ArithmeticOp>
  void fallbackArithmetic1T(const Img<T> *src, Img<T> *dst,const ArithmeticOp &op)
    // {{{ open
  {
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    for(int c=src->getChannels()-1; c >= 0; --c) {
      ConstImgIterator<T> itSrc = src->getROIIterator(c);
      ImgIterator<T> itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = op(*itSrc);
      }
    }
  }
  // }}}

  // }}}

  // {{{ C++ fallback Logical Operators

  template <typename T> class AddOp {
  public:
    inline T operator()(T val1,T val2) const { return val1 + val2; }
  };

  template <typename T> class SubOp {
  public:
    inline T operator()(T val1,T val2) const { return val1 - val2; }
  };
  template <typename T> class MulOp {
  public:
    inline T operator()(T val1,T val2) const { return val1 * val2; }
  };
  template <typename T> class DivOp {
  public:
    inline T operator()(T val1,T val2) const { return val1 / val2; }
  };
  template <typename T> class SqrOp {
  public:
    inline T operator()(T val) const { return val*val; }
  };
  template <typename T> class SqrtOp {
  public:
    inline T operator()(T val) const { return sqrt(val); }
  };
  template <typename T> class LnOp {
  public:
    inline T operator()(T val) const { return log(val); }
  };
  template <typename T> class ExpOp {
  public:
    inline T operator()(T val) const { return exp(val); }
  };
  template <typename T> class AbsOp {
  public:
    inline T operator()(T val) const { return fabs(val); }
  };


  
  
  template <typename T> class AbsDiffOp_8u {
  public:
    inline T operator()(T val1,T val2) const { return abs(val1 - val2); }
  };
  template <typename T> class AbsDiffOp_32f {
  public:
    inline T operator()(T val1,T val2) const { return fabs(val1 - val2); }
  };
  
  // }}}

  // {{{ C++ fallback function specializations
  void Arithmetic::Add (const Img32f *src1, const Img32f *src2, Img32f *dst)
  {
    fallbackArithmetic2T<icl32f>(src1, src2,dst,AddOp<icl32f>());
  }
  void Arithmetic::Sub (const Img32f *src1, const Img32f *src2, Img32f *dst)
  {
    fallbackArithmetic2T<icl32f>(src1, src2,dst,SubOp<icl32f>());
  }
  void Arithmetic::Mul (const Img32f *src1, const Img32f *src2, Img32f *dst)
  {
    fallbackArithmetic2T<icl32f>(src1, src2,dst,MulOp<icl32f>());
  }
  void Arithmetic::Div (const Img32f *src1, const Img32f *src2, Img32f *dst)
  {
    fallbackArithmetic2T<icl32f>(src1, src2,dst,DivOp<icl32f>());
  }
  
  void Arithmetic::AddC (const Img32f *src, const icl32f value, Img32f *dst)
  {
    fallbackArithmetic2TC<icl32f>(src, value,dst,AddOp<icl32f>());
  }
  void Arithmetic::SubC (const Img32f *src, const icl32f value, Img32f *dst)
  {
    fallbackArithmetic2TC<icl32f>(src, value,dst,SubOp<icl32f>());
  }
  void Arithmetic::MulC (const Img32f *src, const icl32f value, Img32f *dst)
  {
    fallbackArithmetic2TC<icl32f>(src, value,dst,MulOp<icl32f>());
  }
  void Arithmetic::DivC (const Img32f *src, const icl32f value, Img32f *dst)
  {
    fallbackArithmetic2TC<icl32f>(src, value,dst,DivOp<icl32f>());
  }
  void Arithmetic::Sqr (const Img32f *src, Img32f *dst)
  {
    fallbackArithmetic1T<icl32f>(src, dst,SqrOp<icl32f>());
  }
  void Arithmetic::Sqrt (const Img32f *src, Img32f *dst)
  {
    fallbackArithmetic1T<icl32f>(src, dst,SqrtOp<icl32f>());
  }

  void Arithmetic::Exp (const Img32f *src, Img32f *dst)
  {
    fallbackArithmetic1T<icl32f>(src, dst,ExpOp<icl32f>());
  }
  void Arithmetic::Ln (const Img32f *src, Img32f *dst)
  {
    fallbackArithmetic1T<icl32f>(src, dst,LnOp<icl32f>());
  }
  void Arithmetic::Abs (const Img32f *src, Img32f *dst)
  {
    fallbackArithmetic1T<icl32f>(src, dst,AbsOp<icl32f>());
  }
	void Arithmetic::Abs (Img32f *srcdst)
  {
    fallbackArithmetic1T<icl32f>(srcdst, srcdst,AbsOp<icl32f>());
  }
  void Arithmetic::AbsDiff (const Img32f *src1, const Img32f *src2, Img32f *dst){
    fallbackArithmetic2T<icl32f>(src1, src2,dst,AbsDiffOp_32f<icl32f>());
  }
  void Arithmetic::AbsDiffC (const Img32f *src, const icl32f value, Img32f *dst)
  {
    fallbackArithmetic2TC<icl32f>(src, value,dst,AbsDiffOp_32f<icl32f>());
  }
  void Arithmetic::AbsDiff (const Img8u *src1, const Img8u *src2, Img8u *dst){
    fallbackArithmetic2T<icl8u>(src1, src2,dst,AbsDiffOp_8u<icl8u>());
  }
  void Arithmetic::AbsDiffC (const Img8u *src, const int value, Img8u *dst)
  {
    fallbackArithmetic2TC<icl8u>(src, value,dst,AbsDiffOp_8u<icl8u>());
  }
  // }}}

#endif

  // {{{ ImgBase* versions
  void Arithmetic::Add (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    Add(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
  // }}}  
  void Arithmetic::Sub (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    Sub(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
  // }}}
  void Arithmetic::Mul (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    Mul(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
  // }}}
  void Arithmetic::Div (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    Div(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
  // }}}


  void Arithmetic::AddC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    AddC(poSrc->asImg<icl32f>(),value,(*ppoDst)->asImg<icl32f>());
  }
  // }}}
  void Arithmetic::SubC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    SubC(poSrc->asImg<icl32f>(),value,(*ppoDst)->asImg<icl32f>());
  }
  // }}}
  void Arithmetic::MulC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    MulC(poSrc->asImg<icl32f>(),value,(*ppoDst)->asImg<icl32f>());
  }
  // }}}

  void Arithmetic::DivC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    DivC(poSrc->asImg<icl32f>(),value,(*ppoDst)->asImg<icl32f>());
  }
  // }}}



  void Arithmetic::MulScale (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getDepth() == depth8u);
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    MulScale(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
  }
  // }}}


  void Arithmetic::MulCScale (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth8u);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    MulCScale(poSrc->asImg<icl8u>(),Cast<icl32f,icl8u>::cast(value),(*ppoDst)->asImg<icl8u>());
  }
  // }}}


  void Arithmetic::Sqr (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    Sqr(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
  // }}}

  void Arithmetic::Sqrt (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    Sqrt(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
  // }}}

  void Arithmetic::Ln (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    Ln(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
  // }}}

  void Arithmetic::Exp (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    Exp(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
  // }}}

  void Arithmetic::Abs (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f);
    if (!Filter::prepare (ppoDst, poSrc)) return;
    Abs(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
  void Arithmetic::Abs (ImgBase *poSrcDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrcDst);
    Abs(poSrcDst->asImg<icl32f>());
  }
  // }}}

  void Arithmetic::AbsDiff (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    switch (poSrc1->getDepth()) {
      case depth8u: AbsDiff(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: AbsDiff(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    };
  }
  // }}}

  void Arithmetic::AbsDiffC (const ImgBase *poSrc, icl32f value, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      case depth8u: AbsDiffC(poSrc->asImg<icl8u>(),Cast<icl32f,int>::cast(value),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: AbsDiffC(poSrc->asImg<icl32f>(),value,(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    };
  }
  // }}}

// }}}
}
