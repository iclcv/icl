#include <Morphological.h>
#include <Img.h>

namespace icl {

  // {{{ Constructor / Destructor

  Morphological::Morphological (const Size& maskSize, char* pcMask) {
    if (maskSize.width <= 0 || maskSize.height<=0) {
      ERROR_LOG("illegal width/height: " << maskSize.width << "/" << maskSize.height);
      //????
    } else setMask (maskSize,pcMask);
    this->pcMask=(icl8u*)pcMask;
  }
  // }}}
  void Morphological::setMask (Size maskSize, char* pcMask) {
    // make maskSize odd:
    maskSize.width  = (maskSize.width/2)*2 + 1;
    maskSize.height = (maskSize.height/2)*2 + 1;
    FilterMask::setMask (maskSize);
    this->pcMask=(icl8u*)pcMask;
  }


#ifdef WITH_IPP_OPTIMIZATION
  template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, const Ipp8u*, IppiSize, IppiPoint)>
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


  template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize)>
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

  template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, _IppiBorderType, IppiMorphState*)>
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




  template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
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
/*
  template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphGrayState_32f*)>
  void Morphological::ippiMorphologicalGrayCall (const Img<T> *src, Img<T> *dst) {
    for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset),
                 src->getLineStep(),
                 dst->getROIData (c),
                 dst->getLineStep(),
                 dst->getROISize(),
                 ippBorderRepl,
                 pGrayState_32f
        );
    };
  }
*/

  void Morphological::InitMorphState (Img8u *img) {
    ippiMorphologyInitAlloc_8u_C1R(img->getROISize().width, pcMask, oMaskSize, oAnchor,&pState);
  } 
  void Morphological::InitMorphAdvState (Img8u *img) {
    ippiMorphAdvInitAlloc_8u_C1R(&pAdvState, img->getROISize(), pcMask, oMaskSize, oAnchor);
  }
  /*
  void Morphological::InitMorphGrayState (Img8u *img) {
    ippiMorphGrayInitAlloc_8u_C1R(&pGrayState_8u, img->getROISize(), pcMask, oMaskSize, oAnchor);
  }
  */
  void Morphological::InitMorphState (Img32f *img) {
    ippiMorphologyInitAlloc_32f_C1R(img->getROISize().width, pcMask, oMaskSize, oAnchor,&pState);
  } 
  void Morphological::InitMorphAdvState (Img32f *img) {
    ippiMorphAdvInitAlloc_32f_C1R(&pAdvState, img->getROISize(), pcMask, oMaskSize, oAnchor);
  }
  /*
  void Morphological::InitMorphGrayState (Img32f *img) {
    ippiMorphGrayInitAlloc_32f_C1R(&pGrayState_32f, img->getROISize(), pcMask, oMaskSize, oAnchor);
  }*/
  void Morphological::MorphStateFree(){
    ippiMorphologyFree(pState);
  }
  void Morphological::MorphAdvStateFree(){
    ippiMorphAdvFree(pAdvState);
  }
  /*
  void Morphological::MorphGrayStateFree_8u(){
    ippiMorphAdvFree(pGrayState_8u);
  }
  void Morphological::MorphGrayStateFree_32f(){
    ippiMorphAdvFree(pGrayState_32f);
  }*/
  


  void Morphological::Dilate(const Img8u *src, Img8u *dst){
    ippiMorphologicalCall<icl8u,ippiDilate_8u_C1R> (src,dst);
  }
  void Morphological::Dilate3x3(const Img8u *src, Img8u *dst){
    ippiMorphologicalCall3x3<icl8u,ippiDilate3x3_8u_C1R> (src,dst);
  }

  void Morphological::Dilate(const Img32f *src, Img32f *dst){
    ippiMorphologicalCall<icl32f,ippiDilate_32f_C1R> (src,dst);
  }
  void Morphological::Dilate3x3(const Img32f *src, Img32f *dst){
    ippiMorphologicalCall3x3<icl32f,ippiDilate3x3_32f_C1R> (src,dst);
  }

  void Morphological::Erode(const Img8u *src, Img8u *dst){
    ippiMorphologicalCall<icl8u,ippiErode_8u_C1R> (src,dst);
  }
  void Morphological::Erode3x3(const Img8u *src, Img8u *dst){
    ippiMorphologicalCall3x3<icl8u,ippiErode3x3_8u_C1R> (src,dst);
  }
  void Morphological::Erode(const Img32f *src, Img32f *dst){
    ippiMorphologicalCall<icl32f,ippiErode_32f_C1R> (src,dst);
  }
  void Morphological::Erode3x3(const Img32f *src, Img32f *dst){
    ippiMorphologicalCall3x3<icl32f,ippiErode3x3_32f_C1R> (src,dst);
  }

