#include "ICLConverter.h"
#include "ICLcc.h"

namespace icl{
  IclConverter::IclConverter():m_poDepthBuffer(0),m_poSizeBuffer(0){}
  IclConverter::~IclConverter(){
    if(m_poDepthBuffer)delete m_poDepthBuffer;
    if(m_poSizeBuffer)delete m_poSizeBuffer;
  }

  void IclConverter::convert(ICLBase *poDst, ICLBase *poSrc){
    iclformat eSrcFmt = poSrc->getFormat();
    iclformat eDstFmt = poDst->getFormat();
    icldepth eSrcDepth = poSrc->getDepth();
    icldepth eDstDepth = poDst->getDepth();
    int iNeedDepthConversion = eSrcDepth!=eDstDepth;
    int iNeedSizeConversion = poDst->getWidth() != poSrc->getWidth() || poDst->getHeight() != poSrc->getHeight();
    int iNeedColorConversion = eSrcFmt != formatMatrix && eDstFmt != formatMatrix && eSrcFmt != eDstFmt;
    
    
    //---- convert depth ----------------- 
    ICLBase *poNextSrcImage=poSrc;
    if(iNeedDepthConversion){ 
      // test if other convesion steps will follow:
      if(iNeedSizeConversion || iNeedColorConversion){
        iclEnsureDepth(&m_poDepthBuffer,eDstDepth);
        m_poDepthBuffer->setFormat(eSrcFmt);
        m_poDepthBuffer->resize(poNextSrcImage->getWidth(),poNextSrcImage->getHeight());
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
        iclEnsureDepth(&m_poSizeBuffer,poNextSrcImage->getDepth());
        m_poSizeBuffer->renew(poDst->getWidth(),poDst->getHeight(),poDst->getChannels());
        m_poSizeBuffer->setFormat(eSrcFmt);
        poNextSrcImage->scaledCopy(m_poSizeBuffer);

        //---- convert color ----------------- 
        iclcc(poDst,m_poSizeBuffer);
      }else{
        poNextSrcImage->scaledCopy(poDst);
      }      
    }
  }



  void ICLBase::print(string sTitle)
  {
    printf(   " -----------------------------------------\n"
              "| image: %s\n"
              "| width: %d, height: %d, channels: %d\n"
              "| depth: %s, format: %s\n"
              " -----------------------------------------\n",
              sTitle.c_str(),
              getWidth(),getHeight(),getChannels(),
              getDepth()==depth8u ? "depth8u" : "depth32f",iclTranslateFormat(m_eFormat).c_str()
              );
  }
  
}
