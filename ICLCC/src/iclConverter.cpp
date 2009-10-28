#include <iclCCFunctions.h>
#include <iclConverter.h>


namespace icl{


  Converter::Converter(bool bROIOnly) : 
    // {{{ open

    m_poSizeBuffer(0),m_poCCBuffer(0),m_poDepthBuffer(0),m_poROIBuffer(0), 
    m_poColorBuffer(0),m_bROIOnly(bROIOnly),m_eOpOrder(orderScaleConvertCC),
    m_scaleMode(interpolateNN){
    FUNCTION_LOG("");
  }

  // }}}

  Converter::Converter(Converter::oporder o, bool bROIOnly):
    m_poSizeBuffer(0),m_poCCBuffer(0),m_poDepthBuffer(0),m_poROIBuffer(0), 
    m_poColorBuffer(0),m_bROIOnly(bROIOnly),m_eOpOrder(o),m_scaleMode(interpolateNN){
    FUNCTION_LOG("");
    // {{{ open
    
  }
  // }}}
  
  Converter::Converter(const ImgBase *srcImage, ImgBase *dstImage, bool applyToROIOnly):
    // {{{ open
    m_poSizeBuffer(0), m_poCCBuffer(0),m_poDepthBuffer(0),m_poROIBuffer(0),
    m_poColorBuffer(0), m_bROIOnly(applyToROIOnly),m_eOpOrder(orderScaleConvertCC),
    m_scaleMode(interpolateNN){
    apply(srcImage,dstImage);
  }

  // }}}

  Converter::~Converter(){
    // {{{ open

    FUNCTION_LOG("");
    if(m_poSizeBuffer)delete m_poSizeBuffer;
    if(m_poCCBuffer)delete m_poCCBuffer;
    if(m_poDepthBuffer)delete m_poDepthBuffer;
    if(m_poROIBuffer)delete m_poROIBuffer;
    if(m_poColorBuffer)delete m_poColorBuffer;
  }

  // }}}

