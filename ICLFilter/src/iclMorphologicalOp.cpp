#include <iclMorphologicalOp.h>
#include <iclImg.h>
#include <iclRange.h>
#include <iclImgBorder.h>
#include <functional>
#include <iclBinaryArithmeticalOp.h>



namespace icl {
#ifndef HAVE_IPP
  template<class T, typename cmp_func>
  void morph_cpp(const Img<T> &src, Img<T> &dst, MorphologicalOp &op,
                 T init, cmp_func cmp, const icl8u *mask){

    Point an = op.getAnchor();
    Size si = op.getMaskSize();
    for(int c=0;c<src.getChannels();++c){
      const ImgIterator<T> s(const_cast<T*>(src.getData(c)), src.getWidth(),Rect(op.getROIOffset(), dst.getROISize()));
      const ImgIterator<T> sEnd = ImgIterator<T>::create_end_roi_iterator(src.getData(c),src.getWidth(), Rect(op.getROIOffset(), dst.getROISize()));
      ImgIterator<T> d = dst.beginROI(c);
      for(; s != sEnd; ++s){
        const icl8u *m = mask;
        T best = init;
        for(const ImgIterator<T> sR (s,si,an);sR.inRegionSubROI(); ++sR, ++m){
          if(*m && cmp(*sR,best)) best = *sR;
        }
        *d++ = best;
      }
    }
  }

  static Rect shrink_roi(Rect roi, const Size &maskSize){
    int dx = (maskSize.width-1)/2;
    int dy = (maskSize.height-1)/2;
    roi.x+=dx;
    roi.y+=dy;
    roi.width -= 2*dx;
    roi.height -= 2*dy;
    return roi;
  }

  template<class T>
  static void rep_border(Img<T> image, const Size &maskSize){
    Rect roi = shrink_roi(image.getImageRect(),maskSize);
    image.setROI(roi);
    ImgBorder::copy(&image);
  }
  
 
  template<class T>
  void MorphologicalOp::apply_t(const ImgBase *poSrc, ImgBase **ppoDst){
    const Img<T> &src = *poSrc->asImg<T>();
    Img<T> &dst = *(*ppoDst)->asImg<T>();
    Range<T> limits = Range<T>::limits();
    Size sizeSave;
    std::vector<icl8u> maskSave;
    if(m_eType == dilate3x3 || m_eType == erode3x3){
      sizeSave = getMaskSize();
      std::copy(getMask(),getMask()+sizeSave.getDim(),back_inserter(maskSave));
      setMask(Size(3,3));
    }
    switch (m_eType){
      case dilate:
      case dilate3x3:
      case dilateBorderReplicate: 
        morph_cpp(src,dst,*this,limits.minVal,std::greater<T>(),getMask()); 
        break;
      case erode: 
      case erode3x3:
      case erodeBorderReplicate: 
        morph_cpp(src,dst,*this,limits.maxVal,std::less<T>(),getMask()); 
        break;
      case tophatBorder:
      case blackhatBorder:{
        MorphologicalOp op(m_eType==tophatBorder ? openBorder : closeBorder,getMaskSize(),getMask());
        op.setClipToROI(getClipToROI());
        op.setCheckOnly(getCheckOnly());
        op.apply(poSrc,&m_openingAndClosingBuffer);
        BinaryArithmeticalOp sub(BinaryArithmeticalOp::subOp);
        sub.setClipToROI(getClipToROI());
        sub.setCheckOnly(getCheckOnly());
        
        Rect roi = poSrc->getROI();
        roi = shrink_roi(roi,getMaskSize());
        roi = shrink_roi(roi,getMaskSize());
        const ImgBase *srcROIAdapted = poSrc->shallowCopy(roi);
        
        if(m_eType == tophatBorder){
          //sub.apply(srcROIAdapted,m_openingAndClosingBuffer,ppoDst);
          sub.apply(srcROIAdapted,m_openingAndClosingBuffer,ppoDst);
        }else{
          //sub.apply(m_openingAndClosingBuffer,srcROIAdapted,ppoDst);
          sub.apply(m_openingAndClosingBuffer,srcROIAdapted,ppoDst);
        }

        delete srcROIAdapted;
        break;
      }
      case gradientBorder:{
        MorphologicalOp op(closeBorder,getMaskSize(),getMask());
        op.setClipToROI(getClipToROI());
        op.setCheckOnly(getCheckOnly());
        op.apply(poSrc,&m_gradientBorderBuffer_1);
        op.setOptype(openBorder);
        op.apply(poSrc,&m_gradientBorderBuffer_2);
        BinaryArithmeticalOp sub(BinaryArithmeticalOp::subOp);
        sub.setClipToROI(getClipToROI());
        sub.setCheckOnly(getCheckOnly());

        sub.apply(m_gradientBorderBuffer_1,m_gradientBorderBuffer_2,ppoDst);
        break;
      }
      case openBorder:
      case closeBorder:{
        MorphologicalOp op(m_eType==openBorder ? erode : dilate,getMaskSize(),getMask());
        op.setClipToROI(getClipToROI());
        op.setCheckOnly(getCheckOnly());
        op.apply(poSrc,&m_openingAndClosingBuffer);
        op.setOptype(m_eType==openBorder ? erode : dilate);
        op.apply(m_openingAndClosingBuffer,ppoDst);
        break;
      }
      default:
        ERROR_LOG("invalid optype: " << (int) m_eType);
    }
    if(!getClipToROI() && (m_eType == erodeBorderReplicate || m_eType == dilateBorderReplicate) ){
      rep_border(dst,getMaskSize());
    }else if (m_eType == erodeBorderReplicate || m_eType == dilateBorderReplicate){
      ERROR_LOG("border replication does not work if clipToROI is set [operation was applied, border replication was skipped]");
    }
    if(m_eType == dilate3x3 || m_eType == erode3x3){
      setMask(sizeSave,maskSave.data());
    }
  }

  
  void MorphologicalOp::apply (const ImgBase *poSrc, ImgBase **ppoDst){
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;
    
    switch (poSrc->getDepth()){
      case depth8u:
        apply_t<icl8u>(poSrc,ppoDst);
        break;
      case depth32f:
        apply_t<icl32f>(poSrc,ppoDst);
        break;
      default:
        ICL_INVALID_DEPTH;
        break;
    }
  }

