#include <iclLogical.h>
#include <iclImg.h>

namespace icl {


// {{{ C++ fallback functions

  template <typename T>
  inline void fallbackLogicalNot(const Img<T> *src, Img<T> *dst)
    // {{{ open
  {
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    for(int c=src->getChannels()-1; c >= 0; --c) {
      ConstImgIterator<T> itSrc = src->getROIIterator(c);
      ImgIterator<T>      itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = ~*itSrc ;
      }
    }
  }
  // }}}

  template <typename T,class LogicalOp>
  inline void fallbackLogicalAndOrXor(const Img<T> *src1, const Img<T> *src2, Img<T> *dst,const LogicalOp &op)
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
      ImgIterator<T>      itDst  =  dst->getROIIterator(c);
      for(;itSrc1.inRegion(); ++itSrc1, ++itSrc2, ++itDst){
        *itDst = op(*itSrc1,*itSrc2);
      }
    }
  }
  // }}}

  template <typename T,class LogicalOp>
  inline void fallbackLogicalAndOrXorC(const Img<T> *src, const T value, Img<T> *dst,const LogicalOp &op)
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

  // }}}

    // {{{ C++ fallback Logical Operators
  template <typename T> class AndOp {
  public:
    inline T operator()(T val1,T val2) const { return val1 & val2; }
  };

  template <typename T> class OrOp {
  public:
    inline T operator()(T val1,T val2) const { return val1 | val2; }
  };

  template <typename T> class XorOp {
  public:
    inline T operator()(T val1,T val2) const { return val1 ^ val2; }
  };
  // }}}

  
#define ICL_INSTANTIATE_DEPTH(T,U) \
  void Logical:: U  (const Img ## T *src1, const Img ## T *src2, Img## T *dst){\
    ippiAndOrXorCall<icl ## T,ippi ## U ##_ ## T ##_C1R>(src1,src2,dst);}


#define ICL_INSTANTIATE_DEPTH_FB(T,U) \
  void Logical:: U (const Img ## T *src1, const Img ## T *src2, Img ## T *dst){\
    fallbackLogicalAndOrXor<icl ## T>(src1, src2,dst, U ##Op<icl ## T>());}

#define ICL_INSTANTIATE_DEPTH_NOT_FB(T) \
  void Logical::Not (const Img ## T *src, Img ## T *dst){\
    fallbackLogicalNot<icl ## T>(src, dst);}

#define ICL_INSTANTIATE_DEPTH_C(T,U) \
  void Logical:: U ## C (const Img ## T *src, const icl ## T value, Img## T *dst){\
    ippiAndOrXorCallC<icl ## T,ippi ## U ## C_ ## T ## _C1R>(src,value,dst);}


#define ICL_INSTANTIATE_DEPTH_C_FB(T,U) \
  void Logical:: U ## C (const Img ## T *src, const icl ## T value, Img ## T *dst){\
    fallbackLogicalAndOrXorC<icl ##T>(src, value,dst, U ## Op<icl ## T>());}
  
#ifdef WITH_IPP_OPTIMIZATION

  // {{{ ippi-function call templates

  template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, const T*, int, T*, int, IppiSize)>
  inline void ippiAndOrXorCall(const Img<T> *src1, const Img<T> *src2, Img<T> *dst)
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

  template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, const T, T*, int, IppiSize)>
  inline void ippiAndOrXorCallC(const Img<T> *src, T value, Img<T> *dst)
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
  // }}}

  template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize)>
  inline void ippiNotCall(const Img<T> *src, Img<T> *dst)
  {
    // {{{ open
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels()== dst->getChannels());
    for (int c=src->getChannels()-1; c >= 0; --c) {
      ippiFunc (src->getROIData (c), src->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize());
    }
  }
  // }}}

  // }}}

  // {{{ function specializations

  ICL_INSTANTIATE_DEPTH(8u,And)
  ICL_INSTANTIATE_DEPTH_FB(16s,And)
  ICL_INSTANTIATE_DEPTH(32s,And)
  
  ICL_INSTANTIATE_DEPTH(8u,Or)
  ICL_INSTANTIATE_DEPTH_FB(16s,Or)  
  ICL_INSTANTIATE_DEPTH(32s,Or)
  
  ICL_INSTANTIATE_DEPTH(8u,Xor)
  ICL_INSTANTIATE_DEPTH_FB(16s,Xor)
  ICL_INSTANTIATE_DEPTH(32s,Xor)

  ICL_INSTANTIATE_DEPTH_C(8u,And)
  ICL_INSTANTIATE_DEPTH_C_FB(16s,And)
  ICL_INSTANTIATE_DEPTH_C(32s,And)
  
  ICL_INSTANTIATE_DEPTH_C(8u,Or)
  ICL_INSTANTIATE_DEPTH_C_FB(16s,Or)  
  ICL_INSTANTIATE_DEPTH_C(32s,Or)
  
  ICL_INSTANTIATE_DEPTH_C(8u,Xor)
  ICL_INSTANTIATE_DEPTH_C_FB(16s,Xor)
  ICL_INSTANTIATE_DEPTH_C(32s,Xor)


  ICL_INSTANTIATE_DEPTH_NOT_FB(16s)
  ICL_INSTANTIATE_DEPTH_NOT_FB(32s)
  
  void Logical::Not (const Img8u *src, Img8u *dst)
  {
    ippiNotCall<icl8u,ippiNot_8u_C1R>(src,dst);
  }

  // }}}

