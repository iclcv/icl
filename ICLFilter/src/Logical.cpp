#include "Logical.h"
#include "Img.h"
namespace icl {

// {{{ not for all (the implementation of the IPP icl32f version for NOT uses the fallback, because no IPP version is aviable)
template <typename T,typename R>
  void fallbacklogicalNot(const Img<T> *src, Img<T> *dst)
    // {{{ open
  {
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    for(int c=src->getChannels()-1; c >= 0; --c) {
      ImgIterator<T> itSrc = const_cast<Img<T>*>(src)->getROIIterator(c);
      ImgIterator<T> itDst = dst->getROIIterator(c);
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = ~(R)*itSrc ;
      }
    }
  }
    // }}}
// }}}
#ifdef WITH_IPP_OPTIMIZATION
  // {{{ ippi-function call templates

template <typename T,typename R, IppStatus (*ippiFunc) (const R*, int, const R*, int, R*, int, IppiSize)>
  inline void ippiAndOrXorCall(const Img<T> *src1, const Img<T> *src2, Img<T> *dst)
  {
    // {{{ open
    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels()==src2->getChannels());
    ICLASSERT_RETURN( src1->getChannels()==dst->getChannels());
    for (int c=src1->getChannels()-1; c >= 0; --c) {
      ippiFunc ((R*)(src1->getROIData (c)), src1->getLineStep(),
                (R*)(src2->getROIData (c)), src2->getLineStep(),
                (R*)(dst->getROIData (c)), dst->getLineStep(),
                dst->getROISize());
    }    
  }
  // }}}
template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize)>
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



  void Logical::And (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    ippiAndOrXorCall<icl8u,icl8u,ippiAnd_8u_C1R>(src1,src2,dst);
  }
  void Logical::And (const Img32f *src1, const Img32f *src2, Img32f *dst)
  {
    ippiAndOrXorCall<icl32f,Ipp32s,ippiAnd_32s_C1R>(src1,src2,dst);
  }
  void Logical::Or (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    ippiAndOrXorCall<icl8u,icl8u,ippiOr_8u_C1R>(src1,src2,dst);
  }
  void Logical::Or (const Img32f *src1, const Img32f *src2, Img32f *dst)
  {
    ippiAndOrXorCall<icl32f,Ipp32s,ippiOr_32s_C1R>(src1,src2,dst);
  }
  void Logical::Xor (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    ippiAndOrXorCall<icl8u,icl8u,ippiXor_8u_C1R>(src1,src2,dst);
  }
  void Logical::Xor (const Img32f *src1, const Img32f *src2, Img32f *dst)
  {
    ippiAndOrXorCall<icl32f,Ipp32s,ippiXor_32s_C1R>(src1,src2,dst);
  }
  
  void Logical::Not (const Img8u *src, Img8u *dst)
  {
    ippiNotCall<icl8u,ippiNot_8u_C1R>(src,dst);
  }
  void Logical::Not (const Img32f *src, Img32f *dst)
  {
    fallbacklogicalNot<icl32f,int>(src, dst);
  }
  // }}}

#else
  // {{{ C++ fallback Logical class

  template <typename T,typename R>
  //void fallbacklogical(const Img<T> *src1, const Img<T> *src2, Img<T> *dst,T (*op)(T o1,T o2))
  void fallbacklogicalOr(const Img<T> *src1, const Img<T> *src2, Img<T> *dst)
    // {{{ open
  {
    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
    ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );
    for(int c=src1->getChannels()-1; c >= 0; --c) {
      ImgIterator<T> itSrc1 = const_cast<Img<T>*>(src1)->getROIIterator(c);
      ImgIterator<T> itSrc2 = const_cast<Img<T>*>(src2)->getROIIterator(c);
      ImgIterator<T> itDst = dst->getROIIterator(c);
      for(;itSrc1.inRegion(); ++itSrc1, ++itSrc2, ++itDst){
        *itDst = (R)*itSrc1 | (R)*itSrc2;
      }
    }
  }

