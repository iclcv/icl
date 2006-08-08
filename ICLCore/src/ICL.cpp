/*
  ICL.cpp

  Written by: Michael Götting (2004)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "ICL.h"

namespace icl {

// {{{  construtors and desctructor

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(int iWidth,int iHeight,int iChannels):
  // {{{ open

  ICLBase(iWidth,iHeight,formatMatrix,iclGetDepth<Type>(),iChannels){
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICL(int,int,int) -> " << this);
  
  //---- Variable definiton/ initialisation ----
  m_ppChannels.resize(m_iChannels);
  
  //---- ICL Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(iWidth,iHeight));
    }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(int iWidth, int iHeight, iclformat eFormat, int iChannels):
  // {{{ open

  ICLBase(iWidth,iHeight,eFormat,iclGetDepth<Type>(),iChannels){
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICL(int,int,int) -> " << this);
  
  //---- Variable definiton/ initialisation ----
  m_ppChannels.resize(m_iChannels);
  
  //---- ICL Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(iWidth,iHeight));
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(int iWidth, int iHeight, iclformat eFormat, int iChannels, Type** pptData):
  // {{{ open

  ICLBase(iWidth,iHeight,eFormat,iclGetDepth<Type>(),iChannels){
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICL(int,int,int) -> " << this);
  
  //---- Variable definiton/ initialisation ----
  m_ppChannels.resize(m_iChannels);
  
  //---- ICL Channel using shared memory ----
  for(int i=0;i<m_iChannels;i++)
    m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(iWidth,iHeight,pptData[i]));
} 

  // }}}
 
//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(const ICL<Type>& tSrc):
  // {{{ open
    ICLBase(tSrc.getWidth(),tSrc.getHeight(),
            tSrc.getFormat(),tSrc.getDepth(),tSrc.getChannels())
{
  
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICL(const ICL<Type>&) -> " << this);
  
  //---- Variable initialisation ----
  m_iChannels = tSrc.getChannels();
  m_ppChannels.resize(m_iChannels);
  
  //---- Shallow copy of source channels ----
  std::copy(tSrc.m_ppChannels.begin(), 
            tSrc.m_ppChannels.end(),
            m_ppChannels.begin());
}

  // }}}

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::~ICL()
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("Destruktor: ICL() -> " << this);
  
  //---- Delete channels ----
  deleteChannels ();
}

  // }}}

// }}} 

// {{{  assign operator: shallow copy

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>& ICL<Type>::operator=(const ICL<Type>& tSrc)
{
  //---- Log Message ----
  DEBUG_LOG4("operator=(const ICL<Type>&");
  
  //---- Free old data ----
  this->deleteChannels();
  
  //---- Assign new channels to ICL ----
  this->m_iWidth = tSrc.getWidth();
  this->m_iHeight = tSrc.getHeight();
  this->m_eFormat = tSrc.getFormat();
  this->m_eDepth = tSrc.getDepth();
  this->m_iChannels = tSrc.getChannels();  
  this->m_ppChannels.resize(this->m_iChannels);
  
  //---- Shallow copy of source channels ----
  std::copy(tSrc.m_ppChannels.begin(), 
            tSrc.m_ppChannels.end(),
            this->m_ppChannels.begin());

  //---- return ----
  return *this;
}

// }}} 

// {{{  class organisation : 

//----------------------------------------------------------------------------
template<class Type> ICLBase*
ICL<Type>::deepCopy(ICLBase* poDst) const
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("deepCopy(ICL<Type>*)"); 

  //---- Allocate memory ----
  if (poDst == NULL) 
    {
      poDst = new ICL<Type>(m_iWidth,m_iHeight,m_eFormat,m_iChannels);
    }
  else
    {
      poDst->renew(m_iWidth,m_iHeight,m_iChannels);
      poDst->setFormat(m_eFormat);
    }
  
  poDst->setROIRect(getROIRect());
  
  if(poDst->getDepth() == getDepth())
    {
      int nByteSize = m_iWidth * m_iHeight * sizeof(Type);
      for(int c=0;c<m_iChannels;c++)
        {
          memcpy(poDst->getDataPtr(c), getDataPtr(c), nByteSize);
        }
      return poDst;
    }
  else
    {
      if(poDst->getDepth() == depth8u)
        {
          return convertTo8Bit(poDst->asIcl8u());
        }
      else
        {
          return convertTo32Bit(poDst->asIcl32f());
        }
    }
}

  // }}}

//--------------------------------------------------------------------------
template<class Type> ICLBase*
ICL<Type>::scaledCopy(ICLBase *poDst,iclscalemode eScaleMode) const
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("smartCopy(ICL,iclscalemode)");
  
  //---- deep copy case -----
  if(!poDst || isEqual(poDst->getWidth(),poDst->getHeight(),poDst->getChannels())){
    return deepCopy(poDst); 
  }
  
  //---- type conversion case -----------
  if(getDepth() != poDst->getDepth())
    {
      ICLBase *poTmp = iclNew(getDepth(),poDst->getWidth(),poDst->getHeight(),formatMatrix,poDst->getChannels());
      scaledCopy(poTmp,eScaleMode);
      poTmp->deepCopy(poDst);
      delete poTmp;
      return poDst;
    }
  
  poDst->setNumChannels( getChannels() );
  poDst->setFormat( getFormat() );
  poDst->delROI();
    
#ifdef WITH_IPP_OPTIMIZATION
 
  IppiRect oHoleImageROI = {0,0,m_iWidth,m_iHeight};
 
  for(int c=0;c<m_iChannels;c++)
    {
      if(getDepth()==depth8u)
        {
          ippiResize_8u_C1R((Ipp8u*)getDataPtr(c),ippSize(),ippStep(),oHoleImageROI,
                            poDst->ippData8u(c),poDst->ippStep(),poDst->ippROISize(),
                            (double)poDst->getWidth()/(double)getWidth(),
                            (double)poDst->getHeight()/(double)getHeight(),
                            (int)eScaleMode);
        }else{
          ippiResize_32f_C1R((Ipp32f*)getDataPtr(c),ippSize(),ippStep(),oHoleImageROI,
                             poDst->ippData32f(c),poDst->ippStep(),poDst->ippROISize(),
                             (double)poDst->getWidth()/(double)getWidth(),
                             (double)poDst->getHeight()/(double)getHeight(),
                             (int)eScaleMode);
        }
    } 
  
#else

  //---- Variable initilazation ----
  float fXStep = ((float)getWidth()-1)/(float)(poDst->getWidth()); 
  float fYStep = ((float)getHeight()-1)/(float)(poDst->getHeight());

  //---- scale ICL ----
  for(int c=0;c<m_iChannels;c++)
    {
      //---- Take the correct scaling method ----
      switch(eScaleMode) 
        {
          case interpolateNN: 
            if(poDst->getDepth()==depth8u){
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  (*(poDst->asIcl8u()))(x,y,c)=static_cast<iclbyte>((*this)((int)rint(x*fXStep),(int)rint(y*fYStep),c));
                }
              }
            }else{
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  (*(poDst->asIcl32f()))(x,y,c)=static_cast<iclfloat>((*this)((int)rint(x*fXStep),(int)rint(y*fYStep),c));
                }
              }
            }
            break;
          
          case interpolateLIN: 
            if(poDst->getDepth()==depth8u){
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  (*(poDst->asIcl8u()))(x,y,c)=static_cast<iclbyte>(interpolate((fXStep/2)+ x*fXStep,(fYStep/2)+y*fYStep,c));
                }
              }
            }else{
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  (*(poDst->asIcl32f()))(x,y,c)=static_cast<iclfloat>(interpolate((fXStep/2)+ x*fXStep,(fYStep/2)+y*fYStep,c));
                }
              }
            }
            break;
          
          case interpolateRA: 
            ERROR_LOG("not yet implemented for Region Average");
            break;
          
          default:
            ERROR_LOG("Illegal operation selected!");
            break;
        }
    }
  
#endif
  
  return poDst;
}

// }}}

//--------------------------------------------------------------------------
template<class Type> ICLBase*
ICL<Type>::deepCopyROI(ICLBase *poDst) const
  // {{{ open

{
  int iW,iH;
  getROISize(iW,iH);
  if(!poDst){
    poDst = iclNew(getDepth(),iW,iH,getFormat(),getChannels());
  }else{
    poDst->setNumChannels(getChannels());
    poDst->setFormat(getFormat());
  }
  int iDstW,iDstH;
  poDst->getROISize(iDstW,iDstH);
  if(iW!=iDstW || iH != iDstH)
    {
      ERROR_LOG("roi size of source and destination must be equal");
    }
  for(int c=0;c<m_iChannels;c++)
    {
      if(m_eDepth == poDst->getDepth())
        {
          if(m_eDepth == depth8u){
#ifndef WITH_IPP_OPTIMIZATION
            for(ICL8u::iterator s=asIcl8u()->begin(c),d=poDst->asIcl8u()->begin(c); s.inRegion();s.incRow(),d.incRow()){
              memcpy(&*d,&*s,s.getROIWidth()*sizeof(iclbyte));
            }
#else
            ippiCopy_8u_C1R(ippData8u(c),ippStep(),poDst->ippData8u(c),poDst->ippStep(),ippROISize());
#endif
          }else{
#ifndef WITH_IPP_OPTIMIZATION
             for(ICL32f::iterator s=asIcl32f()->begin(c),d=poDst->asIcl32f()->begin(c); s.inRegion();s.incRow(),d.incRow()){
              memcpy(&*d,&*s,s.getROIWidth()*sizeof(iclfloat));
            }
#else
             ippiCopy_32f_C1R(ippData32f(c),ippStep(),poDst->ippData32f(c),poDst->ippStep(),ippROISize());
#endif
          }
        }
      else
        {
          if(m_eDepth == depth8u){
#ifndef WITH_IPP_OPTIMIZATION
            ICL8u::iterator s=asIcl8u()->begin(c);
            ICL32f::iterator d=poDst->asIcl32f()->begin(c);
            for(;s.inRegion();d++,s++){
              *d = static_cast<iclfloat>(*s);
            }
#else
            ippiConvert_8u32f_C1R(ippData8u(c),ippStep(),poDst->ippData32f(c),poDst->ippStep(),ippROISize());
#endif
          }else{
#ifndef WITH_IPP_OPTIMIZATION
            ICL32f::iterator s=asIcl32f()->begin(c);
            ICL8u::iterator d=poDst->asIcl8u()->begin(c);
            for(;s.inRegion();d++,s++){
              *d = static_cast<iclbyte>(*s);
            }
#else
            ippiConvert_32f8u_C1R(ippData32f(c),ippStep(),poDst->ippData8u(c),poDst->ippStep(),ippROISize(),ippRndNear);
#endif
          }
        }
    }
  return poDst;
}

// }}}
  
//----------------------------------------------------------------------------
template<class Type> ICLBase*
ICL<Type>::scaledCopyROI(ICLBase *poDst, iclscalemode eScaleMode) const
  // {{{ open
{
  //---- deep copy case -----
  if(!poDst){
    return deepCopyROI(poDst); 
  }

  //---- deep copy ROI case -----
  int iW,iH,iDstW,iDstH;
  getROISize(iW,iH);
  poDst->getROISize(iDstW,iDstH);
  if(iW==iDstW && iH == iDstH){
    return deepCopyROI(poDst);
  }
  
  //---- type conversion case -----------
  if(getDepth() != poDst->getDepth())
    {
      ICLBase *poTmp = iclNew(getDepth(),iW,iH,formatMatrix,poDst->getChannels());
      scaledCopyROI(poTmp,eScaleMode);
      poTmp->deepCopyROI(poDst);
      delete poTmp;
      return poDst;
    }    
  
  poDst->setNumChannels( getChannels() );
  
#ifdef WITH_IPP_OPTIMIZATION
  for(int c=0;c<m_iChannels;c++)
    {
      if(getDepth()==depth8u)
        {
          ippiResize_8u_C1R(ippData8u(c),ippSize(),ippStep(),ippROI(),
                            poDst->ippData8u(c),poDst->ippStep(),poDst->ippROISize(),
                            (double)iDstW/iW,
                            (double)iDstH/iH,
                            (int)eScaleMode);
        }else{
          ippiResize_32f_C1R(ippData32f(c),ippSize(),ippStep(),ippROI(),
                             poDst->ippData32f(c),poDst->ippStep(),poDst->ippROISize(),
                             (double)iDstW/(double)iW,
                             (double)iDstH/(double)iH,
                             (int)eScaleMode);
        }
    }   
#else
  /// _VERY_ slow fallback implementation
 
  ICLBase * poROITmp = iclNew(getDepth(),iW,iH,formatMatrix,m_iChannels);
  ICLBase * poDstTmp = iclNew(getDepth(),iDstW,iDstH,formatMatrix,m_iChannels);
  
  deepCopyROI(poROITmp);
  poROITmp->scaledCopy(poDstTmp,eScaleMode);
  poDstTmp->deepCopyROI(poDst);
  
  delete poROITmp;
  delete poDstTmp; 
#endif
  return poDst;
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::detach(int iIndex)
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("detach()");
  
  //---- Make the whole ICL independent ----
  if(iIndex == -1)
    {
      for(int i=0;i<m_iChannels;i++) 
        {
          m_ppChannels[i] = 
            ICLChannelPtr(new ICLChannel<Type>(*m_ppChannels[i]));
        }
    }
  //---- Make a specific channel independent ----
  else
    {
      m_ppChannels[iIndex] 
        = ICLChannelPtr (new ICLChannel<Type>(*m_ppChannels[iIndex]));
    } 
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::removeChannel(int iChannel)
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("removeChannel(int)"); 
  
  if(m_iChannels < 2)
    {
      ERROR_LOG("removing the last remaining channel is not allowed!");
    }
  
  m_ppChannels.erase(m_ppChannels.begin()+iChannel);

  m_iChannels--;
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::append(ICL<Type> *poSrc)
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("appendImage(const ICL<Type>&)"); 

  
  //---- ensure identical image size
  if(poSrc->getWidth() != getWidth() || poSrc->getHeight() != getHeight())
    {
      ERROR_LOG("error in ICL::append: image sizes are different!");
      return;
    }
  
  for(int i=0;i<poSrc->m_iChannels;i++){
    m_ppChannels.push_back(poSrc->m_ppChannels[i]); 
    m_ppChannels[m_iChannels]->setROIRect(getROIRect());
    m_iChannels++;
  }      
} 

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::appendChannel(int iChannel, ICL<Type> *poSrc) 
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("appendChannel(int,const ICL<Type>&)");
  
  
  //---- ensure identical image size
  if(poSrc->getWidth() != getWidth() || poSrc->getHeight() != getHeight())
    {
      ERROR_LOG("error in ICL::append: image sizes are different!");
      return;
    }
  
  m_ppChannels.push_back(poSrc->m_ppChannels[iChannel]); 
  m_ppChannels[m_iChannels]->setROIRect(getROIRect());
  m_iChannels++;
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void 
ICL<Type>::swapChannels(int iIndexA, int iIndexB)
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("swapChannels(int,int)"); 
  std::swap(m_ppChannels[iIndexA], m_ppChannels[iIndexB]);
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::scale(int iNewWidth,int iNewHeight,iclscalemode eScaleMode)
  // {{{ open

{  
  //---- Log Message ----
  DEBUG_LOG4("scale(int,int,iclscalemode)"); 
  
  //---- estimate destination values in respect to defaults ----
  if(iNewWidth < 0) iNewWidth = m_iWidth;
  if(iNewHeight < 0) iNewHeight = m_iHeight;
  
  if(isEqual(iNewWidth,iNewHeight,m_iChannels))
    {
      return;    
    }  
  else
    {
      ICL<Type> oTmp(iNewWidth,iNewHeight,m_eFormat,m_iChannels);
      scaledCopy(&oTmp,eScaleMode);
      (*this)=oTmp;
    }
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::resize(int iWidth,int iHeight)
  // {{{ open

{
  
  //---- Log Message ----
  DEBUG_LOG4("resize(int,int)"); 
  
  //---- estimate destination values in respect to defaults ----
  if(iWidth < 0) iWidth = m_iWidth;
  if(iHeight < 0) iHeight = m_iHeight;
  
  if(isEqual(iWidth,iHeight,m_iChannels))
    {
      return;
    }
  
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(iWidth,iHeight));
    }
  
  m_iWidth = iWidth;
  m_iHeight = iHeight;
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::setNumChannels(int iNumNewChannels)
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("setNumChannels(int)"); 

  if(iNumNewChannels <= 0)
    {
      ERROR_LOG("channel count must be > 0");
    }
   
  //---- reduce number of channels ----
  if(iNumNewChannels < m_iChannels)
  {
    for (int i=m_iChannels-1;i>=iNumNewChannels;i--)
    {
      removeChannel(i);
    }
  }
  //---- Extend number of channels ----
  else if (iNumNewChannels > m_iChannels)
  {

    int iNew = iNumNewChannels - m_iChannels;
    for(int i=0;i<iNew;i++)
      {
        m_ppChannels.push_back(ICLChannelPtr (new ICLChannel<Type>(getWidth(),getHeight())));
        m_ppChannels[m_iChannels]->setROIRect(getROIRect());
        m_iChannels++;
      }
  }
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::renew(int iNewWidth, int iNewHeight, int iNewNumChannels)
  // {{{ open

{
  DEBUG_LOG4("renewICL(int,int.int)"); 

  if(iNewWidth < 0)iNewWidth = m_iWidth;
  if(iNewHeight < 0)iNewHeight = m_iHeight;
  if(iNewNumChannels < 0)iNewNumChannels = m_iChannels;

  if(m_iChannels == 0)
    {
      ERROR_LOG("channel count must be > 0");
    }
  if(iNewWidth != getWidth() || iNewHeight != getHeight())
    {
      resize(iNewWidth,iNewHeight);
    }
  if(getChannels() != iNewNumChannels)
    {
      setNumChannels(iNewNumChannels);
    }
}

// }}}

//----------------------------------------------------------------------------
template<class Type> inline void 
ICL<Type>::replaceChannel(int iIndexA,int iIndexB,ICL<Type>  *poSrc) 
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("replaceChannel(int,int,const ICL<Type>&)");
  
  //---- replace channel ----
  m_ppChannels[iIndexA] = poSrc->m_ppChannels[iIndexB];
}

// }}}


// }}} 

// {{{  Type converter: 

//--------------------------------------------------------------------------
template<class Type> ICL32f*
ICL<Type>::convertTo32Bit(ICL32f *poDst) const
// {{{ open

{
  DEBUG_LOG4("convertTo32Bit()");
  
  if(!poDst)
  {
    poDst = new ICL32f(getWidth(),getHeight(),getFormat(),getChannels());
  }
  else
  {
    poDst->renew(m_iWidth,m_iHeight,m_iChannels);
    poDst->setFormat(m_eFormat);
  }
  
  int iX,iY,iW,iH;
  getROI(iX,iY,iW,iH);
  poDst->setROI(iX,iY,iW,iH);
  
  if(m_eDepth == depth8u)
  {
    for(int c=0;c<m_iChannels;c++)
    {
      iclbyte *pucSrc = reinterpret_cast<iclbyte*>(getDataPtr(c));
      iclfloat *pfDst = reinterpret_cast<iclfloat*>(poDst->getDataPtr(c));
#ifdef WITH_IPP_OPTIMIZATION
      IppiSize oWholeImageROI = {m_iWidth,m_iHeight};
      ippiConvert_8u32f_C1R(pucSrc,ippStep(),pfDst,poDst->ippStep(),oWholeImageROI);
#else
      int iDim = m_iWidth * m_iHeight;
      std::copy (pucSrc, pucSrc + iDim, pfDst);
#endif
    }
    return poDst;
  }
  else
  {
    return asIcl32f()->deepCopy(poDst)->asIcl32f();
  }  
}

  // }}}

//--------------------------------------------------------------------------
template<class Type> ICL8u*
ICL<Type>::convertTo8Bit(ICL8u *poDst) const
  // {{{ open

{
  DEBUG_LOG4("convertTo8Bit()");
  
  if(!poDst)
    {
      poDst = new ICL8u(getWidth(),getHeight(),getFormat(),getChannels());
    }
  else
    {
      poDst->renew(m_iWidth,m_iHeight,m_iChannels);
      poDst->setFormat(m_eFormat);
    }
  
  int iX,iY,iW,iH;
  getROI(iX,iY,iW,iH);
  poDst->setROI(iX,iY,iW,iH);
 
  if(m_eDepth == depth32f)
    {
      for(int c=0;c<m_iChannels;c++)
        {
          iclfloat *pfSrc = reinterpret_cast<iclfloat*>(getDataPtr(c));
          iclbyte *pucDst =  reinterpret_cast<iclbyte*>(poDst->getDataPtr(c));
#ifdef WITH_IPP_OPTIMIZATION
          IppiSize oHoleImageROI = {m_iWidth,m_iHeight};
          ippiConvert_32f8u_C1R(pfSrc,ippStep(),pucDst,poDst->ippStep(),oHoleImageROI,ippRndNear);
#else
          int iDim = m_iWidth* m_iHeight;
          std::copy (pfSrc, pfSrc + iDim, pucDst);
#endif
        }
      return poDst;
    }
  else
    {
      return asIcl8u()->deepCopy(poDst)->asIcl8u();
    }  
}

  // }}}

// }}} 

// {{{  Getter Functions: 

// ---------------------------------------------------------------------
template<class Type> Type 
ICL<Type>::getMax(int iChannel) const
  // {{{ open
{
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      iclbyte ucMax;
      ippiMax_8u_C1R(ippData8u(iChannel),ippStep(),ippROISize(),&ucMax);
      return static_cast<Type>(ucMax);
    }
  else
    {
      iclfloat fMax;
      ippiMax_32f_C1R(ippData32f(iChannel),ippStep(),ippROISize(),&fMax);
      return static_cast<Type>(fMax);
    }
                     
#else
  return m_ppChannels[iChannel]->getMax();
#endif
}

  // }}}

// ---------------------------------------------------------------------  
template<class Type> Type 
ICL<Type>::getMin(int iChannel) const
  // {{{ open
{
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      iclbyte ucMin;
      ippiMin_8u_C1R(ippData8u(iChannel),ippStep(),ippROISize(),&ucMin);
      return static_cast<Type>(ucMin);
    }
  else
    {
      iclfloat fMin;
      ippiMin_32f_C1R(ippData32f(iChannel),ippStep(),ippROISize(),&fMin);
      return static_cast<Type>(fMin);
    }
                     
#else
  return m_ppChannels[iChannel]->getMin();
#endif
}

  // }}}
  
// ---------------------------------------------------------------------
template<class Type> inline void 
ICL<Type>::getROI(int &riX, int &riY, int &riWidth, int &riHeight) const
  // {{{ open

{
  DEBUG_LOG4("getROI(int&,int&,int&,int&)");
  
  if(m_iChannels > 0)
    {
      m_ppChannels[0]->getROI(riX,riY,riWidth,riHeight);
    }
  else
    {
      ERROR_LOG("getROI channel count is 0 \n");
    }
}

  // }}}

// ---------------------------------------------------------------------
template<class Type> inline void 
ICL<Type>::getROIOffset(int &riX, int &riY) const
  // {{{ open
{
  DEBUG_LOG4("getROIOffset(int&,int&)");

  if(m_iChannels > 0)
    {
      m_ppChannels[0]->getROIOffset(riX,riY);
    }
  else
    {
      ERROR_LOG("getROIOffset channel count is 0 \n");
    }
}

  // }}}

// ---------------------------------------------------------------------
template<class Type> inline void 
ICL<Type>::getROISize(int &riWidth, int &riHeight) const
  // {{{ open

{
  DEBUG_LOG4("getROISize(int&,int&)");
  
  if(m_iChannels > 0)
    {
      m_ppChannels[0]->getROISize(riWidth,riHeight);
    }
  else
    {
      ERROR_LOG("getROISize channel count is 0 \n");
    }
}

  // }}}

// ---------------------------------------------------------------------
template<class Type> inline std::vector<int> 
ICL<Type>::getROIRect() const
  // {{{ open

{
  DEBUG_LOG4("getROIRect()");
   if(m_iChannels > 0)
    {
      return m_ppChannels[0]->getROIRect();
    }
  else
    {
      ERROR_LOG("getROISize channel count is 0 \n");
    }
}

  // }}}



// }}}
  
// {{{  Setter Function:
  
// ---------------------------------------------------------------------  
template<class Type> void 
ICL<Type>::setROI(int iX, int iY,int iWidth,int iHeight)
  // {{{ open

{
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i]->setROI(iX,iY,iWidth,iHeight);
    }
}

  // }}}
  
// ---------------------------------------------------------------------  
template<class Type> void 
ICL<Type>::setROIOffset(int iX, int iY)
  // {{{ open

{
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i]->setROIOffset(iX,iY);
    }
}

  // }}}
  
// ---------------------------------------------------------------------  
template<class Type> void 
ICL<Type>::setROISize(int iWidth, int iHeight)
  // {{{ open

{
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i]->setROISize(iWidth,iHeight);
    }
}

  // }}}

// ---------------------------------------------------------------------  
template<class Type> void 
ICL<Type>::setROIRect(std::vector<int> oRect)
  // {{{ open

{
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i]->setROIRect(oRect);
    }
  
}

  // }}}

// }}}

// {{{  Auxillary and basic image manipulation functions

template<class Type> Type
ICL<Type>::interpolate(float fX, float fY, int iChannel) const
{
  // {{{ open

  //---- Variable initialization ----
  float fY1,fY2,fY3,fY4,fT,fU;
  float fReturn;
  
  fY1 = fY2 = fY3 = fY4 = fT = fU = 0;
  
  //---- interpolate  ----
  if( (int) ceil(fX) >= this->getWidth(iChannel) ||  
      (int) ceil(fY) >= this->getHeight(iChannel) )
  {
    ERROR_LOG("Interpolation position out of range");
  }
  else
  {
    fY1=(float)(*this)((int)floor(fX), (int)floor(fY), iChannel);
    fY4=(float)(*this)((int)floor(fX), (int)ceil(fY), iChannel);
    fY3=(float)(*this)((int)ceil(fX), (int)ceil(fY), iChannel);
    fY2=(float)(*this)((int)ceil(fX), (int)floor(fY), iChannel);
    fT=fX-floor(fX);
    fU=fY-floor(fY);
  }
  
  fReturn=(1-fT)*(1-fU)*fY1+ fT*(1-fU)*fY2 + fT*fU*fY3 + (1-fT)*fU*fY4;
  
  //---- return ----
  if(m_eDepth == depth8u)
    return (Type) rint(fReturn);
  else
    return (Type) fReturn;
}

  // }}}

//--------------------------------------------------------------------------
template<class Type> void
ICL<Type>::scaleRange(float tMin, float tMax, int iChannel)
  // {{{ open

{
  DEBUG_LOG4("scaleRange(float,float,iChannel");
  
  if (iChannel == -1)
  {
    for(int i=0;i<getChannels();i++)
    {
      DEBUG_LOG4("Scale channel :" << i);
      
      m_ppChannels[i]->scaleRange(tMin, tMax,
                                  getMin(i), getMax(i));
    }
  }
  else
  {
    m_ppChannels[iChannel]->scaleRange(tMin, tMax,
                                       getMin(iChannel), getMax(iChannel));
  }
}

// }}}

// ---------------------------------------------------------------------
template<class Type>
void ICL<Type>::clear(int iChannel, Type tValue) 
  // {{{ open

{
  //---- Log Message ----
  DEBUG_LOG4("clear(int, type)");
  
  if(iChannel == -1) 
    {
      for(int i=0;i<m_iChannels;i++) 
        m_ppChannels[i]->clear(tValue);
    }
  else 
    {
      m_ppChannels[iChannel]->clear(tValue);
    }
}

  // }}}

// }}}

template class ICL<iclbyte>;
template class ICL<iclfloat>;

} //namespace icl