  MorphologicalOp::MorphologicalOp (optype eOptype, const Size &maskSize,const icl8u *pcMask):
    m_openingAndClosingBuffer(0),m_gradientBorderBuffer_1(0),m_gradientBorderBuffer_2(0)
  {
    ICLASSERT_RETURN(maskSize.getDim());
    m_pcMask = 0;
    setMask (maskSize,pcMask);
    m_eType = eOptype;    
  }
  MorphologicalOp::~MorphologicalOp(){
    ICL_DELETE(m_pcMask);
    ICL_DELETE(m_openingAndClosingBuffer);
    ICL_DELETE(m_gradientBorderBuffer_1);
    ICL_DELETE(m_gradientBorderBuffer_2);
  }



#else //  HAVE_IPP is defined !
  MorphologicalOp::MorphologicalOp (optype eOptype, const Size &maskSize,const icl8u *pcMask){
    ICLASSERT_RETURN(maskSize.getDim());
    
    m_eType=eOptype;    
    m_pcMask = 0; 
    setMask (maskSize,pcMask);
    
    m_bMorphState8u=false;
    m_bMorphState32f=false;
    m_bMorphAdvState8u=false;
    m_bMorphAdvState32f=false;
    m_bHas_changed=true;
    m_bHas_changedAdv=true;

    m_pState8u = 0;
    m_pState32f = 0;
    m_pAdvState8u = 0;
    m_pAdvState32f = 0;
  }
  MorphologicalOp::~MorphologicalOp(){
    deleteMorphStates();
    ICL_DELETE(m_pcMask);
  }

