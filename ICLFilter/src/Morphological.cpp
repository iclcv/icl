#include "Morphological.h"
#include "Img.h"

namespace icl {

  // {{{ Constructor / Destructor

  Morphological::Morphological (const Size& maskSize, char* pcMask) {
    if (maskSize.width <= 0 || maskSize.height<=0) {
      ERROR_LOG("illegal width/height: " << maskSize.width << "/" << maskSize.height);
      //????
    } else setMask (maskSize,pcMask);
    this->pcMask=(Ipp8u*)pcMask;
  }
  // }}}
  void Morphological::setMask (Size maskSize, char* pcMask) {
    // make maskSize odd:
    maskSize.width  = (maskSize.width/2)*2 + 1;
    maskSize.height = (maskSize.height/2)*2 + 1;
    FilterMask::setMask (maskSize);
    this->pcMask=(Ipp8u*)pcMask;
  }


#ifdef WITH_IPP_OPTIMIZATION
  template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, const Ipp8u*, IppiSize, IppiPoint)>
  void Morphological::ippiMorphologicalCall (const Img<T> *src, Img<T> *dst) {
    for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset),
                 src->getLineStep(),
                 dst->getROIData (c),
                 dst->getLineStep(),
                 dst->getROISize(), pcMask,oMaskSize, oAnchor
        );
    };
  }


  template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize)>
  void Morphological::ippiMorphologicalCall3x3 (const Img<T> *src, Img<T> *dst) {
    for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset),
                 src->getLineStep(),
                 dst->getROIData (c),
                 dst->getLineStep(),
                 dst->getROISize()
        );
    };
  }

  template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, _IppiBorderType, IppiMorphState*)>
  void Morphological::ippiMorphologicalBorderReplicateCall (const Img<T> *src, Img<T> *dst) {
    for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset),
                 src->getLineStep(),
                 dst->getROIData (c),
                 dst->getLineStep(),
                 dst->getROISize(),
                 ippBorderRepl,
                 pState
        );
    };
  }




  template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
  void Morphological::ippiMorphologicalBorderCall (const Img<T> *src, Img<T> *dst) {
    for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset),
                 src->getLineStep(),
                 dst->getROIData (c),
                 dst->getLineStep(),
                 dst->getROISize(),
                 ippBorderRepl,
                 pAdvState
        );
    };
  }


  void Morphological::InitMorphState (Img8u *img) {
    ippiMorphologyInitAlloc_8u_C1R(img->getROISize().width, pcMask, oMaskSize, oAnchor,&pState);
  } 
  void Morphological::InitMorphAdvState (Img8u *img) {
    ippiMorphAdvInitAlloc_8u_C1R(&pAdvState, img->getROISize(), pcMask, oMaskSize, oAnchor);
  }
  void Morphological::InitMorphState (Img32f *img) {
    ippiMorphologyInitAlloc_32f_C1R(img->getROISize().width, pcMask, oMaskSize, oAnchor,&pState);
  } 
  void Morphological::InitMorphAdvState (Img32f *img) {
    ippiMorphAdvInitAlloc_32f_C1R(&pAdvState, img->getROISize(), pcMask, oMaskSize, oAnchor);
  }

  void Morphological::MorphAdvStateFree(){
    ippiMorphAdvFree(pAdvState);
  }
  void Morphological::MorphStateFree(){
    ippiMorphologyFree(pState);
  }


  void Morphological::Dilate(const Img8u *src, Img8u *dst){
    ippiMorphologicalCall<icl8u,ippiDilate_8u_C1R> (src,dst);
  };
  void Morphological::Dilate3x3(const Img8u *src, Img8u *dst){
    ippiMorphologicalCall3x3<icl8u,ippiDilate3x3_8u_C1R> (src,dst);
  };

  void Morphological::Dilate(const Img32f *src, Img32f *dst){
    ippiMorphologicalCall<icl32f,ippiDilate_32f_C1R> (src,dst);
  };
  void Morphological::Dilate3x3(const Img32f *src, Img32f *dst){
    ippiMorphologicalCall3x3<icl32f,ippiDilate3x3_32f_C1R> (src,dst);
  };

  void Morphological::Erode(const Img8u *src, Img8u *dst){
    ippiMorphologicalCall<icl8u,ippiErode_8u_C1R> (src,dst);
  };
  void Morphological::Erode3x3(const Img8u *src, Img8u *dst){
    ippiMorphologicalCall3x3<icl8u,ippiErode3x3_8u_C1R> (src,dst);
  };
  void Morphological::Erode(const Img32f *src, Img32f *dst){
    ippiMorphologicalCall<icl32f,ippiErode_32f_C1R> (src,dst);
  };
  void Morphological::Erode3x3(const Img32f *src, Img32f *dst){
    ippiMorphologicalCall3x3<icl32f,ippiErode3x3_32f_C1R> (src,dst);
  };

