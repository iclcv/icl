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
             Format eFormat,
             Depth
 eDepth,
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
                 << "," << (eDepth==depth8u) ? (char*)"depth8u" : (char*)"depth32f" 
                 << "," << iChannels << ")  this:" << this); 
    
  }


ImgI::~ImgI()
{
  FUNCTION_LOG("");
}

// }}} 

// {{{ setter functions

void ImgI::setFormat(Format eFormat)
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
      *(*ppoDst)->asImg<iclbyte>() = *this->asImg<iclbyte>();
    }
  else
    {
      *(*ppoDst)->asImg<iclfloat>() = *this->asImg<iclfloat>();
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
      printf("| channel: %d, min: %f, max:%f \n",i,asImg<iclfloat>()->getMin(i),asImg<iclfloat>()->getMax(i));
    }
  }
  printf(" -----------------------------------------\n");
 

}

// }}}

// {{{ ROI functions

void ImgI::setROISize(const Size &s){
  FUNCTION_LOG("setROISize("<< s.width << "," << s.height << ")");

  int iW = s.width <=  0 ? getSize().width  + s.width  : s.width;
  int iH = s.height <= 0 ? getSize().height + s.height : s.height;
  m_oROISize.width  = std::min (std::max (1, iW), getSize().width);
  m_oROISize.height = std::min (std::max (1, iH), getSize().height);
}

void ImgI::setROIOffset(const Point &p){
  FUNCTION_LOG("setROIOffset("<< p.x << "," << p.y << ")");
  
  int x = p.x < 0 ? getSize().width  - getROISize().width  + p.x : p.x;
  int y = p.y < 0 ? getSize().height - getROISize().height + p.y : p.y;
  m_oROIOffset.x =  std::min (std::max (0, x), getSize().width  - getROISize().width);
  m_oROIOffset.y =  std::min (std::max (0, y), getSize().height - getROISize().height);
}


// }}}

template <class T>
Img<T> *ImgI::convertTo( Img<T>* poDst) const {
  return poDst;
}

  template Img<iclbyte>* ImgI::convertTo<iclbyte>(Img<iclbyte>*) const;
  template Img<iclfloat>* ImgI::convertTo<iclfloat>(Img<iclfloat>*) const;

} //namespace icl