  void MorphologicalOp::deleteMorphStates(){
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
    
  void MorphologicalOp::checkMorphAdvState8u(const Size roiSize){
    if (m_bHas_changedAdv){
      deleteMorphStates();
      ippiMorphAdvInitAlloc_8u_C1R(&m_pAdvState8u, roiSize, m_pcMask, m_oMaskSizeMorphOp, m_oAnchor);
      m_bMorphAdvState8u=true;
      m_bHas_changedAdv=false;
    }
  }

    void MorphologicalOp::checkMorphState8u(const Size roiSize){
    if (m_bHas_changed){
      deleteMorphStates();
      
      ippiMorphologyInitAlloc_8u_C1R(roiSize.width, m_pcMask, m_oMaskSizeMorphOp, m_oAnchor,&m_pState8u);
      
      m_bMorphState8u=true;
      m_bHas_changed=false;
    }
  }
  void MorphologicalOp::checkMorphAdvState32f(const Size roiSize){
    if (m_bHas_changedAdv){
      deleteMorphStates();
      ippiMorphAdvInitAlloc_32f_C1R(&m_pAdvState32f, roiSize, m_pcMask, m_oMaskSize, m_oAnchor);
      m_bMorphAdvState32f=true;
      m_bHas_changedAdv=false;
    }
  }
  
    void MorphologicalOp::checkMorphState32f(const Size roiSize){
    if (m_bHas_changed){
      deleteMorphStates();
      
      ippiMorphologyInitAlloc_32f_C1R(roiSize.width, m_pcMask, m_oMaskSizeMorphOp, m_oAnchor,&m_pState32f);
      
      m_bMorphState32f=true;
      m_bHas_changed=false;
    }
  }

  void MorphologicalOp::apply (const ImgBase *poSrc, ImgBase **ppoDst){
    FUNCTION_LOG("");
    if (!prepare (ppoDst, poSrc)) return;

    // DEBUG_LOG("before");
    //poSrc->print("source image");
    //(*ppoDst)->print("destination image");
    
    IppStatus s = ippStsNoErr;
    switch (poSrc->getDepth()){
      case depth8u:
        switch (m_eType){
          case dilate:s=ippiMorphologicalCall<icl8u,ippiDilate_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());break;
          case erode:s=ippiMorphologicalCall<icl8u,ippiErode_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());break;
          case dilate3x3:s=ippiMorphologicalCall3x3<icl8u,ippiDilate3x3_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());break;
          case erode3x3:s=ippiMorphologicalCall3x3<icl8u,ippiErode3x3_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>());break;
          case dilateBorderReplicate:checkMorphState8u(poSrc->getROISize());s=ippiMorphologicalBorderReplicateCall<icl8u,ippiDilateBorderReplicate_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pState8u);break;
          case erodeBorderReplicate:checkMorphState8u(poSrc->getROISize());s=ippiMorphologicalBorderReplicateCall<icl8u,ippiErodeBorderReplicate_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pState8u);break;
          case openBorder:checkMorphAdvState8u(poSrc->getROISize());s=ippiMorphologicalBorderCall<icl8u,ippiMorphOpenBorder_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pAdvState8u);break;
          case closeBorder:checkMorphAdvState8u(poSrc->getROISize());s=ippiMorphologicalBorderCall<icl8u,ippiMorphCloseBorder_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pAdvState8u);break;
          case tophatBorder:checkMorphAdvState8u(poSrc->getROISize());s=ippiMorphologicalBorderCall<icl8u,ippiMorphTophatBorder_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pAdvState8u);break;
          case blackhatBorder:checkMorphAdvState8u(poSrc->getROISize());s=ippiMorphologicalBorderCall<icl8u,ippiMorphBlackhatBorder_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pAdvState8u);break;
          case gradientBorder:checkMorphAdvState8u(poSrc->getROISize());s=ippiMorphologicalBorderCall<icl8u,ippiMorphGradientBorder_8u_C1R> (poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl8u>(),m_pAdvState8u);break;
        }
      break;
      case depth32f:
        switch (m_eType){
          case dilate:s=ippiMorphologicalCall<icl32f,ippiDilate_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());break;
          case erode:s=ippiMorphologicalCall<icl32f,ippiErode_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());break;
          case dilate3x3:s=ippiMorphologicalCall3x3<icl32f,ippiDilate3x3_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());break;
          case erode3x3:s=ippiMorphologicalCall3x3<icl32f,ippiErode3x3_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>());break;
          case dilateBorderReplicate:checkMorphState32f(poSrc->getROISize());s=ippiMorphologicalBorderReplicateCall<icl32f,ippiDilateBorderReplicate_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pState32f);break;
          case erodeBorderReplicate:checkMorphState32f(poSrc->getROISize());s=ippiMorphologicalBorderReplicateCall<icl32f,ippiErodeBorderReplicate_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pState32f);break;
          case openBorder:checkMorphAdvState32f(poSrc->getROISize());s=ippiMorphologicalBorderCall<icl32f,ippiMorphOpenBorder_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pAdvState32f);break;
          case closeBorder:checkMorphAdvState32f(poSrc->getROISize());s=ippiMorphologicalBorderCall<icl32f,ippiMorphCloseBorder_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pAdvState32f);break;
          case tophatBorder:checkMorphAdvState32f(poSrc->getROISize());s=ippiMorphologicalBorderCall<icl32f,ippiMorphTophatBorder_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pAdvState32f);break;
          case blackhatBorder:checkMorphAdvState32f(poSrc->getROISize());s=ippiMorphologicalBorderCall<icl32f,ippiMorphBlackhatBorder_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pAdvState32f);break;
          case gradientBorder:checkMorphAdvState32f(poSrc->getROISize());s=ippiMorphologicalBorderCall<icl32f,ippiMorphGradientBorder_32f_C1R> (poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),m_pAdvState32f);break;
        }
      break;
      default: ICL_INVALID_DEPTH; break;
    }
    if(s != ippStsNoErr){
      ERROR_LOG("IPP-Error: \"" << ippGetStatusString(s) << "\"");
    }
   

  }

  

  template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, const Ipp8u*, IppiSize, IppiPoint)>
  IppStatus MorphologicalOp::ippiMorphologicalCall (const Img<T> *src, Img<T> *dst) {
    for(int c=0; c < src->getChannels(); c++) {
      IppStatus s = ippiFunc(src->getROIData (c, this->m_oROIOffset),
                             src->getLineStep(),
                             dst->getROIData (c),
                             dst->getLineStep(),
                             dst->getROISize(), m_pcMask,m_oMaskSize, m_oAnchor
                             );
      if(s != ippStsNoErr) return s;
    }
    return ippStsNoErr;
  }

  template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize)>
  IppStatus MorphologicalOp::ippiMorphologicalCall3x3 (const Img<T> *src, Img<T> *dst) {
    for(int c=0; c < src->getChannels(); c++) {
      IppStatus s = ippiFunc(src->getROIData (c, this->m_oROIOffset),
                             src->getLineStep(),
                             dst->getROIData (c),
                             dst->getLineStep(),
                             dst->getROISize()
                             );
      if(s != ippStsNoErr) return s;
    }
    return ippStsNoErr;
  }

  template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, _IppiBorderType, IppiMorphState*)>
  IppStatus MorphologicalOp::ippiMorphologicalBorderReplicateCall (const Img<T> *src, Img<T> *dst,IppiMorphState* state) {
    for(int c=0; c < src->getChannels(); c++) {
      IppStatus s = ippiFunc(src->getROIData (c, this->m_oROIOffset),
                             src->getLineStep(),
                             dst->getROIData (c),
                             dst->getLineStep(),
                             dst->getROISize(),
                             ippBorderRepl,
                             state
                             );
      if(s != ippStsNoErr) return s;
    }
    return ippStsNoErr;
  }

  template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
  IppStatus MorphologicalOp::ippiMorphologicalBorderCall (const Img<T> *src, Img<T> *dst, IppiMorphAdvState* advState) {
    for(int c=0; c < src->getChannels(); c++) {
      IppStatus s = ippiFunc(src->getROIData (c, this->m_oROIOffset),
                             src->getLineStep(),
                             dst->getROIData (c),
                             dst->getLineStep(),
                             dst->getROISize(),
                             ippBorderRepl,
                             advState
                             );
      if(s != ippStsNoErr) return s;
    }
    return ippStsNoErr;
  }

  // }}}
#endif

  void MorphologicalOp::setMask (Size maskSize,const icl8u* pcMask) {
    //make maskSize odd:
    maskSize = ((maskSize/2)*2)+Size(1,1);
    
    if(m_eType >= 6){
      NeighborhoodOp::setMask (Size(1,1));
    }else{
      NeighborhoodOp::setMask (maskSize);
    }

    ICL_DELETE(m_pcMask);
    m_pcMask = new icl8u[maskSize.getDim()];
    if(pcMask){
      std::copy(pcMask,pcMask+maskSize.getDim(),m_pcMask);
    }else{
      std::fill(m_pcMask,m_pcMask+maskSize.getDim(),255);
    }

    m_oMaskSizeMorphOp=maskSize;
    m_bHas_changed=true;
    m_bHas_changedAdv=true;
  }
  
  const icl8u* MorphologicalOp::getMask() const{
    return m_pcMask;
  }
  Size MorphologicalOp::getMaskSize() const{
    return m_oMaskSizeMorphOp;
  }
  void MorphologicalOp::setOptype(optype type){
    m_eType=type;
    setMask(m_oMaskSizeMorphOp,m_pcMask);
  }
  MorphologicalOp::optype MorphologicalOp::getOptype() const{
    return m_eType;
  }


}