/*

template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphState*),IppStatus (*ippiInitFunc) (int, const Ipp8u*, IppiSize, IppiPoint,IppiMorphState**)>

*/

  void Morphological::DilateBorderReplicate(const Img8u *src, Img8u *dst){
  //  ippiDilateBorderReplicate_8u_C1R();
//    ippiMorphologyInitAlloc_8u_C1R();
    
    ippiMorphologicalBorderReplicateCall<icl8u,ippiDilateBorderReplicate_8u_C1R> (src,dst);
  }

  void Morphological::DilateBorderReplicate(const Img32f *src, Img32f *dst){
    ippiMorphologicalBorderReplicateCall<icl32f,ippiDilateBorderReplicate_32f_C1R> (src,dst);
  }
  void Morphological::ErodeBorderReplicate(const Img8u *src, Img8u *dst){
    ippiMorphologicalBorderReplicateCall<icl8u,ippiErodeBorderReplicate_8u_C1R> (src,dst);
  }
  void Morphological::ErodeBorderReplicate(const Img32f *src, Img32f *dst){
    ippiMorphologicalBorderReplicateCall<icl32f,ippiErodeBorderReplicate_32f_C1R> (src,dst);
  }
  void Morphological::OpenBorder(const Img8u *src, Img8u *dst){
    ippiMorphologicalBorderCall<icl8u,ippiMorphOpenBorder_8u_C1R> (src,dst);
  }

  void Morphological::OpenBorder(const Img32f *src, Img32f *dst){
    ippiMorphologicalBorderCall<icl32f,ippiMorphOpenBorder_32f_C1R> (src,dst);
  }
  void Morphological::CloseBorder(const Img8u *src, Img8u *dst){
    ippiMorphologicalBorderCall<icl8u,ippiMorphCloseBorder_8u_C1R> (src,dst);
  }
  void Morphological::CloseBorder(const Img32f *src, Img32f *dst){
    ippiMorphologicalBorderCall<icl32f,ippiMorphCloseBorder_32f_C1R> (src,dst);
  }


  // {{{ ImgBase* version

   void Morphological::Dilate (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       Dilate(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       Dilate(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}
   void Morphological::Dilate3x3 (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       Dilate3x3(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       Dilate3x3(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}
   void Morphological::Erode (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       Erode(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       Erode(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}
   void Morphological::Erode3x3 (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       Erode3x3(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       Erode3x3(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}

   void Morphological::DilateBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       DilateBorderReplicate(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       DilateBorderReplicate(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}
   void Morphological::ErodeBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       ErodeBorderReplicate(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       ErodeBorderReplicate(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}

   void Morphological::OpenBorder (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       OpenBorder(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       OpenBorder(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}

   void Morphological::CloseBorder (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       CloseBorder(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       CloseBorder(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}

   void Morphological::InitMorphAdvState (ImgBase **ppoImg)
      // {{{ open
   {
     FUNCTION_LOG("");
     ICLASSERT_RETURN(ppoImg);
     if ((*ppoImg)->getDepth () == depth8u)
       InitMorphAdvState((*ppoImg)->asImg<icl8u>());
     else
       InitMorphAdvState((*ppoImg)->asImg<icl32f>());
   }
   // }}}
   void Morphological::InitMorphState (ImgBase **ppoImg)
      // {{{ open
   {
     FUNCTION_LOG("");
     ICLASSERT_RETURN(ppoImg);
     if ((*ppoImg)->getDepth () == depth8u)
       InitMorphState((*ppoImg)->asImg<icl8u>());
     else
       InitMorphState((*ppoImg)->asImg<icl32f>());
   }
   // }}}



  // }}}
#endif

}
