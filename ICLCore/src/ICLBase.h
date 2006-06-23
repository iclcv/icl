/*
  ICLBase.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLBASE_H
#define ICLBASE_H
    
//-------- includes --------
#include "ICLMacros.h"

//---- Use the following namespaces as default ----
using namespace std;

//---- ICL in its own namespace ----
namespace ICL {

/* {{{ class ICLBase */

/**
   @short ICLBase 
**/

//---- Foreward deklaration ----
class ICL<iclbyte>;
class ICL<iclfloat>;

//---- ICL Base ----
class ICLBase
{   
  //------------------------------------------------------------------------
 private:
  int m_iWidth, m_iHeight, m_iChannels;
  iclformat m_eFormat;
  icldepth m_eDepth;
  
 protected:
  //------------------------------------------------------------------------
  
 public:
  /* {{{ Konstruktor/ Destruktor: */
  //@{
  //-------------------------------------------------------------------------- 
  /** ICLBase 1    
  **/
  ICLBase(int iWidth=1, 
          int iHeight=1, 
          int iChannel=1, 
          icldepth eDepth=depth8u);

  //-------------------------------------------------------------------------- 
  /** ICLBase 2
   **/
  ICLBase(int iWidth, 
          int iHeight, 
          iclformat eFormat, 
          icldepth eDepth=depth8u,
          int iChannel=-1);
  
  //-------------------------------------------------------------------------- 
  /** Destructor **/
  virtual ~ICLBase();

  //@}
  /* }}} */

/* {{{ Get Functions */

  //-------------------------------------------------------------------------- 
  /** getHeight **/
  int getHeight(int iChannel = 0)
    {
      return m_iHeight;
    }

  //-------------------------------------------------------------------------- 
  /** getWidth **/
  int getWidth(int iChannel = 0)
    {
      return m_iWidth;
    }

  //-------------------------------------------------------------------------- 
  /** getChannels **/
  int getChannels()
    {
      return m_iChannels;
    }

  //-------------------------------------------------------------------------- 
  /** getDepth **/
  icldepth getDepth()
    {
      return m_eDepth;
    }

  //-------------------------------------------------------------------------- 
  /** getFormat **/
  iclformat getFormat()
    {
      return m_eFormat;
    }
  
  //-------------------------------------------------------------------------- 
  /** getData **/
  virtual void* getDataPtr(int iChannel) = 0;
  
  //-------------------------------------------------------------------------- 
  /** asIcl8u **/
  ICL<iclbyte>* asIcl8u()
    {
      return reinterpret_cast<ICL<iclbyte> >(this);
    }
  
  //-------------------------------------------------------------------------- 
  /** asIcl32f **/
  ICL<iclfloat>* asIcl32f()
    {
      return reinterpret_cast<ICL<iclfloat> >(this);
    }

/* }}} */

/* {{{ Set Functions */

  //-------------------------------------------------------------------------- 
  /** setFormat **/
  void setFormat(iclformat eFormat);

/* }}} */
 
};

/* }}} */

} //namespace ICL

#endif //ICLCHANNEL_H
