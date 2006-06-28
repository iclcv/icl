/*
  ICLChannel.cpp

  Written by: Michael Götting (2004)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "ICLChannel.h"

namespace ICL {

// {{{  Konstruktor/ Destruktor: 
//----------------------------------------------------------------------------
template <class Type> 
ICLChannel<Type>::ICLChannel(int iWidth, int iHeight, Type *ptData)
{
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICLChannel(int,int) -> " << this);

  //---- Variable initialisation ----
  m_oInfo.setSize(iWidth, iHeight);
  m_oInfo.setRoi(iWidth, iHeight);
  
  //---- Allocate memory ----
  m_ptData = ptData ? ptData : new Type[m_oInfo.getDim()];

  //---- Enshure data will be deleted ----
  m_bDeleteData = ptData ? 1 : 0;

  //---- Set all pixel to zero ----
  clear();
}


//--------------------------------------------------------------------------
template <class Type> 
ICLChannel<Type>::ICLChannel(const ICLChannel<Type>& tSrc)
{
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICLChannel(const ICLChannel<Type>&) -> "
             << this);
  
  //---- Variable initialisation ----
  m_oInfo.setSize(tSrc.m_oInfo.getWidth(), 
                  tSrc.m_oInfo.getHeight());
  m_oInfo.setRoi(tSrc.m_oInfo.getWidth(), 
                 tSrc.m_oInfo.getHeight());
  
  //---- Alloccate memory (if needed) ----
  m_ptData = new Type[m_oInfo.getDim()];

  //---- Enshure data will be deleted ----
  m_bDeleteData = 1;
   
  //---- Copy data from source channel----
  std::copy(tSrc.m_ptData,
            (tSrc.m_ptData)+m_oInfo.getDim(), 
            m_ptData);
}

//---------------------------------------------------------------------------
template <class Type> 
ICLChannel<Type>::~ICLChannel()
{
  //---- Log Message ----
  DEBUG_LOG4("Destruktor: ICLChannel() -> " << this);

  //---- free memory ----
  if(m_bDeleteData = 1)
    {
      delete [] m_ptData;
    }
  
}

// }}} 

// {{{  Get functions: 
//--------------------------------------------------------------------------
template <class Type>
Type
ICLChannel<Type>::getMin() const
{
  //---- Log Message ----
  DEBUG_LOG4("getMin()"); 

  //---- Varaibale definition ----
  Type *iter;
  
  //---- Compute min element  ----
  iter = min_element(m_ptData, m_ptData+m_oInfo.getDim());
  
  //---- Log Message ----
  DEBUG_LOG3("min value " << *iter); 
  
  //---- return ----
  return *iter;
}


//----------------------------------------------------------------------------
template <class Type>
Type
ICLChannel<Type>::getMax() const
{
  //---- Log Message ----
  DEBUG_LOG4("getMax()"); 
  
  //---- Variable definition ----
  Type *iter;
  
  //---- Compute max value ----
  iter = max_element(m_ptData, m_ptData+m_oInfo.getDim());
  
  //---- return ----
  return *iter;
  
} 

// }}} 

// {{{  basic channel functions: 

//----------------------------------------------------------------------------
template <class Type>
void ICLChannel<Type>::resize(int iNewWidth, int iNewHeight)
{
  //---- Log Message ----
  DEBUG_LOG4("resize(int, int)"); 
  
  //---- Check if resize is meaningful ----
  if (((m_oInfo.getWidth() != iNewWidth) || 
       (m_oInfo.getHeight() != iNewHeight)) &&
      (iNewWidth*iNewHeight > 0 ) )
  {
  
    //---- set new size ----
    m_oInfo.setSize(iNewWidth, iNewHeight);
    m_oInfo.setRoi(iNewWidth, iNewHeight);
    
    //---- Allocate memory -----
    delete [] m_ptData;
     
    m_ptData = new Type[m_oInfo.getDim()];
    
    //---- Set all pixel to zero ----
    clear();
  }
}

//--------------------------------------------------------------------------
template <class Type>
void ICLChannel<Type>::scaleRange(Type scaleFactor)
{
  //---- Log Message ----
  DEBUG_LOG3("scaleRange(Type)");
  
  //---- scale ----
  for(int y=0;y<m_oInfo.getHeight();y++)
    {
      for(int x=0;x<m_oInfo.getWidth();x++)
        {
          (*this)(x,y)*=scaleFactor;
        }
    }

}

//--------------------------------------------------------------------------
template <class Type>
void ICLChannel<Type>::scaleRange(float fNewMin, 
                                  float fNewMax, 
                                  float fMin, 
                                  float fMax)
{
  //---- Log Message ----
  DEBUG_LOG3("scaleRange(Type, Type, Type, Type");
  
  float fScale  = (fNewMax - fNewMin) / (fMax - fMin);
  float fShift  = (fMax * fNewMin - fMin * fNewMax) / (fMax - fMin);
  
  //---- Compute new value ----
  for(int x=0;x<m_oInfo.getWidth();x++)
    {
      for(int y=0;y<m_oInfo.getHeight();y++)
        {
          iclfloat fPixel = static_cast<iclfloat>((*this)(x,y));
          fPixel=fPixel*fScale+fShift;
          if(fPixel<=fNewMin)
            fPixel=fNewMin;
          else if(fPixel>=fNewMax)
            fPixel=fNewMax;
          
          (*this)(x,y) = static_cast<Type>(fPixel);
        }
    }  
}

// }}} 

template class ICLChannel<iclbyte>;
template class ICLChannel<iclfloat>;

} //namespace ICL


