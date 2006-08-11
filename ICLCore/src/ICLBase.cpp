/*
  ICLBase.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "ICLBase.h"
#include "ICL.h"

namespace icl {

// {{{ constructor / destructor 

ICLBase::ICLBase(int iWidth, 
                 int iHeight, 
                 iclformat eFormat,
                 icldepth eDepth,
                 int iChannels):
  m_iWidth(iWidth),m_iHeight(iHeight),
  m_iChannels((iChannels <= 0) ? iclGetChannelsOfFormat(eFormat) : iChannels),
  m_eFormat(eFormat),m_eDepth(eDepth),
  m_vecROI(std::vector<int>(4))
{
  FUNCTION_LOG("ICLBase("<< iWidth <<","<< iHeight <<","<< iclTranslateformat(eFormat) <<","<< (int)eDepth << "," << iChannels << ")  this:" << this); 
  setROI(0,0,iWidth,iHeight);
}


ICLBase::~ICLBase()
{
  FUNCTION_LOG("");
}

// }}} 

// {{{ setter functions

void ICLBase::setFormat(iclformat eFormat)
{
  FUNCTION_LOG("setFormat(" << iclTranslateFormat(eFormat) << ")");
  if(eFormat != formatMatrix)
    {
      int nChannels = iclGetChannelsOfFormat(eFormat);
      if(nChannels != m_iChannels){
        setNumChannels(nChannels);
      }
    }
  m_eFormat=eFormat;
}

// }}}

// {{{ utillity functions

ICLBase* ICLBase::shallowCopy(ICLBase* poDst) const {
  FUNCTION_LOG("");
   iclEnsureDepth (&poDst, getDepth ());
   if (getDepth() == depth8u)
      *poDst->asIcl8u() = *this->asIcl8u();
   else
      *poDst->asIcl32f() = *this->asIcl32f();
   return poDst;
}

void ICLBase::print(string sTitle) const
{
  FUNCTION_LOG(sTitle);
  int iX,iY,iW,iH;
  getROI(iX,iY,iW,iH);
  printf(   " -----------------------------------------\n"
            "| image: %s\n"
            "| width: %d, height: %d, channels: %d\n"
            "| depth: %s, format: %s\n"
            "| ROI: x: %d, y: %d, w: %d, h: %d \n",        
            sTitle.c_str(),
            getWidth(),getHeight(),getChannels(),
            getDepth()==depth8u ? "depth8u" : "depth32f",iclTranslateFormat(m_eFormat).c_str(),
            iX,iY,iW,iH
            
        );
  if(m_eDepth == depth8u){
    for(int i=0;i<m_iChannels;i++){
      printf("| channel: %d, min: %d, max:%d \n",i,asIcl8u()->getMin(i),asIcl8u()->getMax(i));
    }
  }else{
    for(int i=0;i<m_iChannels;i++){
      printf("| channel: %d, min: %f, max:%f \n",i,asIcl32f()->getMin(i),asIcl32f()->getMax(i));
    }
  }
  printf(" -----------------------------------------\n");
 

}

// }}}

// {{{ ROI functions

  void ICLBase::setROI(int iX, int iY,int iW,int iH){
    FUNCTION_LOG("setROI("<< iX <<"," << iY << "," << iW << "," << iH << ")");
    m_vecROI[2] = std::min (std::max (0, iW), getWidth());
    m_vecROI[3] = std::min (std::max (0, iH), getHeight());
    m_vecROI[0] = std::min (std::max (0, iX), getWidth() - m_vecROI[2]);
    m_vecROI[1] = std::min (std::max (0, iY), getHeight() - m_vecROI[3]); 
  }

  void ICLBase::setROIOffset(int iX, int iY){
    FUNCTION_LOG("setROIOffset("<< iX << "," << iY << ")");
    setROI(iX,iY,m_vecROI[2],m_vecROI[3]);
  }
  
  void ICLBase::setROISize(int iWidth, int iHeight){
    FUNCTION_LOG("setROISize("<< iWidth << "," << iHeight << ")");
    setROI(m_vecROI[0],m_vecROI[1],iWidth,iHeight);
  }

  void ICLBase::setROI(const std::vector<int> &oROIRect){
    FUNCTION_LOG("");    
    ICLASSERT_RETURN(oROIRect.size() == 4);
    setROI(oROIRect[0],oROIRect[1],oROIRect[2],oROIRect[3]);
  }
  
  void ICLBase::getROI(int &riX, int &riY, int &riWidth, int &riHeight) const{
    FUNCTION_LOG("");    
    riX = m_vecROI[0];
    riY = m_vecROI[1];
    riWidth = m_vecROI[2];
    riHeight = m_vecROI[3];
  }
      
  void ICLBase::getROIOffset(int &riX, int &riY) const{
    FUNCTION_LOG("");    
    riX = m_vecROI[0];
    riY = m_vecROI[1];
  }
      
  void ICLBase::getROISize(int &riWidth, int &riHeight) const{
    FUNCTION_LOG("");        
    riWidth = m_vecROI[2];
    riHeight = m_vecROI[3];
  }

  const std::vector<int>& ICLBase::getROI() const{
    FUNCTION_LOG("");    
    return m_vecROI;
  }
  
  int ICLBase::hasROI() const
  {
    FUNCTION_LOG("");    
    return ( m_vecROI[0]==0 &&  m_vecROI[1]==0 &&  m_vecROI[2]==m_iWidth &&  m_vecROI[3] == m_iHeight);
  }
  
  void ICLBase::delROI()
  { 
    FUNCTION_LOG("");    
    setROI(0,0,m_iWidth,m_iHeight);
  }
  
// }}}

} //namespace icl