  void Converter::dynamicConvert(const ImgBase *src, ImgBase *dst){
    // {{{ open
    //case depth##D: src->convert<icl##D>(dst->asImg<icl##D>());
    switch(dst->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: src->convert<icl##D>(dst->asImg<icl##D>()); break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
  }

  // }}}

  void Converter::apply(const ImgBase *poSrc, ImgBase *poDst){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( poDst );

    poDst->setFullROI();
    
    
    int iScalePos = m_eOpOrder/100;
    int iConvertPos = (m_eOpOrder-100*iScalePos)/10;
    int iCCPos = m_eOpOrder-100*iScalePos-10*iConvertPos;
      
    int iNeedDepthConversion = poSrc->getDepth() != poDst->getDepth();
      
    int iNeedColorConversion =  poSrc->getFormat() != poDst->getFormat();
                                //poSrc->getFormat() != formatMatrix && 
                                //poDst->getFormat() != formatMatrix && 
                                
    
    int iNeedSizeConversion;
    if(m_bROIOnly){ 
      iNeedSizeConversion = poSrc->getROISize() != poDst->getSize();
    }else{
      iNeedSizeConversion = poSrc->getSize() != poDst->getSize();
    }
    
    static const int NOTHING = 0;
    static const int SIZE_ONLY = 1;
    static const int COLOR_ONLY = 2;
    static const int SIZE_AND_COLOR = 3;
    static const int DEPTH_ONLY = 4;
    static const int DEPTH_AND_SIZE = 5;
    static const int DEPTH_AND_COLOR = 6;
    static const int DEPTH_SIZE_AND_COLOR = 7;
    switch(iNeedSizeConversion + (iNeedColorConversion << 1) + (iNeedDepthConversion << 2) ){
      case NOTHING:
        SECTION_LOG("deep copy only");
        if(m_bROIOnly){
          poSrc->deepCopyROI(&poDst);
        }else{
          poSrc->deepCopy(&poDst);
        }
        break;
      case SIZE_ONLY:
        SECTION_LOG("size conversion only");
        if(m_bROIOnly){
          poSrc->scaledCopyROI(&poDst,m_scaleMode);
        }else{
          poSrc->scaledCopy(&poDst,m_scaleMode);
        }
        break;
      case COLOR_ONLY:
        SECTION_LOG("color conversion only");
      case DEPTH_AND_COLOR:
        SECTION_LOG("depth and color conversion only");
        if(m_bROIOnly){
          poSrc->deepCopyROI(&m_poROIBuffer);
          this->cc(m_poROIBuffer,poDst);
        }else{
          this->cc(poSrc,poDst);
        }
        break;
      case SIZE_AND_COLOR:
        SECTION_LOG("size and color conversion");
      case DEPTH_SIZE_AND_COLOR:
        SECTION_LOG("depth size and color conversion");
        if(iScalePos < iCCPos){
          ensureCompatible(&m_poSizeBuffer,poSrc->getDepth(), poDst->getSize(), poSrc->getChannels(), poSrc->getFormat()); 
          if(m_bROIOnly){
            poSrc->scaledCopyROI(&m_poSizeBuffer,m_scaleMode);
          }else{
            poSrc->scaledCopy(&m_poSizeBuffer,m_scaleMode);
          }
          this->cc(m_poSizeBuffer,poDst);
        }else{
          if(m_bROIOnly){
            poSrc->deepCopyROI(&m_poROIBuffer);
            ensureCompatible(&m_poColorBuffer,poDst->getDepth(),m_poROIBuffer->getSize(),poDst->getChannels(), poDst->getFormat());
            cc(m_poROIBuffer,m_poColorBuffer);
            m_poColorBuffer->scaledCopy(&poDst,m_scaleMode);
          }else{
            ensureCompatible(&m_poColorBuffer,poDst->getDepth(),poSrc->getSize(),poDst->getChannels(), poDst->getFormat());
            cc(poSrc,m_poColorBuffer);
            m_poColorBuffer->scaledCopy(&poDst,m_scaleMode);
          }
        }       
        break;
      case DEPTH_ONLY:
        SECTION_LOG("depth conversion");
        dynamicConvert(poSrc,poDst);
        break;
      case DEPTH_AND_SIZE:
        if(iConvertPos < iScalePos){
          ensureCompatible(&m_poDepthBuffer,poDst->getDepth(), poSrc->getSize(), poSrc->getChannels(), poSrc->getFormat());
          dynamicConvert(poSrc,m_poDepthBuffer);
          if(m_bROIOnly){
            m_poDepthBuffer->setROI(poSrc->getROI());
            m_poDepthBuffer->scaledCopyROI(&poDst,m_scaleMode);
          }else{
            m_poDepthBuffer->scaledCopy(&poDst,m_scaleMode);
          }
        }else{
          ensureCompatible(&m_poSizeBuffer,poSrc->getDepth(),poDst->getSize(),poSrc->getChannels(), poSrc->getFormat());
          if(m_bROIOnly){
            poSrc->scaledCopyROI(&m_poSizeBuffer,m_scaleMode);
          }else{
            poSrc->scaledCopy(&m_poSizeBuffer,m_scaleMode);
          }
          dynamicConvert(m_poSizeBuffer,poDst);
        }
        break;
    }
  }

  // }}}
 
  void Converter::cc(const ImgBase *srcIn, ImgBase *dst){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN( srcIn );
    ICLASSERT_RETURN( dst );

    const ImgBase *src = srcIn;
    if(!srcIn->hasFullROI()){
      src = srcIn->shallowCopy(srcIn->getImageRect());
    }
    
    if(cc_available(src->getFormat(), dst->getFormat()) == ccEmulated){
      SECTION_LOG("optimized emulated cross color conversion using Converter objects buffer");
      ensureCompatible(&m_poCCBuffer,src->getDepth(), src->getSize(), formatRGB);
      cc(src,m_poCCBuffer);
      cc(m_poCCBuffer,dst);
    }else{
      SECTION_LOG("passing directly icl::cc");
      
      icl::cc(src,dst);
    }

    if(srcIn != src){
      ICL_DELETE(src);
    }
  }

  // }}}


  void Converter::setScaleMode(scalemode scaleMode){
    m_scaleMode = scaleMode;
  }
  

}
