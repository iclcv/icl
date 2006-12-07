#include <Proximity.h>
#include <Img.h>
#include "ICLcc.h"

namespace icl {


#ifdef WITH_IPP_OPTIMIZATION
   // {{{ ippi-function call templates

   template <typename T, IppStatus (*ippiFunc) (const T*, int, IppiSize, const T*, int, IppiSize, icl32f*, int)>
   inline void ippiCall(const Img<T> *src1, const Img<T> *src2, Img32f *dst)
      // {{{ open
   { 
ICLASSERT_RETURN( src1);	   
	   ICLASSERT_RETURN( src2);
	   ICLASSERT_RETURN( dst );
      ICLASSERT_RETURN( src1 && src2 && dst );
      ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );
      ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
      Size srcROI=src1->getSize()-src2->getSize();
      //srcROI.width+=1;
      //srcROI.height+=1;
      for (int c=src1->getChannels()-1; c >= 0; --c) {
         ippiFunc (src1->getROIData (c), src1->getLineStep(),
                   srcROI,
                   src2->getROIData (c), src2->getLineStep(),
                   src2->getROISize(),
                   dst->getROIData (c), dst->getLineStep());
      }
   }
   // }}}
   
  // }}}



  // {{{ IPP Calls

    void Proximity::SqrDistanceFull_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      ippiCall<icl8u,ippiSqrDistanceFull_Norm_8u32f_C1R>(src1,src2,dst);
    }
    void Proximity::SqrDistanceSame_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      ippiCall<icl8u,ippiSqrDistanceSame_Norm_8u32f_C1R>(src1,src2,dst);
    }
    void Proximity::SqrDistanceValid_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      ippiCall<icl8u,ippiSqrDistanceValid_Norm_8u32f_C1R>(src1,src2,dst);
    }

    void Proximity::CrossCorrFull_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      ippiCall<icl8u,ippiCrossCorrFull_Norm_8u32f_C1R>(src1,src2,dst);
    }
    void Proximity::CrossCorrSame_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      ippiCall<icl8u,ippiCrossCorrSame_Norm_8u32f_C1R>(src1,src2,dst);
    }
    void Proximity::CrossCorrValid_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      ippiCall<icl8u,ippiCrossCorrValid_Norm_8u32f_C1R>(src1,src2,dst);
    }

    void Proximity::CrossCorrFull_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst){
      ippiCall<icl8u,ippiCrossCorrFull_NormLevel_8u32f_C1R>(src1,src2,dst);
    }
    void Proximity::CrossCorrSame_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst){
      ippiCall<icl8u,ippiCrossCorrSame_NormLevel_8u32f_C1R>(src1,src2,dst);
    }
    void Proximity::CrossCorrValid_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst){
      ippiCall<icl8u,ippiCrossCorrValid_NormLevel_8u32f_C1R>(src1,src2,dst);
    }

    void Proximity::SqrDistanceFull_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      ippiCall<icl32f,ippiSqrDistanceFull_Norm_32f_C1R>(src1,src2,dst);
    }
    void Proximity::SqrDistanceSame_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      ippiCall<icl32f,ippiSqrDistanceSame_Norm_32f_C1R>(src1,src2,dst);
    }
    void Proximity::SqrDistanceValid_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      ippiCall<icl32f,ippiSqrDistanceValid_Norm_32f_C1R>(src1,src2,dst);
    }

    void Proximity::CrossCorrFull_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      ippiCall<icl32f,ippiCrossCorrFull_Norm_32f_C1R>(src1,src2,dst);
    }
    void Proximity::CrossCorrSame_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      ippiCall<icl32f,ippiCrossCorrSame_Norm_32f_C1R>(src1,src2,dst);
    }
    void Proximity::CrossCorrValid_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      ippiCall<icl32f,ippiCrossCorrValid_Norm_32f_C1R>(src1,src2,dst);
    }

    void Proximity::CrossCorrFull_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst){
      ippiCall<icl32f,ippiCrossCorrFull_NormLevel_32f_C1R>(src1,src2,dst);
    }
    void Proximity::CrossCorrSame_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst){
      ippiCall<icl32f,ippiCrossCorrSame_NormLevel_32f_C1R>(src1,src2,dst);
    }
    void Proximity::CrossCorrValid_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst){
      ippiCall<icl32f,ippiCrossCorrValid_NormLevel_32f_C1R>(src1,src2,dst);
    }


  // }}}

  #else
  // {{{ C++ fallback 

    void Proximity::SqrDistanceFull_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      #warning "SqrDistanceFull_Norm is not implemented without IPP optimization";
    }
    void Proximity::SqrDistanceSame_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      #warning "SqrDistanceSame_Norm is not implemented without IPP optimization";
    }
    void Proximity::SqrDistanceValid_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      #warning "SqrDistanceValid_Norm is not implemented without IPP optimization";
    }

    void Proximity::CrossCorrFull_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      #warning "CrossCorrFull_Norm is not implemented without IPP optimization";
    }
    void Proximity::CrossCorrSame_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      #warning "CrossCorrSame_Norm is not implemented without IPP optimization";
    }
    void Proximity::CrossCorrValid_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst){
      #warning "CrossCorrValid_Norm is not implemented without IPP optimization";
    }

    void Proximity::CrossCorrFull_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst){
      #warning "CrossCorrFull_NormLevel is not implemented without IPP optimization";
    }
    void Proximity::CrossCorrSame_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst){
      #warning "CrossCorrSame_NormLevel is not implemented without IPP optimization";
    }
    void Proximity::CrossCorrValid_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst){
      #warning "CrossCorrValid_NormLevel is not implemented without IPP optimization";
    }

    void Proximity::SqrDistanceFull_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      #warning "SqrDistanceFull_Norm is not implemented without IPP optimization";
    }
    void Proximity::SqrDistanceSame_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      #warning "SqrDistanceSame_Norm is not implemented without IPP optimization";
    }
    void Proximity::SqrDistanceValid_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      #warning "SqrDistanceValid_Norm is not implemented without IPP optimization";
    }

    void Proximity::CrossCorrFull_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      #warning "CrossCorrFull_Norm is not implemented without IPP optimization";
    }
    void Proximity::CrossCorrSame_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      #warning "CrossCorrSame_Norm is not implemented without IPP optimization";
    }
    void Proximity::CrossCorrValid_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst){
      #warning "CrossCorrValid_Norm is not implemented without IPP optimization";
    }

    void Proximity::CrossCorrFull_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst){
      #warning "CrossCorrFull_NormLevel is not implemented without IPP optimization";
    }
    void Proximity::CrossCorrSame_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst){
      #warning "CrossCorrSame_NormLevel is not implemented without IPP optimization";
    }
    void Proximity::CrossCorrValid_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst){
      #warning "CrossCorrValid_NormLevel is not implemented without IPP optimization";
    }



  // }}}
  #endif
  // {{{ ImgBase* version

  void Proximity::SqrDistanceFull_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    switch (poSrc1->getDepth()){
      case depth8u: SqrDistanceFull_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>()); break;
      case depth32f: SqrDistanceFull_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
   // }}}

  void Proximity::SqrDistanceValid_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    switch (poSrc1->getDepth()){
      case depth8u: SqrDistanceValid_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>()); break;
      case depth32f: SqrDistanceValid_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
   // }}}

  void Proximity::SqrDistanceSame_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    switch (poSrc1->getDepth()){
      case depth8u: SqrDistanceSame_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>()); break;
      case depth32f: SqrDistanceSame_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
   // }}}

  void Proximity::CrossCorrFull_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    switch (poSrc1->getDepth()){
      case depth8u: CrossCorrFull_NormLevel(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>()); break;
      case depth32f: CrossCorrFull_NormLevel(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
   // }}}

  void Proximity::CrossCorrValid_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    switch (poSrc1->getDepth()){
      case depth8u: CrossCorrValid_NormLevel(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>()); break;
      case depth32f: CrossCorrValid_NormLevel(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
   // }}}

  void Proximity::CrossCorrSame_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    switch (poSrc1->getDepth()){
      case depth8u: CrossCorrSame_NormLevel(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>()); break;
      case depth32f: CrossCorrSame_NormLevel(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
   // }}}


  void Proximity::CrossCorrFull_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    ICLASSERT_RETURN( (*ppoDst)->getDepth() == depth32f);
    switch (poSrc1->getDepth()){
      case depth8u: CrossCorrFull_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>()); break;
      case depth32f: CrossCorrFull_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
   // }}}


  void Proximity::CrossCorrValid_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    switch (poSrc1->getDepth()){
      case depth8u: CrossCorrValid_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>()); break;
      case depth32f: CrossCorrValid_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
   // }}}

  void Proximity::CrossCorrSame_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    switch (poSrc1->getDepth()){
      case depth8u: CrossCorrSame_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>()); break;
      case depth32f: CrossCorrSame_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
   // }}}

// }}}

}