/*

template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphState*),IppStatus (*ippiInitFunc) (int, const Ipp8u*, IppiSize, IppiPoint,IppiMorphState**)>

*/

  void Morphological::DilateBorderReplicate(const Img8u *src, Img8u *dst){
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




    void Morphological::TophatBorder(const Img8u *src, Img8u *dst){
      ippiMorphologicalBorderCall<icl8u,ippiMorphTophatBorder_8u_C1R> (src,dst);
    }
    void Morphological::BlackhatBorder(const Img8u *src, Img8u *dst){
      ippiMorphologicalBorderCall<icl8u,ippiMorphBlackhatBorder_8u_C1R> (src,dst);
    }
    void Morphological::GradientBorder(const Img8u *src, Img8u *dst){
      ippiMorphologicalBorderCall<icl8u,ippiMorphGradientBorder_8u_C1R> (src,dst);
    }
/*    void Morphological::GrayDilateBorder(const Img8u *src, Img8u *dst){
      ippiMorphologicalGrayCall<icl8u,ippiGrayDilateBorder_8u_C1R> (src,dst);
    }
    void Morphological::GrayErodeBorder(const Img8u *src, Img8u *dst){
      ippiMorphologicalGrayCall<icl8u,ippiGrayErodeBorder_8u_C1R> (src,dst);
    }*/
/*  void Morphological::ReconstructDilate(const Img8u *src, Img8u *dst){
      #warning "ReconstructDilate is not implemented with IPP optimization";
    }
    void Morphological::ReconstructErode(const Img8u *src, Img8u *dst){
      #warning "ReconstructErode is not implemented with IPP optimization";
    }*/

    void Morphological::TophatBorder(const Img32f *src, Img32f *dst){
      ippiMorphologicalBorderCall<icl32f,ippiMorphTophatBorder_32f_C1R> (src,dst);
    }
    void Morphological::BlackhatBorder(const Img32f *src, Img32f *dst){
      ippiMorphologicalBorderCall<icl32f,ippiMorphBlackhatBorder_32f_C1R> (src,dst);
    }
    void Morphological::GradientBorder(const Img32f *src, Img32f *dst){
      ippiMorphologicalBorderCall<icl32f,ippiMorphGradientBorder_32f_C1R> (src,dst);
    }
/*    void Morphological::GrayDilateBorder(const Img32f *src, Img32f *dst){
      ippiMorphologicalGrayCall<icl32f,ippiGrayDilateBorder_32f_C1R> (src,dst);
    }
    void Morphological::GrayErodeBorder(const Img32f *src, Img32f *dst){
      ippiMorphologicalGrayCall<icl32f,ippiGrayErodeBorder_32f_C1R> (src,dst);
    }*/
/*    void Morphological::ReconstructDilate(const Img32f *src, Img32f *dst){
      #warning "ReconstructDilate is not implemented with IPP optimization";
    }
    void Morphological::ReconstructErode(const Img32f *src, Img32f *dst){
      #warning "ReconstructErode is not implemented with IPP optimization";
    }*/












  // {{{ ImgBase* version

  void Morphological::Dilate (const ImgBase *poSrc, ImgBase **ppoDst)
    // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: Dilate(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: Dilate(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}
  void Morphological::Dilate3x3 (const ImgBase *poSrc, ImgBase **ppoDst)
    // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: Dilate3x3(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: Dilate3x3(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
     }
   }
  // }}}
  void Morphological::Erode (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: Erode(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: Erode(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}
  void Morphological::Erode3x3 (const ImgBase *poSrc, ImgBase **ppoDst)
    // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: Erode3x3(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: Erode3x3(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}

  void Morphological::DilateBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst)
    // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: DilateBorderReplicate(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: DilateBorderReplicate(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}
  void Morphological::ErodeBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst)
    // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: ErodeBorderReplicate(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: ErodeBorderReplicate(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}

  void Morphological::OpenBorder (const ImgBase *poSrc, ImgBase **ppoDst)
    // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: OpenBorder(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break; 
      case depth32f: OpenBorder(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}

  void Morphological::CloseBorder (const ImgBase *poSrc, ImgBase **ppoDst)
    // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: CloseBorder(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: CloseBorder(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}

  void Morphological::TophatBorder (const ImgBase *poSrc, ImgBase **ppoDst)
    // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: TophatBorder(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: TophatBorder(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}

  void Morphological::BlackhatBorder (const ImgBase *poSrc, ImgBase **ppoDst)
    // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: BlackhatBorder(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: BlackhatBorder(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}

  void Morphological::GradientBorder (const ImgBase *poSrc, ImgBase **ppoDst)
    // {{{ open
  {
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u: GradientBorder(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>()); break;
      case depth32f: GradientBorder(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}

/*
   void Morphological::GrayDilateBorder (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       GrayDilateBorder(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       GrayDilateBorder(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}


   void Morphological::GrayErodeBorder (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       GrayErodeBorder(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       GrayErodeBorder(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}
*/
/*   void Morphological::ReconstructDilate (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       ReconstructDilate(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       ReconstructDilate(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}

   void Morphological::ReconstructErode (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
   {
     FUNCTION_LOG("");
     if (!prepare (ppoDst, poSrc)) return;
     if (poSrc->getDepth () == depth8u)
       ReconstructErode(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());
     else
       ReconstructErode(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());
   }
   // }}}
*/

  void Morphological::InitMorphAdvState (ImgBase **ppoImg)
    // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN(ppoImg);
    switch ((*ppoImg)->getDepth ()){
      case depth8u: InitMorphAdvState((*ppoImg)->asImg<icl8u>()); break;
      case depth32f: InitMorphAdvState((*ppoImg)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}
   void Morphological::InitMorphState (ImgBase **ppoImg)
    // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN(ppoImg);
    switch ((*ppoImg)->getDepth ()){
      case depth8u: InitMorphState((*ppoImg)->asImg<icl8u>()); break;
      case depth32f: InitMorphState((*ppoImg)->asImg<icl32f>()); break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}
/*
  void Morphological::InitMorphGrayState (ImgBase **ppoImg)
      // {{{ open
   {
     FUNCTION_LOG("");
     ICLASSERT_RETURN(ppoImg);
     if ((*ppoImg)->getDepth () == depth8u)
       InitMorphGrayState((*ppoImg)->asImg<icl8u>());
     else
       InitMorphGrayState((*ppoImg)->asImg<icl32f>());
   }
   // }}}
*/


  // }}}
#else
    void Morphological::MorphStateFree(){
      #warning "MorphStateFree is not implemented without IPP optimization";
    }
    void Morphological::MorphAdvStateFree(){
      #warning "MorphAdvStateFree is not implemented without IPP optimization";
    }
/*    void Morphological::MorphGrayStateFree_8u(){
      #warning "MorphAdvStateFree_8u is not implemented without IPP optimization";
    }
    void Morphological::MorphGrayStateFree_32f(){
      #warning "MorphAdvStateFree_32f is not implemented without IPP optimization";
    }*/
    void Morphological::Erode (const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "Erode is not implemented without IPP optimization";
    }
    void Morphological::Erode3x3 (const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "Erode3x3 is not implemented without IPP optimization";
    }
    void Morphological::Dilate (const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "Dilate is not implemented without IPP optimization";
    }
    void Morphological::Dilate3x3 (const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "Dilate3x3 is not implemented without IPP optimization";
    }
    void Morphological::DilateBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "DilateBorderReplicate is not implemented without IPP optimization";
    }
    void Morphological::ErodeBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "ErodeBorderReplicate is not implemented without IPP optimization";
    }
    void Morphological::OpenBorder(const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "OpenBorder is not implemented without IPP optimization";
    }
    void Morphological::CloseBorder(const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "CloseBorder is not implemented without IPP optimization";
    }
    void Morphological::TophatBorder(const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "TopHatBorder is not implemented without IPP optimization";
    }
    void Morphological::BlackhatBorder(const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "BlackhatBorder is not implemented without IPP optimization";
    }
    void Morphological::GradientBorder(const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "GradientBorder is not implemented without IPP optimization";
    }
/*    void Morphological::GrayDilateBorder(const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "GrayDilateBorder is not implemented without IPP optimization";
    }
    void Morphological::GrayErodeBorder(const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "GrayErodeBorder is not implemented without IPP optimization";
    }*/
/*
    void Morphological::ReconstructDilate(const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "ReconstructDilate is not implemented without IPP optimization";
    }
    void Morphological::ReconstructErode(const ImgBase *poSrc, ImgBase **ppoDst){
      #warning "ReconstructErode is not implemented without IPP optimization";
    }
*/

    void Morphological::InitMorphState(ImgBase **ppoImg){
      #warning "InitMorphState is not implemented without IPP optimization";
    }
    void Morphological::InitMorphAdvState(ImgBase **ppoImg){
      #warning "InitMorphAdvState is not implemented without IPP optimization";
    }
/*    void Morphological::InitMorphGrayState(ImgBase **ppoImg){
      #warning "InitMorphAdvState is not implemented without IPP optimization";
    }*/
#endif

}
