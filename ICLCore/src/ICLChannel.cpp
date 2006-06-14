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
ICLChannel<Type>::ICLChannel(int iWidth, int iHeight)
{
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICLChannel(int,int) -> " << this);

  //---- Variable definition ----
  typename vector<Type>::iterator dataIter;

  //---- Variable initialization ----
  this->m_iDepth = 8 * sizeof(Type);
  
  m_oInfo.setImageSize(iWidth, iHeight);
  m_oInfo.setImageRoi(iWidth, iHeight);
  
  //---- Alloccate memory ----
  m_ptData.resize(m_oInfo.getImageDim());
  m_pptRow.resize(m_oInfo.getImageHeight());
  
  dataIter = m_ptData.begin();
  
  for(int i=0; i<m_oInfo.m_iImageHeight; i++)
  {
    m_pptRow[i] = dataIter;
    dataIter += m_oInfo.m_iImageWidth;
  }
  
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
  
  //---- Variable definition ----
  typename vector<Type>::iterator dataIter;

  //---- Variable initialisation ----
  m_oInfo.setImageSize(tSrc.m_oInfo.m_iImageWidth, 
                       tSrc.m_oInfo.m_iImageHeight);
  m_iDepth = tSrc.m_iDepth;
  m_oInfo.setImageRoi(tSrc.m_oInfo.m_iImageWidth, 
                      tSrc.m_oInfo.m_iImageHeight);
  
  //---- Alloccate memory ----
  m_ptData.resize(m_oInfo.m_iImageDim);
  m_pptRow.resize(m_oInfo.m_iImageHeight);
  
  dataIter = m_ptData.begin();
  
  for(int i=0; i<m_oInfo.m_iImageHeight; i++)
  {
    m_pptRow[i] = dataIter;
    dataIter += m_oInfo.m_iImageWidth;
  }
  
  //---- Copy data from source channel----
  std::copy(tSrc.m_ptData.begin(),
            tSrc.m_ptData.end(), 
            m_ptData.begin() );
}

//---------------------------------------------------------------------------
template <class Type> 
ICLChannel<Type>::~ICLChannel()
{
  //---- Log Message ----
  DEBUG_LOG4("Destruktor: ICLChannel() -> " << this);
  
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
  typename vector<Type>::const_iterator iter;

  //---- Compute min element  ----
  iter = min_element(m_ptData.begin(), m_ptData.end());
  
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
  typename vector<Type>::const_iterator iter;
  
  //---- Compute max value ----
  iter = max_element(m_ptData.begin(), m_ptData.end());
  
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
  if (((m_oInfo.m_iImageWidth != iNewWidth) || 
       (m_oInfo.m_iImageHeight != iNewHeight)) &&
      (iNewWidth*iNewHeight > 0 ) )
  {
    //---- Variable defifniton ----
    typename vector<Type>::iterator dataIter;
    
    //---- set new size ----
    m_oInfo.setImageSize(iNewWidth, iNewHeight);
    m_oInfo.setImageRoi(iNewWidth, iNewHeight);
    
    //---- Allocate memory -----
    m_ptData.resize(m_oInfo.m_iImageDim);
    m_pptRow.resize(m_oInfo.m_iImageHeight);
    
    dataIter = m_ptData.begin();
    
    for(int i=0; i<m_oInfo.m_iImageHeight; i++)
    {
      m_pptRow[i] = dataIter;
      dataIter += m_oInfo.m_iImageWidth;
    }
    
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
  for(int y=0;y<m_oInfo.m_iImageHeight;y++)
    for(int x=0;x<m_oInfo.m_iImageWidth;x++)
      (m_pptRow[y])[x] = (m_pptRow[y])[x] * scaleFactor;
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
  for(int x=0;x<m_oInfo.m_iImageWidth;x++)
    for(int y=0;y<m_oInfo.m_iImageHeight;y++)
    {
      float fPixel = getPixel(x,y);
      fPixel=fPixel*fScale+fShift;
      if(fPixel<=fNewMin)
        fPixel=fNewMin;
      else if(fPixel>=fNewMax)
        fPixel=fNewMax;
      
      setPixel(x,y,Type(fPixel));
    }  
}

// }}} 

template class ICLChannel<DEPTH8>;  /* 8bit channel */
template class ICLChannel<DEPTH32>; /* 32bit channel */

} //namespace ICL


