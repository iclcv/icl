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
    ERROR_LOG("incompatible channel count");
  }
  m_eFormat=eFormat;
}



} //namespace icl


