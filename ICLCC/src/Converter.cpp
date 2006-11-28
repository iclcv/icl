#include <ICLcc.h>
#include <Converter.h>

namespace icl{
  Converter::Converter(bool bROIOnly) : 
     m_poDepthBuffer(0), m_poSizeBuffer(0), m_bROIOnly(bROIOnly) {}

  Converter::~Converter(){
    if(m_poDepthBuffer)delete m_poDepthBuffer;
    if(m_poSizeBuffer)delete m_poSizeBuffer;
  }

  void Converter::convert(ImgBase *poDst, ImgBase *poSrc){
    format eSrcFmt = poSrc->getFormat();
    format eDstFmt = poDst->getFormat();
    depth eSrcDepth = poSrc->getDepth();
    depth eDstDepth = poDst->getDepth();
    Size  srcSize   = m_bROIOnly ? poSrc->getROISize() : poSrc->getSize();

    int iNeedDepthConversion = eSrcDepth!=eDstDepth;
    int iNeedSizeConversion  = poDst->getSize() != srcSize ||
                               // cropping only:
                               (m_bROIOnly && poSrc->getSize() != poDst->getSize());
    int iNeedColorConversion = eSrcFmt != formatMatrix && 
                               eDstFmt != formatMatrix && 
                               eSrcFmt != eDstFmt;
    
    //---- convert depth ----------------- 
    ImgBase *poCurSrc=poSrc;
    if(iNeedDepthConversion){ 
      // test if other convesion steps will follow:
      if(iNeedSizeConversion || iNeedColorConversion){
         // if other conversion steps follow, we first convert the depth
         // and store the result in an intermediate buffer: m_poDepthBuffer
         ensureCompatible(&m_poDepthBuffer,eDstDepth,srcSize,
                          poCurSrc->getChannels(),eSrcFmt);
         if (m_bROIOnly) poCurSrc->deepCopyROI(m_poDepthBuffer);
         else poCurSrc->deepCopy(m_poDepthBuffer);
         poCurSrc=m_poDepthBuffer;
      }else{
         // if only depth conversion is desired, we can simply call deepCopy
         if (m_bROIOnly) poCurSrc->deepCopyROI(poDst);
         else poCurSrc->deepCopy(poDst);
         return;
      }
    }

    //---- convert size ----------------- 
    if(iNeedSizeConversion){
      if(iNeedColorConversion){
        // if color conversion follows, we scale the image into the buffer
        // m_poSizeBuffer first
        ensureCompatible (&m_poSizeBuffer,poCurSrc->getDepth(), 
                          poDst->getSize(), poCurSrc->getChannels(), eSrcFmt);
        if (m_bROIOnly) poCurSrc->scaledCopyROI(m_poSizeBuffer);
        else poCurSrc->scaledCopy(m_poSizeBuffer);
        poCurSrc=m_poSizeBuffer;
      }else{
        // if only scaling is desired, we can return after it
        if (m_bROIOnly) poCurSrc->scaledCopyROI(poDst);
        else poCurSrc->scaledCopy(poDst);
        return;
      }     
    }

    if(iNeedColorConversion){
       //---- convert color finally ----------------- 
       iclcc(poDst,poCurSrc);
       return;
    }

    //---- no changed needed at all: do deep / shallow copy
    poCurSrc->shallowCopy(&poDst);
  }
}
