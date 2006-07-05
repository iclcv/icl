/*
  ICLBase.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "ICLBase.h"

namespace icl {

// {{{  Konstruktor/ Destruktor: 

ICLBase::ICLBase(int iWidth, 
                 int iHeight, 
                 int iChannels, 
                 icldepth eDepth):
  m_iWidth(iWidth),m_iHeight(iHeight),m_iChannels(iChannels),m_eDepth(eDepth)
{
  DEBUG_LOG4("Konstruktor: ICLBase() -> " << this);
  
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
  
}


ICLBase::~ICLBase()
{
  DEBUG_LOG4("Destruktor: ICLBase() -> " << this);
  
}

// }}} 

void ICLBase::setFormat(iclformat eFormat)
{
  DEBUG_LOG4("setFormat(iclformat)" << this);
  int nChannels = iclGetChannelsOfFormat(eFormat);
  if(nChannels != m_iChannels){
    setNumChannels(nChannels);
  }
  m_eFormat=eFormat;
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

} //namespace icl


