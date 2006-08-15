#include "Converter.h"
#include "ICLcc.h"

namespace icl{
  Converter::Converter():m_poDepthBuffer(0),m_poSizeBuffer(0){}
  Converter::~Converter(){
    if(m_poDepthBuffer)delete m_poDepthBuffer;
    if(m_poSizeBuffer)delete m_poSizeBuffer;
  }

  void Converter::convert(ImgI *poDst, ImgI *poSrc){
    Format eSrcFmt = poSrc->getFormat();
    Format eDstFmt = poDst->getFormat();
    Depth eSrcDepth = poSrc->getDepth();
    Depth eDstDepth = poDst->getDepth();
    int iNeedDepthConversion = eSrcDepth!=eDstDepth;
    int iNeedSizeConversion = poDst->getSize() != poSrc->getSize();
    int iNeedColorConversion = eSrcFmt != formatMatrix && eDstFmt != formatMatrix && eSrcFmt != eDstFmt;
    
    //---- convert depth ----------------- 
    ImgI *poNextSrcImage=poSrc;
    if(iNeedDepthConversion){ 
      // test if other convesion steps will follow:
      if(iNeedSizeConversion || iNeedColorConversion){
        ensureDepth(&m_poDepthBuffer,eDstDepth);
        m_poDepthBuffer->setFormat(eSrcFmt);
        m_poDepthBuffer->resize(poNextSrcImage->getSize());
        poNextSrcImage->deepCopy(m_poDepthBuffer);
        poNextSrcImage=m_poDepthBuffer;
      }else{
        poNextSrcImage->deepCopy(poDst);
        return;
      }
    }

    //---- convert size ----------------- 
    if(iNeedSizeConversion){
      if(iNeedColorConversion){
        ensureDepth(&m_poSizeBuffer,poNextSrcImage->getDepth());
        m_poSizeBuffer->renew(poDst->getSize(),poDst->getChannels());
        m_poSizeBuffer->setFormat(eSrcFmt);
        poNextSrcImage->scaledCopy(m_poSizeBuffer);
        //---- convert color ----------------- 
        iclcc(poDst,m_poSizeBuffer);
      }else{
        poNextSrcImage->scaledCopy(poDst);
      }      
    }else if(iNeedColorConversion){
      iclcc(poDst,poNextSrcImage);
    }else if(!iNeedDepthConversion){
      poNextSrcImage->deepCopy(poDst);
    }
    
  }
}
