/*
  ICLBase.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "ICLBase.h"

namespace ICL {

// {{{  Konstruktor/ Destruktor: 
//----------------------------------------------------------------------------
ICLBase::ICLBase(int iWidth, 
                 int iHeight, 
                 int iChannels, 
                 icldepth eDepth):
  m_iWidth(iWidth),m_iHeight(iHeight),m_iChannels(iChannels),m_eDepth(eDepth)
{
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICLBase() -> " << this);
  
}
//----------------------------------------------------------------------------
ICLBase::ICLBase(int iWidth, 
                 int iHeight, 
                 iclformat eFormat,
                 icldepth eDepth,
                 int iChannels):
  m_iWidth(iWidth),m_iHeight(iHeight),m_iChannels(iChannels),
  m_eFormat(eFormat),m_eDepth(eDepth)
{
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICLBase() -> " << this);
  
}

//---------------------------------------------------------------------------
ICLBase::~ICLBase()
{
  //---- Log Message ----
  DEBUG_LOG4("Destruktor: ICLBase() -> " << this);
  
}

// }}} 

/* {{{ Set Functions */
//----------------------------------------------------------------------------
void setFormat(iclformat eFormat)
{
  //---- Log Message ----
  DEBUG_LOG4("setFormat(iclformat)" << this);
  
  //ToDo: Ueberpruefung einguegen
}

/* }}} */

} //namespace ICL