template <typename T,typename R>
  //void fallbacklogical(const Img<T> *src1, const Img<T> *src2, Img<T> *dst,T (*op)(T o1,T o2))
  void fallbacklogicalAnd(const Img<T> *src1, const Img<T> *src2, Img<T> *dst)
    // {{{ open
  {
    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
    ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );
    for(int c=src1->getChannels()-1; c >= 0; --c) {
      ImgIterator<T> itSrc1 = const_cast<Img<T>*>(src1)->getROIIterator(c);
      ImgIterator<T> itSrc2 = const_cast<Img<T>*>(src2)->getROIIterator(c);
      ImgIterator<T> itDst = dst->getROIIterator(c);
      for(;itSrc1.inRegion(); ++itSrc1, ++itSrc2, ++itDst){
        *itDst = (R)*itSrc1 & (R)*itSrc2;
      }
    }
  }

template <typename T,typename R>
  //void fallbacklogical(const Img<T> *src1, const Img<T> *src2, Img<T> *dst,T (*op)(T o1,T o2))
  void fallbacklogicalXor(const Img<T> *src1, const Img<T> *src2, Img<T> *dst)
    // {{{ open
  {
    ICLASSERT_RETURN( src1 && src2 && dst );
    ICLASSERT_RETURN( src1->getROISize() == src2->getROISize() );
    ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
    ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );
    for(int c=src1->getChannels()-1; c >= 0; --c) {
      ImgIterator<T> itSrc1 = const_cast<Img<T>*>(src1)->getROIIterator(c);
      ImgIterator<T> itSrc2 = const_cast<Img<T>*>(src2)->getROIIterator(c);
      ImgIterator<T> itDst = dst->getROIIterator(c);
      for(;itSrc1.inRegion(); ++itSrc1, ++itSrc2, ++itDst){
        *itDst = (R)*itSrc1 ^ (R)*itSrc2;
      }
    }
  }










  void Logical::Or (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    //fallbacklogical(src1, src2,dst, std::operator |);
    fallbacklogicalOr<icl8u,icl8u>(src1, src2,dst);
  }
  void Logical::Or (const Img32f *src1, const Img32f *src2, Img32f *dst)
  {
    fallbacklogicalOr<icl32f,int>(src1, src2,dst);
  }
  void Logical::And (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    fallbacklogicalAnd<icl8u,icl8u>(src1, src2,dst);
  }
  void Logical::And (const Img32f *src1, const Img32f *src2, Img32f *dst)
  {
    fallbacklogicalAnd<icl32f,int>(src1, src2,dst);
  }
  void Logical::Xor (const Img8u *src1, const Img8u *src2, Img8u *dst)
  {
    fallbacklogicalXor<icl8u,icl8u>(src1, src2,dst);
  }
  void Logical::Xor (const Img32f *src1, const Img32f *src2, Img32f *dst)
  {
    fallbacklogicalXor<icl32f,int>(src1, src2,dst);
  }
  void Logical::Not (const Img8u *src, Img8u *dst)
  {
    fallbacklogicalNot<icl8u,icl8u>(src, dst);
  }
  void Logical::Not (const Img32f *src, Img32f *dst)
  {
    fallbacklogicalNot<icl32f,int>(src, dst);
  }
  // }}}


#endif

  // {{{ ImgI* versions
  
  void Logical::Not (const ImgI *poSrc, ImgI **ppoDst)
  {

    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    if (poSrc->getDepth () == depth8u)
      Not(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
    else
      Not(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
  void Logical::And (const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    if (poSrc1->getDepth ()!=poSrc1->getDepth ())return;
    if (poSrc1->getDepth () == depth8u)
      And(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
    else
      And(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }

  void Logical::Or (const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    if (poSrc1->getDepth ()!=poSrc1->getDepth ())return;
    if (poSrc1->getDepth () == depth8u)
      Or(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
    else
      Or(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }

  void Logical::Xor (const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst)
  {
    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc1)) return;
    if (poSrc1->getDepth ()!=poSrc1->getDepth ())return;
    if (poSrc1->getDepth () == depth8u)
      Xor(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
    else
      Xor(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }

  
  // }}}
  
// }}}
}
