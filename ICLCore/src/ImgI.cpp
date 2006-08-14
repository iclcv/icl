/*
  ImgI.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "ImgI.h"
#include "Img.h"

namespace icl {

// {{{ constructor / destructor 

  ImgI::ImgI(const Size &s,
             iclformat eFormat,
             icldepth eDepth,
             int iChannels):
    m_iChannels((iChannels <= 0) ? getChannelsOfFormat(eFormat) : iChannels),
    m_oSize(s),
    m_eFormat(eFormat),
    m_eDepth(eDepth),
    m_oROISize(s)
  {
    FUNCTION_LOG("ImgI(" << s.width 
                 << "," << s.height 
                 << "," << translateformat(eFormat) 
                 << "," << (eDepth==depth8u) ? "depth8u" : "depth32f" 
                 << "," << iChannels << ")  this:" << this); 
    
  }


ImgI::~ImgI()
{
  FUNCTION_LOG("");
}

// }}} 

// {{{ setter functions

void ImgI::setFormat(iclformat eFormat)
{
  FUNCTION_LOG("setFormat(" << translateFormat(eFormat) << ")");
  if(eFormat != formatMatrix)
    {
      int nChannels = getChannelsOfFormat(eFormat);
      if(nChannels != m_iChannels){
        setNumChannels(nChannels);
      }
    }
  m_eFormat=eFormat;
}

// }}}

// {{{ utillity functions

void ImgI::shallowCopy(ImgI** ppoDst) const {
  FUNCTION_LOG("");
  ensureDepth (ppoDst, getDepth ());
  
  if (getDepth() == depth8u)
    {
      *poDst->asImg<iclbyte>() = *this->asImg<iclbyte>();
    }
  else
    {
      *poDst->asImg<iclfloat>() = *this->asImg<iclfloat>();
    }
  
}

void ImgI::print(string sTitle) const
{
  FUNCTION_LOG(sTitle);
  printf(   " -----------------------------------------\n"
            "| image: %s\n"
            "| width: %d, height: %d, channels: %d\n"
            "| depth: %s, format: %s\n"
            "| ROI: x: %d, y: %d, w: %d, h: %d \n",        
            sTitle.c_str(),
            getSize().width,getSize().height,getChannels(),
            getDepth()==depth8u ? "depth8u" : "depth32f",
            translateFormat(m_eFormat).c_str(),
            getROI().x, getROI().y,getROI().width, getROI().height);
  if(m_eDepth == depth8u){
    for(int i=0;i<m_iChannels;i++){
      printf("| channel: %d, min: %d, max:%d \n",i,asImg<iclbyte>()->getMin(i),asImg<iclbyte>()->getMax(i));
    }
  }else{
    for(int i=0;i<m_iChannels;i++){
      printf("| channel: %d, min: %f, max:%f \n",i,asImg<iclfloat>()->getMin(i),asImg<iclfloat>->getMax(i));
    }
  }
  printf(" -----------------------------------------\n");
 

}

// }}}

// {{{ ROI functions

void setROISSize(const Size &s){
  FUNCTION_LOG("setROISize("<< s.width << "," << s.height << ")");

  int iW = s.width <=  0 ? getSize.width()  + s.width;
  int iH = s.height <= 0 ? getSize.height() + s.height;
  m_oROISize.width  = std::min (std::max (1, iW), getSize().width);
  m_oROISize.height = std::min (std::max (1, iH), getSize().height);
}

void setROIOffset(const Point &p){
  FUNCTION_LOG("setROIOffset("<< p.x << "," << p.y << ")");
  
  if (p.x < 0) p.x = getSize().width  - getROISize().width  + p.x;
  if (p.y < 0) p.y = getSize().height - getROISize().height + p.y;
  m_oROIOffset.x =  std::min (std::max (0, iX), getSize().width  - getROISize().width);
  m_oROIOffset.y =  std::min (std::max (0, iY), getSize().height - getROISize().height);
}


// }}}

} //namespace icl
