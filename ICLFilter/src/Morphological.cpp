#include <Morphological.h>
#include <Img.h>

namespace icl {

  // {{{ Constructor / Destructor

  Morphological::Morphological (const Size& maskSize, char* pcMask, optype eOptype) {
    if (maskSize.width <= 0 || maskSize.height<=0) {
      ERROR_LOG("illegal width/height: " << maskSize.width << "/" << maskSize.height);
      //????
    } else setMask (maskSize,pcMask);
    m_pcMask=(icl8u*)pcMask;
    m_sMasksize=maskSize;
    m_eType=eOptype;
    m_bMorphState8u=false;
    m_bMorphState32f=false;
    m_bMorphAdvState8u=false;
    m_bMorphAdvState32f=false;
    m_bHas_changed=true;
    m_bHas_changedAdv=true;
  }
  Morphological::~Morphological(){
    deleteMorphStates();
  }
  // }}}
  void Morphological::setMask (Size maskSize, char* pcMask) {
    // make maskSize odd:
    maskSize.width  = (maskSize.width/2)*2 + 1;
    maskSize.height = (maskSize.height/2)*2 + 1;
    FilterMask::setMask (maskSize);
    m_pcMask=(icl8u*)pcMask;
    m_sMasksize=maskSize;
    m_bHas_changed=true;
    m_bHas_changedAdv=true;
  }
  //ippiMorphologyFree(m_pState8u);

  void Morphological::deleteMorphStates(){
    if (m_bMorphState8u){
      ippiMorphologyFree(m_pState8u);
      m_bMorphState8u=false;
    }
    if (m_bMorphAdvState8u){
      ippiMorphAdvFree(m_pAdvState8u);
      m_bMorphAdvState8u=false;
    }
    if (m_bMorphState32f){
      ippiMorphologyFree(m_pState32f);
      m_bMorphState32f=false;
    }
    if (m_bMorphAdvState32f){
      ippiMorphAdvFree(m_pAdvState32f);
      m_bMorphAdvState32f=false;
    }
  }
    
  void Morphological::checkMorphAdvState8u(const Size roiSize){
    if (m_bHas_changedAdv){
      deleteMorphStates();
      ippiMorphAdvInitAlloc_8u_C1R(&m_pAdvState8u, roiSize, m_pcMask, oMaskSize, oAnchor);
      m_bMorphAdvState8u=true;
      m_bHas_changedAdv=false;
    }
  }

    void Morphological::checkMorphState8u(const Size roiSize){
    if (m_bHas_changed){
      deleteMorphStates();
      
      ippiMorphologyInitAlloc_8u_C1R(roiSize.width, m_pcMask, oMaskSize, oAnchor,&m_pState8u);
      
      m_bMorphState8u=true;
      m_bHas_changed=false;
    }
  }
  void Morphological::checkMorphAdvState32f(const Size roiSize){
    if (m_bHas_changedAdv){
      deleteMorphStates();
      ippiMorphAdvInitAlloc_32f_C1R(&m_pAdvState32f, roiSize, m_pcMask, oMaskSize, oAnchor);
      m_bMorphAdvState32f=true;
      m_bHas_changedAdv=false;
    }
  }
  
    void Morphological::checkMorphState32f(const Size roiSize){
    if (m_bHas_changed){
      deleteMorphStates();
      
      ippiMorphologyInitAlloc_32f_C1R(roiSize.width, m_pcMask, oMaskSize, oAnchor,&m_pState32f);
      
      m_bMorphState32f=true;
      m_bHas_changed=false;
    }
  }

