#include <Arithmetic.h>
#include <Img.h>
#include <math.h>
namespace icl {

  
  
//Define Fallback also when IPP is - needed for datatypes, that are not supported by IPP  
  
  // {{{ C++ fallback functions

  template <typename T,class ArithmeticOp>
  inline void fallbackArithmetic2T(const Img<T> *src1, const Img<T> *src2, Img<T> *dst,const ArithmeticOp &op)
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
  inline void fallbackArithmetic2TC(const Img<T> *src, const T value, Img<T> *dst,const ArithmeticOp &op)
    // {{{ open
  {
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    for(int c=src->getChannels()-1; c >= 0; --c) {
      ConstImgIterator<T> itSrc = src->getROIIterator(c);
      ImgIterator<T> itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = op(*itSrc,value);
      }
    }
  }
  // }}}


  template <typename T,class ArithmeticOp>
  inline void fallbackArithmetic1T(const Img<T> *src, Img<T> *dst,const ArithmeticOp &op)
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
    inline T operator()(T val) const { return (T)sqrt(val); } //TODO?? const_cast<T>(sqrt(val)
  };
  template <typename T> class LnOp {
  public:
    inline T operator()(T val) const { return (T)log(val); } //TODO?? const_cast<T>
  };
  template <typename T> class ExpOp {
  public:
    inline T operator()(T val) const { return (T)exp(val); } //TODO?? const_cast<T>
  };
  template <typename T> class fAbsOp {
  public:
    inline T operator()(T val) const { return fabs(val); }
  };
  template <typename T> class AbsOp {
  public:
    inline T operator()(T val) const { return abs(val); }
  };


  
  
  template <typename T> class AbsDiffOp {
  public:
    inline T operator()(T val1,T val2) const { return abs(val1 - val2); }
  };
  template <typename T> class fAbsDiffOp {
  public:
    inline T operator()(T val1,T val2) const { return fabs(val1 - val2); }
  };
  
  // }}}
  
  
  
  
  
  
  
  
  
  
  
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

  
  template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize,int)>
  inline void ippi1srcCall_sc(const Img<T> *src, Img<T> *dst)
  {
    // {{{ open
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels()==dst->getChannels());
    for (int c=src->getChannels()-1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(), 0);
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

  template <typename T, IppStatus (*ippiFunc) (const T*, int, const T*, int, T*, int, IppiSize,int)>
  inline void ippi2srcCall_sc(const Img<T> *src1, const Img<T> *src2, Img<T> *dst)
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
                dst->getROISize(),0);
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
  
   template <typename T, IppStatus (*ippiFunc) (const T*, int, const T, T*, int, IppiSize,int)>
  inline void ippiCallC_sc(const Img<T> *src, T value, Img<T> *dst)
  {
    // {{{ open
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels()==dst->getChannels());
    for (int c=src->getChannels()-1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                value,
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(),0);
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
  void Arithmetic::Add (const Img8u *src1, const Img8u *src2, Img8u *dst){
    ippi2srcCall_sc<icl8u,ippiAdd_8u_C1RSfs>(src1,src2,dst);
  }
  void Arithmetic::Add (const Img16s *src1, const Img16s *src2, Img16s *dst){
    ippi2srcCall_sc<icl16s,ippiAdd_16s_C1RSfs>(src1,src2,dst);
  }  
  void Arithmetic::Add (const Img32s *src1, const Img32s *src2, Img32s *dst){
    fallbackArithmetic2T<icl32s>(src1, src2,dst,AddOp<icl32s>());
  }
  void Arithmetic::Add (const Img32f *src1, const Img32f *src2, Img32f *dst){
    ippi2srcCall<icl32f,ippiAdd_32f_C1R>(src1,src2,dst);
  }  
  void Arithmetic::Add (const Img64f *src1, const Img64f *src2, Img64f *dst){
    fallbackArithmetic2T<icl64f>(src1, src2,dst,AddOp<icl64f>());
  }
  
  void Arithmetic::Sub (const Img8u *src1, const Img8u *src2, Img8u *dst){
    ippi2srcCall_sc<icl8u,ippiSub_8u_C1RSfs>(src1,src2,dst);
  }
  void Arithmetic::Sub (const Img16s *src1, const Img16s *src2, Img16s *dst){
    ippi2srcCall_sc<icl16s,ippiSub_16s_C1RSfs>(src1,src2,dst);
  }  
  void Arithmetic::Sub (const Img32s *src1, const Img32s *src2, Img32s *dst){
    fallbackArithmetic2T<icl32s>(src1, src2,dst,SubOp<icl32s>());
  }
  void Arithmetic::Sub (const Img32f *src1, const Img32f *src2, Img32f *dst){
    ippi2srcCall<icl32f,ippiSub_32f_C1R>(src1,src2,dst);
  }  
  void Arithmetic::Sub (const Img64f *src1, const Img64f *src2, Img64f *dst){
    fallbackArithmetic2T<icl64f>(src1, src2,dst,SubOp<icl64f>());
  }

  void Arithmetic::Mul (const Img8u *src1, const Img8u *src2, Img8u *dst){
    ippi2srcCall_sc<icl8u,ippiMul_8u_C1RSfs>(src1,src2,dst);
  }
  void Arithmetic::Mul (const Img16s *src1, const Img16s *src2, Img16s *dst){
    ippi2srcCall_sc<icl16s,ippiMul_16s_C1RSfs>(src1,src2,dst);
  }  
  void Arithmetic::Mul (const Img32s *src1, const Img32s *src2, Img32s *dst){
    fallbackArithmetic2T<icl32s>(src1, src2,dst,MulOp<icl32s>());
  }
  void Arithmetic::Mul (const Img32f *src1, const Img32f *src2, Img32f *dst){
    ippi2srcCall<icl32f,ippiMul_32f_C1R>(src1,src2,dst);
  }  
  void Arithmetic::Mul (const Img64f *src1, const Img64f *src2, Img64f *dst){
    fallbackArithmetic2T<icl64f>(src1, src2,dst,MulOp<icl64f>());
  }



  void Arithmetic::MulScale (const Img8u  *src1, const Img8u  *src2, Img8u  *dst){
    ippi2srcCall<icl8u,ippiMulScale_8u_C1R>(src1,src2,dst);
  }
  void Arithmetic::Div (const Img8u *src1, const Img8u *src2, Img8u *dst){
    ippi2srcCall_sc<icl8u,ippiDiv_8u_C1RSfs>(src1,src2,dst);
  }
  void Arithmetic::Div (const Img16s *src1, const Img16s *src2, Img16s *dst){
    ippi2srcCall_sc<icl16s,ippiDiv_16s_C1RSfs>(src1,src2,dst);
  }  
  void Arithmetic::Div (const Img32s *src1, const Img32s *src2, Img32s *dst){
    fallbackArithmetic2T<icl32s>(src1, src2,dst,DivOp<icl32s>());
  }
  void Arithmetic::Div (const Img32f *src1, const Img32f *src2, Img32f *dst){
    ippi2srcCall<icl32f,ippiDiv_32f_C1R>(src1,src2,dst);
  }  
  void Arithmetic::Div (const Img64f *src1, const Img64f *src2, Img64f *dst){
    fallbackArithmetic2T<icl64f>(src1, src2,dst,DivOp<icl64f>());
  }




  void Arithmetic::AbsDiff (const Img8u *src1, const Img8u *src2, Img8u *dst){
    ippi2srcCall<icl8u,ippiAbsDiff_8u_C1R>(src1,src2,dst);
  }
  void Arithmetic::AbsDiff (const Img16s *src1, const Img16s *src2, Img16s *dst){
    fallbackArithmetic2T<icl16s>(src1, src2,dst,AbsDiffOp<icl16s>());
  }
  void Arithmetic::AbsDiff (const Img32s *src1, const Img32s *src2, Img32s *dst){
    fallbackArithmetic2T<icl32s>(src1, src2,dst,AbsDiffOp<icl32s>());
  }
  void Arithmetic::AbsDiff (const Img32f *src1, const Img32f *src2, Img32f *dst){
      ippi2srcCall<icl32f,ippiAbsDiff_32f_C1R>(src1,src2,dst);
  }
  void Arithmetic::AbsDiff (const Img64f *src1, const Img64f *src2, Img64f *dst){
    fallbackArithmetic2T<icl64f>(src1, src2,dst,fAbsDiffOp<icl64f>());
  }
  
  void Arithmetic::AbsDiffC (const Img8u *src, const int value, Img8u *dst){
    ippiCallAbsDiffC<icl8u,int,ippiAbsDiffC_8u_C1R>(src,dst,value);
  }
  void Arithmetic::AbsDiffC (const Img16s *src, const int value, Img16s *dst)
  {
    fallbackArithmetic2TC<icl16s>(src, value,dst,AbsDiffOp<icl16s>());
  }
  void Arithmetic::AbsDiffC (const Img32s *src, const int value, Img32s *dst)
  {
    fallbackArithmetic2TC<icl32s>(src, value,dst,AbsDiffOp<icl32s>());
  }
  void Arithmetic::AbsDiffC (const Img32f *src, const icl32f value, Img32f *dst){
    ippiCallAbsDiffC<icl32f,icl32f,ippiAbsDiffC_32f_C1R>(src,dst,value);
  }
  void Arithmetic::AbsDiffC (const Img64f *src, const icl64f value, Img64f *dst)
  {
    fallbackArithmetic2TC<icl64f>(src, value,dst,fAbsDiffOp<icl64f>());
  }

  void Arithmetic::Sqr (const Img8u *src, Img8u *dst){
    ippi1srcCall_sc<icl8u,ippiSqr_8u_C1RSfs>(src,dst);
  }
  void Arithmetic::Sqr (const Img16s *src, Img16s *dst){
    ippi1srcCall_sc<icl16s,ippiSqr_16s_C1RSfs>(src,dst);
  }
  void Arithmetic::Sqr (const Img32s *src, Img32s *dst){
    fallbackArithmetic1T<icl32s>(src,dst,SqrOp<icl32s>());
  }
  void Arithmetic::Sqr (const Img32f *src, Img32f *dst){
    ippi1srcCall<icl32f,ippiSqr_32f_C1R>(src,dst);
  }
  void Arithmetic::Sqr (const Img64f *src, Img64f *dst){
    fallbackArithmetic1T<icl64f>(src,dst,SqrOp<icl64f>());
  }


  void Arithmetic::Sqrt (const Img8u *src, Img8u *dst){
    ippi1srcCall_sc<icl8u,ippiSqrt_8u_C1RSfs>(src,dst);
  }
  void Arithmetic::Sqrt (const Img16s *src, Img16s *dst){
    ippi1srcCall_sc<icl16s,ippiSqrt_16s_C1RSfs>(src,dst);
  }
  void Arithmetic::Sqrt (const Img32s *src, Img32s *dst){
    fallbackArithmetic1T<icl32s>(src,dst,SqrtOp<icl32s>());
  }
  void Arithmetic::Sqrt (const Img32f *src, Img32f *dst){
    ippi1srcCall<icl32f,ippiSqrt_32f_C1R>(src,dst);
  }
  void Arithmetic::Sqrt (const Img64f *src, Img64f *dst){
    fallbackArithmetic1T<icl64f>(src,dst,SqrtOp<icl64f>());
  }

  void Arithmetic::Exp (const Img8u *src, Img8u *dst){
    ippi1srcCall_sc<icl8u,ippiExp_8u_C1RSfs>(src,dst);
  }
  void Arithmetic::Exp (const Img16s *src, Img16s *dst){
    ippi1srcCall_sc<icl16s,ippiExp_16s_C1RSfs>(src,dst);
  }
  void Arithmetic::Exp (const Img32s *src, Img32s *dst){
    fallbackArithmetic1T<icl32s>(src,dst,ExpOp<icl32s>());
  }
  void Arithmetic::Exp (const Img32f *src, Img32f *dst){
    ippi1srcCall<icl32f,ippiExp_32f_C1R>(src,dst);
  }
  void Arithmetic::Exp (const Img64f *src, Img64f *dst){
    fallbackArithmetic1T<icl64f>(src,dst,ExpOp<icl64f>());
  }
  
  
  void Arithmetic::Ln (const Img8u *src, Img8u *dst){
    ippi1srcCall_sc<icl8u,ippiLn_8u_C1RSfs>(src,dst);
  }
  void Arithmetic::Ln (const Img16s *src, Img16s *dst){
    ippi1srcCall_sc<icl16s,ippiLn_16s_C1RSfs>(src,dst);
  }
  void Arithmetic::Ln (const Img32s *src, Img32s *dst){
    fallbackArithmetic1T<icl32s>(src,dst,LnOp<icl32s>());
  }
  void Arithmetic::Ln (const Img32f *src, Img32f *dst){
    ippi1srcCall<icl32f,ippiLn_32f_C1R>(src,dst);
  }
  void Arithmetic::Ln (const Img64f *src, Img64f *dst){
    fallbackArithmetic1T<icl64f>(src,dst,LnOp<icl64f>());
  }

  void Arithmetic::Abs (const Img16s *src, Img16s *dst){
    ippi1srcCall<icl16s,ippiAbs_16s_C1R>(src,dst);
  }
  void Arithmetic::Abs (const Img32s *src, Img32s *dst)
  {
    fallbackArithmetic1T<icl32s>(src, dst,AbsOp<icl32s>());
  }
  void Arithmetic::Abs (const Img32f *src, Img32f *dst){
    ippi1srcCall<icl32f,ippiAbs_32f_C1R>(src,dst);
  }
  void Arithmetic::Abs (const Img64f *src, Img64f *dst)
  {
    fallbackArithmetic1T<icl64f>(src, dst,fAbsOp<icl64f>());
  }
  
  
  void Arithmetic::Abs (Img16s *srcdst){
    ippi1srcInplaceCall<icl16s,ippiAbs_16s_C1IR>(srcdst);
  }
	void Arithmetic::Abs (Img32s *srcdst)
  {
    fallbackArithmetic1T<icl32s>(srcdst, srcdst,AbsOp<icl32s>());
  }
  void Arithmetic::Abs (Img32f *srcdst){
    ippi1srcInplaceCall<icl32f,ippiAbs_32f_C1IR>(srcdst);
  }
  void Arithmetic::Abs (Img64f *srcdst)
  {
    fallbackArithmetic1T<icl64f>(srcdst, srcdst,fAbsOp<icl64f>());
  }


  
  
  
  void Arithmetic::AddC (const Img8u *src, const icl8u value, Img8u *dst){
    ippiCallC_sc<icl8u,ippiAddC_8u_C1RSfs>(src,value,dst);
  }
  void Arithmetic::AddC (const Img16s *src, const icl16s value, Img16s *dst){
    ippiCallC_sc<icl16s,ippiAddC_16s_C1RSfs>(src,value,dst);
  }
  void Arithmetic::AddC (const Img32s *src, const icl32s value, Img32s *dst){
    fallbackArithmetic2TC<icl32s>(src, value,dst,AddOp<icl32s>());
  }
  void Arithmetic::AddC (const Img32f *src, const icl32f value, Img32f *dst){
    ippiCallC<icl32f,ippiAddC_32f_C1R>(src,value,dst);
  }
  void Arithmetic::AddC (const Img64f *src, const icl64f value, Img64f *dst){
        fallbackArithmetic2TC<icl64f>(src, value,dst,AddOp<icl64f>());
  }



  void Arithmetic::SubC (const Img8u *src, const icl8u value, Img8u *dst){
    ippiCallC_sc<icl8u,ippiSubC_8u_C1RSfs>(src,value,dst);
  }
  void Arithmetic::SubC (const Img16s *src, const icl16s value, Img16s *dst){
    ippiCallC_sc<icl16s,ippiSubC_16s_C1RSfs>(src,value,dst);
  }
  void Arithmetic::SubC (const Img32s *src, const icl32s value, Img32s *dst){
    fallbackArithmetic2TC<icl32s>(src, value,dst,SubOp<icl32s>());
  }
  void Arithmetic::SubC (const Img32f *src, const icl32f value, Img32f *dst){
    ippiCallC<icl32f,ippiSubC_32f_C1R>(src,value,dst);
  }
  void Arithmetic::SubC (const Img64f *src, const icl64f value, Img64f *dst){
        fallbackArithmetic2TC<icl64f>(src, value,dst,SubOp<icl64f>());
  }
  
  
  
  void Arithmetic::MulC (const Img8u *src, const icl8u value, Img8u *dst){
    ippiCallC_sc<icl8u,ippiMulC_8u_C1RSfs>(src,value,dst);
  }
  void Arithmetic::MulC (const Img16s *src, const icl16s value, Img16s *dst){
    ippiCallC_sc<icl16s,ippiMulC_16s_C1RSfs>(src,value,dst);
  }
  void Arithmetic::MulC (const Img32s *src, const icl32s value, Img32s *dst){
    fallbackArithmetic2TC<icl32s>(src, value,dst,MulOp<icl32s>());
  }
  void Arithmetic::MulC (const Img32f *src, const icl32f value, Img32f *dst){
    ippiCallC<icl32f,ippiMulC_32f_C1R>(src,value,dst);
  }
  void Arithmetic::MulC (const Img64f *src, const icl64f value, Img64f *dst){
        fallbackArithmetic2TC<icl64f>(src, value,dst,MulOp<icl64f>());
  }
  
  void Arithmetic::DivC (const Img8u *src, const icl8u value, Img8u *dst){
    ippiCallC_sc<icl8u,ippiDivC_8u_C1RSfs>(src,value,dst);
  }
  void Arithmetic::DivC (const Img16s *src, const icl16s value, Img16s *dst){
    ippiCallC_sc<icl16s,ippiDivC_16s_C1RSfs>(src,value,dst);
  }
  void Arithmetic::DivC (const Img32s *src, const icl32s value, Img32s *dst){
    fallbackArithmetic2TC<icl32s>(src, value,dst,DivOp<icl32s>());
  }
  void Arithmetic::DivC (const Img32f *src, const icl32f value, Img32f *dst){
    ippiCallC<icl32f,ippiDivC_32f_C1R>(src,value,dst);
  }
  void Arithmetic::DivC (const Img64f *src, const icl64f value, Img64f *dst){
        fallbackArithmetic2TC<icl64f>(src, value,dst,DivOp<icl64f>());
  }
  
  
  void Arithmetic::MulCScale (const Img8u *src, const icl8u value, Img8u *dst){
    ippiCallC<icl8u,ippiMulCScale_8u_C1R>(src,value,dst);
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

  // {{{ C++ fallback function specializations


#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Add (const Img ## T *src1, const Img ## T *src2, Img ## T *dst) {\
    fallbackArithmetic2T<icl ## T>(src1, src2,dst,AddOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Sub (const Img ## T *src1, const Img ## T *src2, Img ## T *dst) {\
    fallbackArithmetic2T<icl ## T>(src1, src2,dst,SubOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Mul (const Img ## T *src1, const Img ## T *src2, Img ## T *dst) {\
    fallbackArithmetic2T<icl ## T>(src1, src2,dst,MulOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Div (const Img ## T *src1, const Img ## T *src2, Img ## T *dst) {\
    fallbackArithmetic2T<icl ## T>(src1, src2,dst,DivOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
  
#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::AddC (const Img ## T *src, const icl ## T value, Img ## T *dst){\
    fallbackArithmetic2TC<icl ## T>(src, value,dst,AddOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::SubC (const Img ## T *src, const icl ## T value, Img ## T *dst){\
    fallbackArithmetic2TC<icl ## T>(src, value,dst,SubOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::MulC (const Img ## T *src, const icl ## T value, Img ## T *dst){\
    fallbackArithmetic2TC<icl ## T>(src, value,dst,MulOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::DivC (const Img ## T *src, const icl ## T value, Img ## T *dst){\
    fallbackArithmetic2TC<icl ## T>(src, value,dst,DivOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

  
#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Sqr (const Img ## T *src, Img ## T *dst){\
    fallbackArithmetic1T<icl ## T>(src,dst,SqrOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Sqrt (const Img ## T *src, Img ## T *dst){\
    fallbackArithmetic1T<icl ## T>(src,dst,SqrtOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Exp (const Img ## T *src, Img ## T *dst){\
    fallbackArithmetic1T<icl ## T>(src,dst,ExpOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Ln (const Img ## T *src, Img ## T *dst){\
    fallbackArithmetic1T<icl ## T>(src,dst,LnOp<icl ## T>());}
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
  
  
#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Abs (const Img ## T *src, Img ## T *dst){ \
    fallbackArithmetic1T<icl ## T>(src, dst,AbsOp<icl ## T>());}

  ICL_INSTANTIATE_DEPTH(16s)
  ICL_INSTANTIATE_DEPTH(32s)
#undef ICL_INSTANTIATE_DEPTH



#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Abs (const Img ## T *src, Img ## T *dst){ \
    fallbackArithmetic1T<icl ## T>(src, dst,fAbsOp<icl ## T>());}
  ICL_INSTANTIATE_DEPTH(32f)
  ICL_INSTANTIATE_DEPTH(64f)
#undef ICL_INSTANTIATE_DEPTH


#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Abs (Img ## T *srcdst){\
    fallbackArithmetic1T<icl ## T>(srcdst, srcdst,AbsOp<icl ## T>());}
  ICL_INSTANTIATE_DEPTH(16s)
  ICL_INSTANTIATE_DEPTH(32s)
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::Abs (Img ## T *srcdst){\
    fallbackArithmetic1T<icl ## T>(srcdst, srcdst,fAbsOp<icl ## T>());}  
  ICL_INSTANTIATE_DEPTH(32f)
  ICL_INSTANTIATE_DEPTH(64f)
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::AbsDiff (const Img ## T *src1, const Img ## T *src2, Img ## T *dst){\
    fallbackArithmetic2T<icl ## T>(src1, src2,dst,AbsDiffOp<icl ## T>());\
  }
  ICL_INSTANTIATE_DEPTH(8u)
  ICL_INSTANTIATE_DEPTH(16s)
  ICL_INSTANTIATE_DEPTH(32s)
#undef ICL_INSTANTIATE_DEPTH
#define ICL_INSTANTIATE_DEPTH(T) \
  void Arithmetic::AbsDiff (const Img ## T *src1, const Img ## T *src2, Img ## T *dst){\
    fallbackArithmetic2T<icl ## T>(src1, src2,dst,fAbsDiffOp<icl ## T>());\
  }
  ICL_INSTANTIATE_DEPTH(32f)
  ICL_INSTANTIATE_DEPTH(64f)
#undef ICL_INSTANTIATE_DEPTH


  
  
  void Arithmetic::AbsDiffC (const Img8u *src, const int value, Img8u *dst)
  {
    fallbackArithmetic2TC<icl8u>(src, value,dst,AbsDiffOp<icl8u>());
  }
  void Arithmetic::AbsDiffC (const Img16s *src, const int value, Img16s *dst)
  {
    fallbackArithmetic2TC<icl16s>(src, value,dst,AbsDiffOp<icl16s>());
  }
  void Arithmetic::AbsDiffC (const Img32s *src, const int value, Img32s *dst)
  {
    fallbackArithmetic2TC<icl32s>(src, value,dst,AbsDiffOp<icl32s>());
  }
  void Arithmetic::AbsDiffC (const Img32f *src, const icl32f value, Img32f *dst)
  {
    fallbackArithmetic2TC<icl32f>(src, value,dst,fAbsDiffOp<icl32f>());
  }
  void Arithmetic::AbsDiffC (const Img64f *src, const icl64f value, Img64f *dst)
  {
    fallbackArithmetic2TC<icl64f>(src, value,dst,fAbsDiffOp<icl64f>());
  }
  // }}}

#endif

  // {{{ ImgBase* versions

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: Add(poSrc1->asImg<icl ## T>(),poSrc2->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::Add (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
//    ICLASSERT_RETURN( poSrc1->getDepth() == depth32f);

    if (!Filter::prepare (ppoDst, poSrc1)) return;
    switch (poSrc1->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS      
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: Sub(poSrc1->asImg<icl ## T>(),poSrc2->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::Sub (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    switch (poSrc1->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: Mul(poSrc1->asImg<icl ## T>(),poSrc2->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::Mul (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    switch (poSrc1->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: Div(poSrc1->asImg<icl ## T>(),poSrc2->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::Div (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    switch (poSrc1->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
  #undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: AddC(poSrc->asImg<icl ## T>(),Cast<icl64f,icl ## T>::cast(value),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::AddC (const ImgBase *poSrc, const icl64f value, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: SubC(poSrc->asImg<icl ## T>(),Cast<icl64f,icl ## T>::cast(value),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::SubC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: MulC(poSrc->asImg<icl ## T>(),Cast<icl64f,icl ## T>::cast(value),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::MulC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: DivC(poSrc->asImg<icl ## T>(),Cast<icl64f,icl ## T>::cast(value),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::DivC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
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

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: Sqr(poSrc->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::Sqr (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: Sqrt(poSrc->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::Sqrt (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}
  
#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: Exp(poSrc->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::Exp (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}
  
#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: Ln(poSrc->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::Ln (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: Abs(poSrc->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Arithmetic::Abs (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_DEPTH(16s)
      ICL_INSTANTIATE_DEPTH(32s)
      ICL_INSTANTIATE_DEPTH(32f)
      ICL_INSTANTIATE_DEPTH(64f)
      default: ICL_INVALID_FORMAT; break;
    }    
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}
  
#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: Abs(poSrcDst->asImg<icl ## T>()); break;  
  void Arithmetic::Abs (ImgBase *poSrcDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrcDst);
    switch (poSrcDst->getDepth()) {
      case depth8u: break; //unsigned, nothing to do
      ICL_INSTANTIATE_DEPTH(16s)
      ICL_INSTANTIATE_DEPTH(32s)
      ICL_INSTANTIATE_DEPTH(32f)
      ICL_INSTANTIATE_DEPTH(64f)
      default: ICL_INVALID_FORMAT; break;
    }
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}
  
#define ICL_INSTANTIATE_DEPTH(T) \
  case depth ## T: AbsDiff(poSrc1->asImg<icl ## T>(),poSrc2->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  
  void Arithmetic::AbsDiff (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    switch (poSrc1->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
     case depth ## T: AbsDiffC(poSrc->asImg<icl ## T>(),Cast<icl64f,icl ## T>::cast(value),(*ppoDst)->asImg<icl ## T>()); break;
 
  void Arithmetic::AbsDiffC (const ImgBase *poSrc, icl64f value, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH  
  // }}}
}
// }}}
