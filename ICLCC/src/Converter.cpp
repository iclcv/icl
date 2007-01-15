#include <ICLCC.h>
#include <Converter.h>

namespace icl{
  Converter::Converter(bool bROIOnly,bool bUseShallowCopy) : 
    m_poBuffer(0),m_poCCBuffer(0), m_bROIOnly(bROIOnly), m_bUseShallowCopy(bUseShallowCopy) {
    FUNCTION_LOG("");
  }
  
  Converter::Converter(ImgBase *srcImage, ImgBase *dstImage, bool applyToROIOnly, bool useShallowCopy):
    m_poBuffer(0), m_poCCBuffer(0), m_bROIOnly(applyToROIOnly), m_bUseShallowCopy(useShallowCopy) {
      apply(srcImage,dstImage);
  }

  Converter::~Converter(){
    FUNCTION_LOG("");
    if(m_poBuffer)delete m_poBuffer;
    if(m_poCCBuffer)delete m_poCCBuffer;
  }

  void Converter::apply(ImgBase *poSrc, ImgBase *poDst){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( poDst );
    
    int iNeedColorConversion = 
      poSrc->getFormat() != formatMatrix && 
      poDst->getFormat() != formatMatrix && 
      poSrc->getFormat() != poDst->getFormat();

    int iNeedSizeConversionOrCopy;
    if(m_bROIOnly){ 
      iNeedSizeConversionOrCopy = 
        poSrc->getROISize() != poDst->getSize() || //scaling from src-roi to dst
        !poSrc->hasFullROI() || // used to crop the source images roi
        (!iNeedColorConversion && poSrc->getDepth() != poDst->getDepth()); //depth conversion
    }else{
      iNeedSizeConversionOrCopy = 
        poSrc->getSize() != poDst->getSize() || // scaling form src to dst
        (!iNeedColorConversion && poSrc->getDepth() != poDst->getDepth()); //depth conversion
    }
    
    static const int NOTHING = 0;
    static const int SIZE_ONLY = 1;
    static const int COLOR_ONLY = 2;
    static const int SIZE_AND_COLOR = 3;
    switch(iNeedSizeConversionOrCopy + (iNeedColorConversion << 1)){
      case NOTHING:
        if(m_bUseShallowCopy){
          SECTION_LOG("shallow copy");  
          poSrc->shallowCopy(&poDst);
        }else{
          SECTION_LOG("deep copy");     
          poSrc->deepCopy(poDst);
        }
        break;
      case SIZE_ONLY:
        SECTION_LOG("scaling or copy only");
        poSrc->scaledCopyROI(poDst);
        break;
      case COLOR_ONLY:
        SECTION_LOG("color conversion only");
        this->cc(poSrc,poDst);
        break;
      case SIZE_AND_COLOR:
        SECTION_LOG("color conversion and copy/scaling");
        ensureCompatible(&m_poBuffer,poSrc->getDepth(), poDst->getSize(), poSrc->getChannels(), poSrc->getFormat()); 
        poSrc->scaledCopyROI(m_poBuffer);
        this->cc(m_poBuffer,poDst);
        break;
    }
  }
  
  void Converter::cc(ImgBase *src, ImgBase *dst){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( src );
    ICLASSERT_RETURN( dst );
    
    if(cc_available(src->getFormat(), dst->getFormat()) == ccEmulated){
      SECTION_LOG("optimized emulated cross color conversion using Converter objects buffer");
      ensureCompatible(&m_poCCBuffer,src->getDepth(), src->getSize(), formatRGB);
      cc(src,m_poCCBuffer);
      cc(m_poCCBuffer,dst);
    }else{
      SECTION_LOG("passing directly icl::cc");
      icl::cc(src,dst);
    }
  }
}
