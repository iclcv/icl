/*
  ICL.cpp

  Written by: Michael Götting (2004)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "ICL.h"

namespace icl {

// {{{  constructors and destructors

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(int iWidth,int iHeight,int iChannels):
  // {{{ open

  ICLBase(iWidth,iHeight,formatMatrix,iclGetDepth<Type>(),iChannels){
  FUNCTION_LOG("ICL(" << iWidth <<","<< iHeight <<","<< iChannels << ")  this:" << this );
  
  //---- ICL Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    {
      m_vecChannels.push_back(createChannel());
    }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(int iWidth, int iHeight, iclformat eFormat, int iChannels):
  // {{{ open

  ICLBase(iWidth,iHeight,eFormat,iclGetDepth<Type>(),iChannels){
  FUNCTION_LOG("ICL(" << iWidth <<","<< iHeight << "," << iclTranslateFormat(eFormat) <<","<< iChannels << ")  this:" << this );
  
   //---- ICL Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    {
      m_vecChannels.push_back(createChannel());
    }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(int iWidth, int iHeight, iclformat eFormat, int iChannels, Type** pptData):
  // {{{ open
  ICLBase(iWidth,iHeight,eFormat,iclGetDepth<Type>(),iChannels){

  FUNCTION_LOG("ICL(" << iWidth <<","<< iHeight << "," << iclTranslateFormat(eFormat) <<","<< iChannels << ",Type**)  this:" << this);
   
  //---- ICL Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    {
       m_vecChannels.push_back(ICLAutoPtr<Type>(*pptData++,0));
    }
} 

  // }}}
 
//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(const ICL<Type>& tSrc):
  // {{{ open

    ICLBase(tSrc.getWidth(),tSrc.getHeight(),
            tSrc.getFormat(),tSrc.getDepth(),tSrc.getChannels())
{
  FUNCTION_LOG("this: " << this);
  
  m_iChannels = tSrc.getChannels();
  m_vecChannels = tSrc.m_vecChannels;
  m_oROIoffset = tSrc.m_oROIoffset;
  m_oROIsize = tSrc.m_oROIsize;
}

  // }}}

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::~ICL()
  // {{{ open
{
  FUNCTION_LOG("this: " << this);
}

  // }}}

// }}} 

// {{{  assign operator: shallow copy

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>& ICL<Type>::operator=(const ICL<Type>& tSrc)
{
  FUNCTION_LOG("");
  
  //---- Assign new channels to ICL ----
  m_oSize = tSrc.m_oSize;
  m_eFormat = tSrc.getFormat();
  m_eDepth = tSrc.getDepth();
  m_iChannels = tSrc.getChannels();  
  m_vecChannels = tSrc.m_vecChannels;
  m_oROIoffset = tSrc.m_oROIoffset;
  m_oROIsize = tSrc.m_oROIsize;

  return *this;
}

// }}} 

// {{{  class organisation : 

//----------------------------------------------------------------------------
template<class Type> ICLBase*
ICL<Type>::deepCopy(ICLBase* poDst) const
  // {{{ open

{
  FUNCTION_LOG("");

  iclEnsureCompatible(&poDst,(ICLBase*)this);

  if(poDst->getDepth() == getDepth())
    {
      for(int c=0;c<m_iChannels;c++)
        {
          memcpy(poDst->getDataPtr(c), getDataPtr(c), getDim()*sizeof(Type));
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
  FUNCTION_LOG("");
  
  //---- deep copy case -----
  if(!poDst || isEqual(poDst->getWidth(),poDst->getHeight(),poDst->getChannels())){
    SECTION_LOG("deep copy case");
    return deepCopy(poDst); 
  }
  
  //---- type conversion case -----------
  if(getDepth() != poDst->getDepth())
    {
      SECTION_LOG("type conversion case");
      ICLBase *poTmp = iclNew(getDepth(),poDst->getWidth(),poDst->getHeight(),formatMatrix,poDst->getChannels());
      scaledCopy(poTmp,eScaleMode);
      poTmp->deepCopy(poDst);
      delete poTmp;
      return poDst;
    }
  
  SECTION_LOG("scaling case");
  poDst->setNumChannels( getChannels() );
  poDst->setFormat( getFormat() );
  poDst->delROI();
    
#ifdef WITH_IPP_OPTIMIZATION
 
  IppiRect oWholeImageROI = {0,0,m_oSize.width,m_oSize.height};
 
  for(int c=0;c<m_iChannels;c++)
    {
      SUBSECTION_LOG("channel: "<< c);
      if(getDepth()==depth8u)
        {
          ippiResize_8u_C1R((Ipp8u*)getDataPtr(c),ippSize(),ippStep(),oWholeImageROI,
                            poDst->roiData8u(c),poDst->ippStep(),poDst->getROISize(),
                            (double)poDst->getWidth()/(double)getWidth(),
                            (double)poDst->getHeight()/(double)getHeight(),
                            (int)eScaleMode);
        }else{
          ippiResize_32f_C1R((Ipp32f*)getDataPtr(c),ippSize(),ippStep(),oWholeImageROI,
                             poDst->roiData32f(c),poDst->ippStep(),poDst->getROISize(),
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
      SUBSECTION_LOG("channel: "<< c);
      //---- Take the correct scaling method ----
      switch(eScaleMode) 
        {
          case interpolateNN: 
            if(poDst->getDepth()==depth8u){
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  LOOP_LOG("interpolateNN: x:"<< x << " y:" << y);
                  (*(poDst->asIcl8u()))(x,y,c)=static_cast<iclbyte>((*this)((int)rint(x*fXStep),(int)rint(y*fYStep),c));
                }
              }
            }else{
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  LOOP_LOG("interpolateNN: x:"<< x << " y:" << y);
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
  FUNCTION_LOG("");

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
      return poDst;
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
            ippiCopy_8u_C1R(roiData8u(c),ippStep(),poDst->roiData8u(c),poDst->ippStep(),getROISize());
#endif
          }else{
#ifndef WITH_IPP_OPTIMIZATION
             for(ICL32f::iterator s=asIcl32f()->begin(c),d=poDst->asIcl32f()->begin(c); s.inRegion();s.incRow(),d.incRow()){
              memcpy(&*d,&*s,s.getROIWidth()*sizeof(iclfloat));
            }
#else
             ippiCopy_32f_C1R(roiData32f(c),ippStep(),poDst->roiData32f(c),poDst->ippStep(),getROISize());
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
            ippiConvert_8u32f_C1R(roiData8u(c),ippStep(),poDst->roiData32f(c),poDst->ippStep(),getROISize());
#endif
          }else{
#ifndef WITH_IPP_OPTIMIZATION
            ICL32f::iterator s=asIcl32f()->begin(c);
            ICL8u::iterator d=poDst->asIcl8u()->begin(c);
            for(;s.inRegion();d++,s++){
              *d = static_cast<iclbyte>(*s);
            }
#else
            ippiConvert_32f8u_C1R(roiData32f(c),ippStep(),poDst->roiData8u(c),poDst->ippStep(),getROISize(),ippRndNear);
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

  FUNCTION_LOG("");
  
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
          ippiResize_8u_C1R(roiData8u(c),ippSize(),ippStep(),getROI(),
                            poDst->roiData8u(c),poDst->ippStep(),poDst->getROISize(),
                            (double)iDstW/iW,
                            (double)iDstH/iH,
                            (int)eScaleMode);
        }else{
          ippiResize_32f_C1R(roiData32f(c),ippSize(),ippStep(),getROI(),
                             poDst->roiData32f(c),poDst->ippStep(),poDst->getROISize(),
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
  FUNCTION_LOG("detach(" << iIndex << ")");
  ICLASSERT(iIndex < getChannels());
  
  //---- Make the whole ICL independent ----
  for(int i=iIndex<0?0:iIndex, iEnd=iIndex<0?m_iChannels:iIndex+1;i<iEnd;i++) 
    {
      m_vecChannels[i] = createChannel (getData(i));
    }
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::removeChannel(int iChannel)
  // {{{ open
{
  FUNCTION_LOG("removeChannel(" << iChannel << ")");
  ICLASSERT_RETURN(iChannel < getChannels());
  
  m_vecChannels.erase(m_vecChannels.begin()+iChannel);
  m_iChannels--;
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::append(ICL<Type> *poSrc)
  // {{{ open
{
  FUNCTION_LOG("");
  
  ICLASSERT_RETURN(poSrc->getWidth() == getWidth());
  ICLASSERT_RETURN(poSrc->getHeight() == getHeight());
  
  for(int i=0;i<poSrc->m_iChannels;i++,m_iChannels++){
    m_vecChannels.push_back(poSrc->m_vecChannels[i]); 
  }      
} 

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::appendChannel(int iChannel, ICL<Type> *poSrc) 
  // {{{ open
{
  FUNCTION_LOG("appendChannel("<< iChannel <<",ICL<Type>*)");
  
  ICLASSERT_RETURN(poSrc->getWidth() == getWidth());
  ICLASSERT_RETURN(poSrc->getHeight() == getHeight());
  ICLASSERT_RETURN(iChannel < poSrc->getChannels());
  
  m_vecChannels.push_back(poSrc->m_vecChannels[iChannel]); 
  m_iChannels++;
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void 
ICL<Type>::swapChannels(int iIndexA, int iIndexB)
  // {{{ open
{
  FUNCTION_LOG("swapChannels("<<iIndexA<<","<< iIndexB<< ")");
  ICLASSERT_RETURN(iIndexA < getChannels());
  ICLASSERT_RETURN(iIndexB < getChannels());

  std::swap(m_vecChannels[iIndexA], m_vecChannels[iIndexB]);
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::scale(int iNewWidth,int iNewHeight,iclscalemode eScaleMode)
  // {{{ open

{  
  FUNCTION_LOG("");
  
  //---- estimate destination values in respect to defaults ----
  if(iNewWidth < 0) iNewWidth = m_oSize.width;
  if(iNewHeight < 0) iNewHeight = m_oSize.height;
  
  if(! isEqual(iNewWidth,iNewHeight,m_iChannels))
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
  FUNCTION_LOG("");
  
  //---- estimate destination values in respect to defaults ----
  if(iWidth < 0) iWidth = m_oSize.width;
  if(iHeight < 0) iHeight = m_oSize.height;
  
  if(!isEqual(iWidth,iHeight,m_iChannels))
    {
      m_oSize.width = iWidth;
      m_oSize.height = iHeight;
      for(int i=0;i<m_iChannels;i++)
        {
          m_vecChannels[i] = createChannel ();
        }
    }
  delROI();
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
ICL<Type>::setNumChannels(int iNumNewChannels)
  // {{{ open
{
  FUNCTION_LOG("");

  if(iNumNewChannels < 0)
    {
      ERROR_LOG("channel count must be >= 0");
      return;
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
        m_vecChannels.push_back(createChannel());
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
  FUNCTION_LOG("");

  if(iNewWidth < 0)iNewWidth = m_oSize.width;
  if(iNewHeight < 0)iNewHeight = m_oSize.height;
  if(iNewNumChannels < 0)iNewNumChannels = m_iChannels;

  resize(iNewWidth,iNewHeight);
  setNumChannels(iNewNumChannels);
}

// }}}

//----------------------------------------------------------------------------
template<class Type> inline void 
ICL<Type>::replaceChannel(int iIndexA,int iIndexB,ICL<Type>  *poSrc) 
  // {{{ open
{
  FUNCTION_LOG("");
  ICLASSERT_RETURN(iIndexA < getChannels());
  ICLASSERT_RETURN(iIndexB < poSrc->getChannels());
  m_vecChannels[iIndexA] = poSrc->m_vecChannels[iIndexB];
}
// }}}

// }}} 

// {{{  Type converter: 

//--------------------------------------------------------------------------
template<class Type> ICL32f*
ICL<Type>::convertTo32Bit(ICL32f *poDst) const
// {{{ open

{
  FUNCTION_LOG("convertTo32Bit(ICL32f*)");
  
  ICLBase *poDstBase = static_cast<ICLBase*>(poDst);
  iclEnsureCompatible(&poDstBase,depth32f,m_oSize.width,m_oSize.height,
                      m_eFormat,m_iChannels, &m_oROIoffset, &m_oROIsize);
  poDst = poDstBase->asIcl32f(); // should only be needed in case of of poDst == NULL

  if(m_eDepth == depth8u)
  {
    for(int c=0;c<m_iChannels;c++)
    {
      iclbyte *pucSrc = reinterpret_cast<iclbyte*>(getDataPtr(c));
      iclfloat *pfDst = reinterpret_cast<iclfloat*>(poDst->getDataPtr(c));
#ifdef WITH_IPP_OPTIMIZATION
      IppiSize oWholeImageROI = {m_oSize.width,m_oSize.height};
      ippiConvert_8u32f_C1R(pucSrc,ippStep(),pfDst,poDst->ippStep(),oWholeImageROI);
#else
      iclbyte *pucSrcEnd = pucSrc+getDim();
      while(pucSrc!=pucSrcEnd){
        *pfDst++ = static_cast<iclbyte>(*pucSrc++);
      }
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

  FUNCTION_LOG("convertTo8Bit(ICL8u*)");

  ICLBase *poDstBase = static_cast<ICLBase*>(poDst);
  iclEnsureCompatible(&poDstBase,depth8u,m_oSize.width,m_oSize.height,
                      m_eFormat,m_iChannels, &m_oROIoffset, &m_oROIsize);
  poDst = poDstBase->asIcl8u(); // should only be needed in case of of poDst == NULL

  if(m_eDepth == depth32f)
    {
      for(int c=0;c<m_iChannels;c++)
        {
          iclfloat *pfSrc = reinterpret_cast<iclfloat*>(getDataPtr(c));
          iclbyte *pucDst =  reinterpret_cast<iclbyte*>(poDst->getDataPtr(c));
#ifdef WITH_IPP_OPTIMIZATION
          IppiSize oWholeImageROI = {m_oSize.width,m_oSize.height};
          ippiConvert_32f8u_C1R(pfSrc,ippStep(),pucDst,poDst->ippStep(),oWholeImageROI,ippRndNear);
#else
          iclfloat *pfSrcEnd = pfSrc+getDim();
          while(pfSrc!=pfSrcEnd){
            *pucDst++ = static_cast<iclbyte>(*pfSrc++);
          }
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
  FUNCTION_LOG("getMax(" << iChannel << ")");
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      iclbyte ucMax;
      ippiMax_8u_C1R(roiData8u(iChannel),ippStep(),getROISize(),&ucMax);
      return static_cast<Type>(ucMax);
    }
  else
    {
      iclfloat fMax;
      ippiMax_32f_C1R(roiData32f(iChannel),ippStep(),getROISize(),&fMax);
      return static_cast<Type>(fMax);
    }
#else
  Type *ptData = getData(iChannel);
  Type *ptDataEnd = ptData+getDim();
  if(ptData == ptDataEnd)return 0;
  Type tMax = *ptData++;
  while(ptData != ptDataEnd)
    {
      tMax = std::max(tMax,*ptData++);
    }
  return tMax;
#endif
}

  // }}}

// ---------------------------------------------------------------------  
template<class Type> Type 
ICL<Type>::getMin(int iChannel) const
  // {{{ open
{
  FUNCTION_LOG("getMin(" << iChannel<< ")");
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      iclbyte ucMin;
      ippiMin_8u_C1R(roiData8u(iChannel),ippStep(),getROISize(),&ucMin);
      return static_cast<Type>(ucMin);
    }
  else
    {
      iclfloat fMin;
      ippiMin_32f_C1R(roiData32f(iChannel),ippStep(),getROISize(),&fMin);
      return static_cast<Type>(fMin);
    }
                     
#else
  Type *ptData = getData(iChannel);
  Type *ptDataEnd = ptData+getDim();
  if(ptData == ptDataEnd)return 0;
  Type tMin = *ptData++;
  while(ptData != ptDataEnd)
    {
      tMin = std::min(tMin,*ptData++);
    }
  return tMin;
#endif
}

  // }}}
  
// ---------------------------------------------------------------------  
template<class Type> void 
ICL<Type>::getMinMax(int iChannel, Type &rtMin, Type &rtMax) const
  // {{{ open

{
  FUNCTION_LOG("getMinMax(" << iChannel << ",int&, int&)");
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      iclbyte ucMin, ucMax;
      ippiMinMax_8u_C1R(roiData8u(iChannel),ippStep(),getROISize(),&ucMin, &ucMax);
      rtMin = static_cast<Type>(ucMin);
      rtMax = static_cast<Type>(ucMax);
    }
  else
    {
      iclfloat fMin,fMax;
      ippiMinMax_32f_C1R(roiData32f(iChannel),ippStep(),getROISize(),&fMin, &fMax);
      rtMin =  static_cast<Type>(fMin);
      rtMax =  static_cast<Type>(fMax);
    }
                     
#else
  Type *ptData = getData(iChannel);
  Type *ptDataEnd = ptData+getDim();
  if(ptData == ptDataEnd){
    rtMin = 0;
    rtMax = 0;
    return;
  }
  rtMin = *ptData;
  rtMax = *ptData++;
  while(ptData != ptDataEnd)
    {
      rtMin = std::min(rtMin,*ptData);
      rtMax = std::max(rtMin,*ptData++);
    }
#endif
}

// }}}

// }}}

// {{{  Auxillary and basic image manipulation functions

template<class Type> Type
ICL<Type>::interpolate(float fX, float fY, int iChannel) const
{
  // {{{ open
  LOOP_LOG("(" << fX << "," << fY << "," << iChannel << ")");

  //---- Variable initialization ----
  float fY1,fY2,fY3,fY4,fT,fU;
  float fReturn;
  
  fY1 = fY2 = fY3 = fY4 = fT = fU = 0;
  
  //---- interpolate  ----
  ICLASSERT_RETURN_VAL((int)ceil(fX) < getWidth(), 0);
  ICLASSERT_RETURN_VAL((int)ceil(fY) < getHeight(), 0);

  fY1=(float)(*this)((int)floor(fX), (int)floor(fY), iChannel);
  fY4=(float)(*this)((int)floor(fX), (int)ceil(fY), iChannel);
  fY3=(float)(*this)((int)ceil(fX), (int)ceil(fY), iChannel);
  fY2=(float)(*this)((int)ceil(fX), (int)floor(fY), iChannel);
  fT=fX-floor(fX);
  fU=fY-floor(fY);
  
  fReturn=(1-fT)*(1-fU)*fY1+ fT*(1-fU)*fY2 + fT*fU*fY3 + (1-fT)*fU*fY4;
  
  //---- return ----
  if(m_eDepth == depth8u)
    return (Type) rint(fReturn);
  else
    return (Type) fReturn;
}

  // }}}
template<class Type>
ICLAutoPtr<Type> ICL<Type>::createChannel(Type *ptDataToCopy) const
{
  Type *ptNewData =  new Type[getDim()];
  if(ptDataToCopy){
    memcpy(ptNewData,ptDataToCopy,getDim()*sizeof(Type));
  }else{
    fill(ptNewData,ptNewData+getDim(),0);
  }
  return ICLAutoPtr<Type>(ptNewData);
}

//--------------------------------------------------------------------------
template<class Type> void
ICL<Type>::scaleRange(float fMin, float fMax, int iChannel)
  // {{{ open

{
  FUNCTION_LOG("scaleRange(" << fMin << "," << fMax << "," << iChannel << ")");
  Type tMin, tMax;
  getMinMax(iChannel,tMin,tMax);
  scaleRange(fMin,fMax,static_cast<float>(tMin),static_cast<float>(tMax),iChannel);
}

// }}}

template <class Type> void 
ICL<Type>::scaleRange(float fNewMin,float fNewMax,float fMin,float fMax, int iChannel)
  // {{{ open
{
  FUNCTION_LOG("scaleRange(" << fNewMin << "," << fNewMax << ","<< fMin << "," << fMax << "," << iChannel << ")");
  
  float fScale  = (fNewMax - fNewMin) / (fMax - fMin);
  float fShift  = (fMax * fNewMin - fMin * fNewMax) / (fMax - fMin);
  iclfloat fPixel;
  int iChannelEnd = iChannel < 0 ? getChannels() : iChannel+1;
  for(int c = (iChannel<0) ? 0 : iChannel; c < iChannelEnd ;c++)
    {
      for(iterator p=begin(c);p.inRegion();p++)
        {
          fPixel = static_cast<iclfloat>(*p);
          fPixel=fPixel*fScale+fShift;
          if(fPixel<=fNewMin)
            fPixel=fNewMin;
          else if(fPixel>=fNewMax)
            fPixel=fNewMax;
          
          *p = static_cast<Type>(fPixel);
          
        }
    }
 
}

// }}}

// ---------------------------------------------------------------------
template<class Type>
void ICL<Type>::clear(int iChannel, Type tValue) 
  // {{{ open
{
  //---- Log Message ----
  FUNCTION_LOG("clear(" << iChannel << "," << tValue << ")");
  
  
  int iEnd = (iChannel<0)?m_iChannels:iChannel+1;
  for(int i=iChannel<0?0:iChannel;i<iEnd ;i++) {
    fill(getData(i),getData(i)+getDim(),tValue);
  }
}
  // }}}

// }}}

template class ICL<iclbyte>;
template class ICL<iclfloat>;

} //namespace icl
