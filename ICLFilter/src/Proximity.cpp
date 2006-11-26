#include "Proximity.h"
#include "Img.h"
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
      for (int c=src1->getChannels()-1; c >= 0; --c) {
         ippiFunc (src1->getROIData (c), src1->getLineStep(),
                   srcROI,
                   src2->getROIData (c), src2->getLineStep(),
                   src2->getROISize(),
                   dst->getROIData (c), dst->getLineStep());
      }
   }
   // }}}

     
   template <IppStatus (*ippiFunc) (const icl8u*, int, IppiSize, const icl8u*, int, IppiSize, icl32f*, int)>
   inline void ippiColorCall(const Img8u *src1, const Img8u *src2, Img32f *dst, Array<icl8u> m_oBuffer8u[2], Array<icl32f> m_oBuffer32f[3])
      // {{{ open
   { 
      ICLASSERT_RETURN( src1);	   
      ICLASSERT_RETURN( src2);
      ICLASSERT_RETURN( dst );
      ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );
      ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
      m_oBuffer8u[0].resize(src1->getDim()*src1->getChannels());
      m_oBuffer8u[1].resize(src2->getDim()*src2->getChannels());
      m_oBuffer32f[2].resize(src1->getDim()*src1->getChannels());
      planarToInterleaved(src1->asImg<icl8u>(), *(m_oBuffer8u[0]));
      planarToInterleaved(src2->asImg<icl8u>(), *(m_oBuffer8u[1]));
	   Size srcROI=src1->getSize()-src2->getSize();
     ippiFunc (*m_oBuffer8u[0], src1->getLineStep(),
                   srcROI,
                   *m_oBuffer8u[1], src2->getLineStep(),
                   src2->getROISize(),
                   *m_oBuffer32f[2], dst->getLineStep());
      interleavedToPlanar(*(m_oBuffer32f[2]),dst->getSize(),src1->getChannels(),dst->asImg<icl32f>());
   }

   template <IppStatus (*ippiFunc) (const icl32f*, int, IppiSize, const icl32f*, int, IppiSize, icl32f*, int)>
   inline void ippiColorCall(const Img32f *src1, const Img32f *src2, Img32f *dst/*, Array<icl32f> *m_oBuffer32f*/)
      // {{{ open
   { 
      ICLASSERT_RETURN( src1);	   
      ICLASSERT_RETURN( src2);
      ICLASSERT_RETURN( dst );
      ICLASSERT_RETURN( src1->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src1->getChannels() == dst->getChannels() );
      ICLASSERT_RETURN( src1->getChannels() == src2->getChannels() );
	   Array<icl32f> m_oBuffer32f[4];
      m_oBuffer32f[0].resize(src1->getDim()*src1->getChannels());
      m_oBuffer32f[1].resize(src2->getDim()*src2->getChannels());
      m_oBuffer32f[2].resize(src1->getDim()*src1->getChannels());  // ohne *2 absurz, seg fault, warum????
      planarToInterleaved(src1->asImg<icl32f>(), *(m_oBuffer32f[0]));
      planarToInterleaved(src2->asImg<icl32f>(), *(m_oBuffer32f[1]));
	   Size srcROI=src1->getSize()-src2->getSize();
	   //int ROIOffset=(src1->getSize().width*(src2->getSize().height-1)/2+(src2->getSize()-1)/2)*src1->getChannels();

     ippiFunc (*m_oBuffer32f[0]/*+ROIOffset*/, src1->getLineStep()*src1->getChannels(),
                   srcROI,
                   *m_oBuffer32f[1], src2->getLineStep()*src2->getChannels(),
                   src2->getROISize(),
                   *m_oBuffer32f[2], src1->getLineStep()*src1->getChannels());
      interleavedToPlanar(*(m_oBuffer32f[0]),dst->getSize(),src1->getChannels(),dst->asImg<icl32f>());
      printf("7\n");
	   /*for (int j=0;j<10;j++){
      planarToInterleaved(dst->asImg<icl32f>(), *(m_oBuffer32f[0]));
      interleavedToPlanar(*(m_oBuffer32f[0]),dst->getSize(),src1->getChannels(),dst->asImg<icl32f>());
	   };*/
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

    void Proximity::CrossCorrFull_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst,bool color){
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

    void Proximity::CrossCorrFull_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst,bool color){
      int ch=src1->getChannels();
      if (!color){  
        ch=1;
      }
      printf("CHans:%i\n",ch);
      switch(ch){
	      case 3:ippiColorCall<ippiCrossCorrFull_Norm_32f_C3R>(src1,src2,dst);break;
        case 4:ippiColorCall<ippiCrossCorrFull_Norm_32f_C4R>(src1,src2,dst);break;
        default:ippiCall<icl32f,ippiCrossCorrFull_Norm_32f_C1R>(src1,src2,dst);break;
     }
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

    void Proximity::CrossCorrFull_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst,bool color){
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

    void Proximity::CrossCorrFull_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst,bool color){
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
    if (poSrc1->getDepth () == depth8u)
       SqrDistanceFull_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>());
    else
       SqrDistanceFull_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
   // }}}

  void Proximity::SqrDistanceValid_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    if (poSrc1->getDepth () == depth8u)
       SqrDistanceValid_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>());
    else
       SqrDistanceValid_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
   // }}}

  void Proximity::SqrDistanceSame_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    if (poSrc1->getDepth () == depth8u)
       SqrDistanceSame_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>());
    else
       SqrDistanceSame_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
   // }}}

  void Proximity::CrossCorrFull_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    if (poSrc1->getDepth () == depth8u)
       CrossCorrFull_NormLevel(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>());
    else
       CrossCorrFull_NormLevel(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
   // }}}

  void Proximity::CrossCorrValid_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    if (poSrc1->getDepth () == depth8u)
       CrossCorrValid_NormLevel(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>());
    else
       CrossCorrValid_NormLevel(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
   // }}}

  void Proximity::CrossCorrSame_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    if (poSrc1->getDepth () == depth8u)
       CrossCorrSame_NormLevel(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>());
    else
       CrossCorrSame_NormLevel(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
   // }}}


  void Proximity::CrossCorrFull_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst,bool color)
      // {{{ open
  {
	  printf("x\n");
    FUNCTION_LOG("");
	    printf("f\n");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
	    printf("g\n");
	  printf("y\n");
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
printf("b\n");
    ICLASSERT_RETURN( (*ppoDst)->getDepth() == depth32f);
    printf("d\n");
    if (poSrc1->getDepth() == depth8u)
       CrossCorrFull_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>(), color);
    else{
	    printf("_\n");
       CrossCorrFull_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),color);
    }
    printf("c\n");
  }
   // }}}


  void Proximity::CrossCorrValid_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    if (poSrc1->getDepth () == depth8u)
       CrossCorrValid_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>());
    else
       CrossCorrValid_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
   // }}}

  void Proximity::CrossCorrSame_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc1->getDepth());
    if (!prepare (ppoDst, poSrc1,depth32f)) return;
    if (poSrc1->getDepth () == depth8u)
       CrossCorrSame_Norm(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>());
    else
       CrossCorrSame_Norm(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
  }
   // }}}

// }}}

}
