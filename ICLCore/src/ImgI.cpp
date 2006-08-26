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

ImgI::ImgI(const Size &s, format eFormat, depth eDepth, int iChannels):
   m_iChannels(eFormat == formatMatrix ? (std::max(0, iChannels)) 
               : getChannelsOfFormat(eFormat)),
   m_oSize(s), m_eFormat(eFormat), m_eDepth(eDepth), m_oROISize(s)
{
   FUNCTION_LOG("ImgI(" << s.width 
                << "," << s.height 
                << "," << translateFormat(eFormat) 
                << ", "<< translateDepth(eDepth) 
                << "," << iChannels << ")  this:" << this); 
   
   ICLASSERT_RETURN ( eFormat == formatMatrix ||
                      getChannelsOfFormat (eFormat) == m_iChannels );
}

ImgI::~ImgI()
{
  FUNCTION_LOG("");
}

// }}} 

// {{{ setter functions

void ImgI::setFormat(format eFormat)
{
  FUNCTION_LOG("setFormat(" << translateFormat(eFormat) << ")");
  if (eFormat != formatMatrix) {
     int nChannels = getChannelsOfFormat(eFormat);
     if(nChannels != m_iChannels){
        setChannels(nChannels);
     }
  }
  m_eFormat=eFormat;
}

// }}}

// {{{ utillity functions

ImgI* ImgI::shallowCopy(ImgI* poDst) const {
  FUNCTION_LOG("");
  // create image with zero channels
  if (!poDst) poDst = imgNew(getDepth(),getSize(),formatMatrix,0);
  else ensureDepth (&poDst, getDepth ());
  
  if (getDepth() == depth8u) {
     *poDst->asImg<icl8u>() = *this->asImg<icl8u>();
  } else {
     *poDst->asImg<icl32f>() = *this->asImg<icl32f>();
  }
  poDst->setROI (getROIOffset(), getROISize());
  return poDst;
}

ImgI* ImgI::shallowCopy(const int* const piStart, const int* const piEnd,
                        ImgI* poDst) const {
  FUNCTION_LOG("");
  // create image with zero channels
  if (!poDst) poDst = imgNew(getDepth(),getSize(),formatMatrix,0);
  else {
     poDst->setChannels (0);
     ensureDepth (&poDst, getDepth ());
     poDst->m_oSize = this->getSize();
     poDst->m_eFormat = formatMatrix;
  }

  if (getDepth() == depth8u) {
     poDst->asImg<icl8u>()->append (this->asImg<icl8u>(), piStart, piEnd);
  } else {
     poDst->asImg<icl32f>()->append (this->asImg<icl32f>(), piStart, piEnd);
  }
  poDst->setROI (getROIOffset(), getROISize());
  return poDst;
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
      printf("| channel: %d, min: %d, max:%d \n",i,asImg<icl8u>()->getMin(i),asImg<icl8u>()->getMax(i));
    }
  }else{
    for(int i=0;i<m_iChannels;i++){
      printf("| channel: %d, min: %f, max:%f \n",i,asImg<icl32f>()->getMin(i),asImg<icl32f>()->getMax(i));
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

// {{{ convertTo - template

template <class T>
Img<T> *ImgI::convertTo( Img<T>* poDst) const {
  FUNCTION_LOG("");
 
  if(!poDst) poDst = new Img<T>(Size(1,1),1);
  poDst->resize(getSize());
  poDst->setFormat(getFormat());
  poDst->setChannels(getChannels());
  poDst->setROI(getROI());
  
  if(getDepth() == depth8u){
    for(int c=0;c<getChannels();c++) deepCopyChannel<icl8u,T>(asImg<icl8u>(),c,poDst,c);
  }else{
    for(int c=0;c<getChannels();c++) deepCopyChannel<icl32f,T>(asImg<icl32f>(),c,poDst,c);
  }
  return poDst;
}
  
  template Img<icl8u>* ImgI::convertTo<icl8u>(Img<icl8u>*) const;
  template Img<icl32f>* ImgI::convertTo<icl32f>(Img<icl32f>*) const;

  // }}}

} //namespace icl
