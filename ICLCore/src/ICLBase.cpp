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

// {{{  Konstruktor/ Destruktor: 

ICLBase::ICLBase(int iWidth, 
                 int iHeight, 
                 int iChannels, 
                 icldepth eDepth):
  m_iWidth(iWidth),m_iHeight(iHeight),m_iChannels(iChannels),m_eFormat(formatMatrix),m_eDepth(eDepth)
{
  DEBUG_LOG4("Konstruktor: ICLBase() -> " << this); 
  if(m_iChannels <= 0)
    {
      ERROR_LOG("channel count must be > 0");
    }
}

ICLBase::ICLBase(int iWidth, 
                 int iHeight, 
                 iclformat eFormat,
                 icldepth eDepth,
                 int iChannels):
  m_iWidth(iWidth),m_iHeight(iHeight),
  m_iChannels((iChannels < 0) ? iclGetChannelsOfFormat(eFormat) : iChannels),
  m_eFormat(eFormat),m_eDepth(eDepth)
{
  DEBUG_LOG4("Konstruktor: ICLBase() -> " << this); 
  if(m_iChannels <= 0)
    {
      ERROR_LOG("channel count must be > 0");
    }
}


ICLBase::~ICLBase()
{
  DEBUG_LOG4("Destruktor: ICLBase() -> " << this);
  
}

// }}} 

// {{{ setter functions

void ICLBase::setFormat(iclformat eFormat)
{
  DEBUG_LOG4("setFormat(iclformat)" << this);
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

void ICLBase::print(string sTitle)
{
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

} //namespace icl