  void Morphological::apply (const ImgBase *poSrc, ImgBase **ppoDst){
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    switch (poSrc->getDepth()){
      case depth8u:
        switch (m_eType){
          case dilate:ippiMorphologicalCall<icl8u,ippiDilate_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());break;
          case erode:ippiMorphologicalCall<icl8u,ippiErode_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());break;
          case dilate3x3:ippiMorphologicalCall3x3<icl8u,ippiDilate3x3_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());break;
          case erode3x3:ippiMorphologicalCall3x3<icl8u,ippiErode3x3_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());break;
          case dilateBorderReplicate:checkMorphState8u(poSrc->getROISize());ippiMorphologicalBorderReplicateCall<icl8u,ippiDilateBorderReplicate_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pState8u);break;
          case erodeBorderReplicate:checkMorphState8u(poSrc->getROISize());ippiMorphologicalBorderReplicateCall<icl8u,ippiErodeBorderReplicate_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pState8u);break;
          case openBorder:checkMorphAdvState8u(poSrc->getROISize());ippiMorphologicalBorderCall<icl8u,ippiMorphOpenBorder_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pAdvState8u);break;
          case closeBorder:checkMorphAdvState8u(poSrc->getROISize());ippiMorphologicalBorderCall<icl8u,ippiMorphCloseBorder_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pAdvState8u);break;
          case tophatBorder:checkMorphAdvState8u(poSrc->getROISize());ippiMorphologicalBorderCall<icl8u,ippiMorphTophatBorder_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pAdvState8u);break;
          case blackhatBorder:checkMorphAdvState8u(poSrc->getROISize());ippiMorphologicalBorderCall<icl8u,ippiMorphBlackhatBorder_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pAdvState8u);break;
          case gradientBorder:checkMorphAdvState8u(poSrc->getROISize());ippiMorphologicalBorderCall<icl8u,ippiMorphGradientBorder_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pAdvState8u);break;
        }
      break;
      case depth32f:
        switch (m_eType){
          case dilate:ippiMorphologicalCall<icl32f,ippiDilate_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());break;
          case erode:ippiMorphologicalCall<icl32f,ippiErode_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());break;
          case dilate3x3:ippiMorphologicalCall3x3<icl32f,ippiDilate3x3_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());break;
          case erode3x3:ippiMorphologicalCall3x3<icl32f,ippiErode3x3_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());break;
          case dilateBorderReplicate:checkMorphState32f(poSrc->getROISize());ippiMorphologicalBorderReplicateCall<icl32f,ippiDilateBorderReplicate_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pState32f);break;
          case erodeBorderReplicate:checkMorphState32f(poSrc->getROISize());ippiMorphologicalBorderReplicateCall<icl32f,ippiErodeBorderReplicate_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pState32f);break;
          case openBorder:checkMorphAdvState32f(poSrc->getROISize());ippiMorphologicalBorderCall<icl32f,ippiMorphOpenBorder_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pAdvState32f);break;
          case closeBorder:checkMorphAdvState32f(poSrc->getROISize());ippiMorphologicalBorderCall<icl32f,ippiMorphCloseBorder_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pAdvState32f);break;
          case tophatBorder:checkMorphAdvState32f(poSrc->getROISize());ippiMorphologicalBorderCall<icl32f,ippiMorphTophatBorder_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pAdvState32f);break;
          case blackhatBorder:checkMorphAdvState32f(poSrc->getROISize());ippiMorphologicalBorderCall<icl32f,ippiMorphBlackhatBorder_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pAdvState32f);break;
          case gradientBorder:checkMorphAdvState32f(poSrc->getROISize());ippiMorphologicalBorderCall<icl32f,ippiMorphGradientBorder_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pAdvState32f);break;
        }
      break;
      default: ICL_INVALID_FORMAT; break;
    }
  }
  
  icl8u* Morphological::getMask() const{
    return m_pcMask;
  }
  Size Morphological::getMaskSize() const{
    return m_sMasksize;
  }
  void Morphological::setOptype(optype type){
    m_eType=type;
  }
  Morphological::optype Morphological::getOptype() const{
    return m_eType;
  }
  
#ifdef WITH_IPP_OPTIMIZATION
  template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, const Ipp8u*, IppiSize, IppiPoint)>
  void Morphological::ippiMorphologicalCall (const Img<T> *src, Img<T> *dst) {
    for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset),
                 src->getLineStep(),
                 dst->getROIData (c),
                 dst->getLineStep(),
                 dst->getROISize(), m_pcMask,oMaskSize, oAnchor
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
  void Morphological::ippiMorphologicalBorderReplicateCall (const Img<T> *src, Img<T> *dst,IppiMorphState* state) {
    for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset),
                 src->getLineStep(),
                 dst->getROIData (c),
                 dst->getLineStep(),
                 dst->getROISize(),
                 ippBorderRepl,
                 state
        );
    };
  }

  template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
  void Morphological::ippiMorphologicalBorderCall (const Img<T> *src, Img<T> *dst, IppiMorphAdvState* advState) {
    for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData (c, this->oROIoffset),
                 src->getLineStep(),
                 dst->getROIData (c),
                 dst->getLineStep(),
                 dst->getROISize(),
                 ippBorderRepl,
                 advState
        );
    };
  }

  // }}}
#endif

}
