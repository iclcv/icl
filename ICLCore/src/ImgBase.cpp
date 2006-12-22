/*
  ImgBase.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include <ImgBase.h>
#include <Img.h>

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
  
  switch (getDepth()){
    case depth8u: *poDst->asImg<icl8u>() = *this->asImg<icl8u>(); break;
    case depth16s: *poDst->asImg<icl16s>() = *this->asImg<icl16s>(); break;
    case depth32s: *poDst->asImg<icl32s>() = *this->asImg<icl32s>(); break;
    case depth32f: *poDst->asImg<icl32f>() = *this->asImg<icl32f>(); break;
    case depth64f: *poDst->asImg<icl64f>() = *this->asImg<icl64f>(); break;
    default: ICL_INVALID_DEPTH; break;
  }

  if (ppoDst) *ppoDst = poDst;
  poDst->setTime(this->getTime());
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
  switch (getDepth()){
    case depth8u: poDst->asImg<icl8u>()->append (this->asImg<icl8u>(), vChannels); break;
    case depth16s: poDst->asImg<icl16s>()->append (this->asImg<icl16s>(), vChannels); break;
    case depth32s: poDst->asImg<icl32s>()->append (this->asImg<icl32s>(), vChannels); break;
    case depth32f: poDst->asImg<icl32f>()->append (this->asImg<icl32f>(), vChannels); break;
    case depth64f: poDst->asImg<icl64f>()->append (this->asImg<icl64f>(), vChannels); break;
    default: ICL_INVALID_DEPTH; break;
  }

  if (ppoDst) *ppoDst = poDst;
  poDst->setTime(this->getTime());
  return poDst;
}


void ImgBase::print(const string sTitle) const
{
  FUNCTION_LOG(sTitle);
  printf(   " -----------------------------------------\n"
            "| image: %s\n"
            "| timestamp: %s\n"
            "| width: %d, height: %d, channels: %d\n",sTitle.c_str(), this->getTime().toString().c_str(),
            getSize().width,getSize().height,getChannels());
  printf(   "| depth: %s  format: %s\n",translateDepth(getDepth()).c_str(), translateFormat(getFormat()).c_str());
  printf(   "| ROI: x: %d, y: %d, w: %d, h: %d \n", getROI().x, getROI().y,getROI().width, getROI().height);

  switch (m_eDepth){
    case depth8u:
      for(int i=0;i<getChannels();i++){
        printf("| channel: %d, min: %d, max:%d \n",i,asImg<icl8u>()->getMin(i),asImg<icl8u>()->getMax(i));
      }
      break;
    case depth16s:
      for(int i=0;i<getChannels();i++){
        printf("| channel: %d, min: %d, max:%d \n",i,asImg<icl16s>()->getMin(i),asImg<icl16s>()->getMax(i));
      }
      break;
    case depth32s:
      for(int i=0;i<getChannels();i++){
        printf("| channel: %d, min: %d, max:%d \n",i,asImg<icl32s>()->getMin(i),asImg<icl32s>()->getMax(i));
      }
      break;
    case depth32f:
      for(int i=0;i<getChannels();i++){
        printf("| channel: %d, min: %f, max:%f \n",i,asImg<icl32f>()->getMin(i),asImg<icl32f>()->getMax(i));
      }
      break;
    case depth64f:
      for(int i=0;i<getChannels();i++){
        printf("| channel: %d, min: %f, max:%f \n",i,asImg<icl64f>()->getMin(i),asImg<icl64f>()->getMax(i));
      }
      break;
    default: ICL_INVALID_DEPTH; break;
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

  switch (getDepth()){
    case depth8u: for(int c=0;c<getChannels();c++) deepCopyChannel<icl8u,T>(asImg<icl8u>(),c,poDst,c); break;
    case depth16s: for(int c=0;c<getChannels();c++) deepCopyChannel<icl16s,T>(asImg<icl16s>(),c,poDst,c); break;
    case depth32s: for(int c=0;c<getChannels();c++) deepCopyChannel<icl32s,T>(asImg<icl32s>(),c,poDst,c); break;
    case depth32f: for(int c=0;c<getChannels();c++) deepCopyChannel<icl32f,T>(asImg<icl32f>(),c,poDst,c); break;
    case depth64f: for(int c=0;c<getChannels();c++) deepCopyChannel<icl64f,T>(asImg<icl64f>(),c,poDst,c); break;
    default: ICL_INVALID_FORMAT; break;
  }
  return poDst;
}
  
template Img<icl8u>* ImgBase::convertTo<icl8u>(Img<icl8u>*) const;
template Img<icl16s>* ImgBase::convertTo<icl16s>(Img<icl16s>*) const;
template Img<icl32s>* ImgBase::convertTo<icl32s>(Img<icl32s>*) const;
template Img<icl32f>* ImgBase::convertTo<icl32f>(Img<icl32f>*) const;
template Img<icl64f>* ImgBase::convertTo<icl64f>(Img<icl64f>*) const;

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