#else

  // {{{ C++ fallback function specializations
  ICL_INSTANTIATE_DEPTH_FB(8u,And)
  ICL_INSTANTIATE_DEPTH_FB(16s,And)
  ICL_INSTANTIATE_DEPTH_FB(32s,And)
  
  ICL_INSTANTIATE_DEPTH_FB(8u,Or)
  ICL_INSTANTIATE_DEPTH_FB(16s,Or)  
  ICL_INSTANTIATE_DEPTH_FB(32s,Or)
  
  ICL_INSTANTIATE_DEPTH_FB(8u,Xor)
  ICL_INSTANTIATE_DEPTH_FB(16s,Xor)
  ICL_INSTANTIATE_DEPTH_FB(32s,Xor)

  ICL_INSTANTIATE_DEPTH_C_FB(8u,And)
  ICL_INSTANTIATE_DEPTH_C_FB(16s,And)
  ICL_INSTANTIATE_DEPTH_C_FB(32s,And)
  
  ICL_INSTANTIATE_DEPTH_C_FB(8u,Or)
  ICL_INSTANTIATE_DEPTH_C_FB(16s,Or)  
  ICL_INSTANTIATE_DEPTH_C_FB(32s,Or)
  
  ICL_INSTANTIATE_DEPTH_C_FB(8u,Xor)
  ICL_INSTANTIATE_DEPTH_C_FB(16s,Xor)
  ICL_INSTANTIATE_DEPTH_C_FB(32s,Xor)

  ICL_INSTANTIATE_DEPTH_NOT_FB(8u)
  ICL_INSTANTIATE_DEPTH_NOT_FB(16s)
  ICL_INSTANTIATE_DEPTH_NOT_FB(32s)
  // }}}

#endif
#undef ICL_INSTANTIATE_DEPTH
#undef ICL_INSTANTIATE_DEPTH_C

#undef ICL_INSTANTIATE_DEPTH_FB
#undef ICL_INSTANTIATE_DEPTH_C_FB
#undef ICL_INSTANTIATE_DEPTH_NOT_FB

  // {{{ ImgBase* versions

#define ICL_INSTANTIATE_DEPTH(T) \
     case depth ## T: Not(poSrc->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Logical::Not (const ImgBase *poSrc, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_INT_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

  #define ICL_INSTANTIATE_DEPTH(T) \
     case depth ## T: And(poSrc1->asImg<icl ## T>(),poSrc2->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Logical::And (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    ICLASSERT_RETURN( poSrc1->getDepth() ==  poSrc2->getDepth());
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    switch (poSrc1->getDepth()) {
      ICL_INSTANTIATE_ALL_INT_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}
    
  #define ICL_INSTANTIATE_DEPTH(T) \
     case depth ## T: Or(poSrc1->asImg<icl ## T>(),poSrc2->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Logical::Or (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    ICLASSERT_RETURN( poSrc1->getDepth() ==  poSrc2->getDepth());
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    switch (poSrc1->getDepth()) {
      ICL_INSTANTIATE_ALL_INT_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

  #define ICL_INSTANTIATE_DEPTH(T) \
     case depth ## T: Xor(poSrc1->asImg<icl ## T>(),poSrc2->asImg<icl ## T>(),(*ppoDst)->asImg<icl ## T>()); break;
  void Logical::Xor (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
  {
    // {{{ open
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    ICLASSERT_RETURN( poSrc1->getDepth() ==  poSrc2->getDepth());
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    switch (poSrc1->getDepth()) {
      ICL_INSTANTIATE_ALL_INT_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
     case depth ## T: AndC(poSrc->asImg<icl ## T>(),value,(*ppoDst)->asImg<icl ## T>()); break;
  void Logical::AndC (const ImgBase *poSrc, const icl8u value, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_INT_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
     case depth ## T: OrC(poSrc->asImg<icl ## T>(),value,(*ppoDst)->asImg<icl ## T>()); break;
  void Logical::OrC (const ImgBase *poSrc, const icl8u value, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_INT_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
#undef ICL_INSTANTIATE_DEPTH
  // }}}

#define ICL_INSTANTIATE_DEPTH(T) \
     case depth ## T: XorC(poSrc->asImg<icl ## T>(),value,(*ppoDst)->asImg<icl ## T>()); break;
  void Logical::XorC (const ImgBase *poSrc, const icl8u value, ImgBase **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()) {
      ICL_INSTANTIATE_ALL_INT_DEPTHS
      default: ICL_INVALID_FORMAT; break;
    };
  }
  #undef ICL_INSTANTIATE_DEPTH
  // }}}
  
// }}}
}
