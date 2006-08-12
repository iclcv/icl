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
  m_iChannels((iChannels <= 0) ? iclGetChannelsOfFormat(eFormat) : iChannels),
  m_eFormat(eFormat),m_eDepth(eDepth)
{
  FUNCTION_LOG("ICLBase("<< iWidth <<","<< iHeight <<","<< iclTranslateformat(eFormat) 
               <<","<< (int)eDepth << "," << iChannels << ")  this:" << this); 
  m_oSize.width = iWidth; m_oSize.height = iHeight;
  delROI();
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

  void ICLBase::setROI(int iX,int iY, int iW,int iH){
    FUNCTION_LOG("setROI("<< iX <<"," << iY << "," << iW << "," << iH << ")");

    if (iW <= 0) iW = getWidth () + iW;
    if (iH <= 0) iH = getHeight () + iH;
    m_oROIsize.width = iW = std::min (std::max (1, iW), getWidth());
    m_oROIsize.height = iH = std::min (std::max (1, iH), getHeight());

    if (iX < 0) iX = getWidth () - iW + iX;
    if (iY < 0) iY = getHeight () - iH + iY;
    m_oROIoffset.x = iX = std::min (std::max (0, iX), getWidth() - iW);
    m_oROIoffset.y = iY = std::min (std::max (0, iY), getHeight() - iH);
  }
  void ICLBase::setROI(const ICLpoint &oOffset, const ICLsize &oSize) {
     m_oROIoffset = oOffset;
     m_oROIsize = oSize;
  }
  void ICLBase::delROI() {
     m_oROIoffset.x = m_oROIoffset.y = 0;
     m_oROIsize = m_oSize;
  }
  
  void ICLBase::getROI(ICLpoint &offset, ICLsize &size) const {
     offset = m_oROIoffset;
     size = m_oROIsize;
  }
  void ICLBase::getROI(int &riX, int &riY, int &riWidth, int &riHeight) const{
    FUNCTION_LOG("");    
    riX = m_oROIoffset.x; riY = m_oROIoffset.y;
    riWidth = m_oROIsize.width; riHeight = m_oROIsize.height;
  }
      
  int ICLBase::hasFullROI() const
  {
    FUNCTION_LOG("");    
    return (m_oROIoffset.x==0 &&  m_oROIoffset.y==0 && 
            m_oROIsize.width==m_oSize.width &&  m_oROIsize.height == m_oSize.height);
  }
  
// }}}

} //namespace icl


