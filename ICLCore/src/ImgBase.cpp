/*
  ImgBase.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "ImgBase.h"
#include "Img.h"

using namespace std;

namespace icl {

// {{{ constructor / destructor 

  ImgBase::ImgBase(depth d, const ImgParams &params):
    m_oParams(params),m_eDepth(d)
  {
    FUNCTION_LOG("ImgBase(" << getWidth()
                 << "," << getHeight()
                 << "," << translateFormat(getFormat()) 
                 << ", "<< translateDepth(getDepth()) 
                 << "," << getChannels() << ")  this:" << this); 
  }

ImgBase::~ImgBase()
{
  FUNCTION_LOG("");
}

// }}} 

// {{{ utillity functions

ImgBase* ImgBase::shallowCopy(ImgBase** ppoDst) const {
  FUNCTION_LOG("");

  ImgBase* poDst = ppoDst ? *ppoDst : 0;

  // create image with zero channels
  if (!poDst) poDst = imgNew(getDepth(),getSize(),0,getROI());
  else ensureDepth (&poDst, getDepth ());
  
  if (getDepth() == depth8u) {
     *poDst->asImg<icl8u>() = *this->asImg<icl8u>();
  } else {
     *poDst->asImg<icl32f>() = *this->asImg<icl32f>();
  }

  if (ppoDst) *ppoDst = poDst;
  return poDst;
}

ImgBase* ImgBase::shallowCopy(const std::vector<int>& vChannels, ImgBase** ppoDst) const {
  FUNCTION_LOG("");

  ImgBase* poDst = ppoDst ? *ppoDst : 0;

  // create image with zero channels
  if (!poDst) poDst = imgNew(getDepth(),getSize(),0,getROI());
  else {
     poDst->setChannels (0);
     ensureDepth (&poDst, getDepth ());
     poDst->setSize(getSize());
     poDst->setROI (getROI());
  }

  if (getDepth() == depth8u) {
     poDst->asImg<icl8u>()->append (this->asImg<icl8u>(), vChannels);
  } else {
     poDst->asImg<icl32f>()->append (this->asImg<icl32f>(), vChannels);
  }

  if (ppoDst) *ppoDst = poDst;
  return poDst;
}


void ImgBase::print(const string sTitle) const
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
            translateFormat(getFormat()).c_str(),
            getROI().x, getROI().y,getROI().width, getROI().height);
  if(m_eDepth == depth8u){
    for(int i=0;i<getChannels();i++){
      printf("| channel: %d, min: %d, max:%d \n",i,asImg<icl8u>()->getMin(i),asImg<icl8u>()->getMax(i));
    }
  }else{
    for(int i=0;i<getChannels();i++){
      printf("| channel: %d, min: %f, max:%f \n",i,asImg<icl32f>()->getMin(i),asImg<icl32f>()->getMax(i));
    }
  }
  printf(" -----------------------------------------\n");
 

}

// }}}

// {{{ convertTo - template

template <class T>
Img<T> *ImgBase::convertTo( Img<T>* poDst) const {
  FUNCTION_LOG("");
 
  if(!poDst) poDst = new Img<T>(getParams());
  else poDst->setParams(getParams());
  
  if(getDepth() == depth8u){
    for(int c=0;c<getChannels();c++) deepCopyChannel<icl8u,T>(asImg<icl8u>(),c,poDst,c);
  }else{
    for(int c=0;c<getChannels();c++) deepCopyChannel<icl32f,T>(asImg<icl32f>(),c,poDst,c);
  }
  return poDst;
}
  
template Img<icl8u>* ImgBase::convertTo<icl8u>(Img<icl8u>*) const;
template Img<icl32f>* ImgBase::convertTo<icl32f>(Img<icl32f>*) const;

// }}}

// {{{ setFormat
void ImgBase::setFormat(format fmt){
  FUNCTION_LOG("");
  int newcc = getChannelsOfFormat(fmt);
  if(fmt != formatMatrix && newcc != getChannels()){
    setChannels(newcc);
  }
  m_oParams.setFormat(fmt);
}

// }}}

// {{{ setParams

void ImgBase::setParams(const ImgParams &params){
  FUNCTION_LOG("");
  setChannels(params.getChannels());
  setSize(params.getSize());
  setFormat(params.getFormat());
  setROI(params.getROI());
}

// }}}


} //namespace icl
