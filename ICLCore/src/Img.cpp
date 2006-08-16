// {{{ <Replaced missing fold top mark>

/*
  Img.cpp

  Written by: Michael Götting (2004)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "Img.h"

namespace icl {

// {{{  constructors and destructors

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Size &s,int iChannels):
  // {{{ open

  ImgI(s,formatMatrix,icl::getDepth<Type>(),iChannels){
  FUNCTION_LOG("Img(" << s.width <<","<< w.height <<","<< iChannels << ")  this:" << this );
  
  //---- Img Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    {
      m_vecChannels.push_back(createChannel());
    }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Size& s, Format eFormat, int iChannels):
  // {{{ open

  ImgI(s,eFormat,icl::getDepth<Type>(),iChannels){
  FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," << translateFormat(eFormat) <<","<< iChannels << ")  this:" << this );
  
   //---- Img Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    {
      m_vecChannels.push_back(createChannel());
    }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Size &s, Format eFormat, int iChannels, Type** pptData):
  // {{{ open
  ImgI(s,eFormat,icl::getDepth<Type>(),iChannels){

  FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," << translateFormat(eFormat) <<","<< iChannels << ",Type**)  this:" << this);
   
  //---- Img Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    {
       m_vecChannels.push_back(SmartPtr<Type>(*pptData++,0));
    }
} 

  // }}}
 
//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Img<Type>& tSrc):
  // {{{ open

    ImgI(tSrc.getSize(),tSrc.getFormat(),tSrc.getDepth(),tSrc.getChannels())
{
  FUNCTION_LOG("this: " << this);
  
  m_vecChannels = tSrc.m_vecChannels;
  m_oROIOffset = tSrc.m_oROIOffset;
  m_oROISize = tSrc.m_oROISize;
}

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::~Img()
  // {{{ open
{
  FUNCTION_LOG("this: " << this);
}

  // }}}

// }}} 

// {{{  assign operator: shallow copy

//----------------------------------------------------------------------------
template<class Type>
Img<Type>& Img<Type>::operator=(const Img<Type>& tSrc)
{
  FUNCTION_LOG("");
  
  //---- Assign new channels to Img ----
  m_oSize = tSrc.m_oSize;
  m_eFormat = tSrc.getFormat();
  m_eDepth = tSrc.getDepth();
  m_iChannels = tSrc.getChannels();  
  m_vecChannels = tSrc.m_vecChannels;
  m_oROIOffset = tSrc.m_oROIOffset;
  m_oROISize = tSrc.m_oROISize;

  return *this;
}

// }}} 

// {{{  class organisation : 

//----------------------------------------------------------------------------
template<class Type> ImgI*
Img<Type>::deepCopy(ImgI* poDst) const
  // {{{ open

{
  FUNCTION_LOG("");

  if(!poDst){
    poDst = imgNew(getDepth(),getSize(),getFormat(),getChannels(),getROI());
  }else{
    ensureCompatible(&poDst,poDst->getDepth(),getSize(),getFormat(),getChannels(),getROI());
  }

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
          return convertTo<iclbyte>(poDst->asImg<iclbyte>());
        }
      else
        {
          return convertTo<iclfloat>(poDst->asImg<iclfloat>());
        }
    }
}

  // }}}

//--------------------------------------------------------------------------
template<class Type> ImgI*
Img<Type>::scaledCopy(ImgI *poDst,ScaleMode eScaleMode) const
  // {{{ open

{
  FUNCTION_LOG("");
  
  //---- deep copy case -----
  if(!poDst || isEqual(poDst->getSize(),poDst->getChannels())){
    SECTION_LOG("deep copy case");
    return deepCopy(poDst); 
  }
  
  //---- type conversion case -----------
  if(getDepth() != poDst->getDepth())
    {
      SECTION_LOG("type conversion case");
      ImgI *poTmp = imgNew(getDepth(),poDst->getSize(),formatMatrix,poDst->getChannels());
      scaledCopy(poTmp,eScaleMode);
      poTmp->deepCopy(poDst);
      delete poTmp;
      return poDst;
    }
  
  SECTION_LOG("scaling case");
  poDst->setNumChannels( getChannels() );
  poDst->setFormat( getFormat() );
    
#ifdef WITH_IPP_OPTIMIZATION
 
  Rect oFullROI(Point(0,0),getSize());
 
  for(int c=0;c<m_iChannels;c++)
    {
      SUBSECTION_LOG("channel: "<< c);
      if(getDepth()==depth8u)
        {
          ippiResize_8u_C1R(asImg<iclbyte>()->getData(c),getSize(),getLineStep(),oFullROI,
                            poDst->asImg<iclbyte>()->getData(c),poDst->getLineStep(),poDst->getROISize(),
                            (double)poDst->getSize().width/(double)getSize().width,
                            (double)poDst->getSize().height/(double)getSize().height,
                            (int)eScaleMode);
        }else{
          ippiResize_32f_C1R(asImg<iclfloat>()->getData(c),getSize(),getLineStep(),oFullROI,
                             poDst->asImg<iclfloat>()->getData(c),poDst->getLineStep(),poDst->getROISize(),                            
                             (double)poDst->getSize().width/(double)getSize().width,
                             (double)poDst->getSize().height/(double)getSize().height,
                             (int)eScaleMode);
        }
    } 
  
#else

  //---- Variable initilazation ----
  float fXStep = ((float)getWidth()-1)/(float)(poDst->getWidth()); 
  float fYStep = ((float)getHeight()-1)/(float)(poDst->getHeight());

  //---- scale Img ----
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
                  (*(poDst->asImg<iclbyte>()))(x,y,c)=static_cast<iclbyte>((*this)((int)rint(x*fXStep),(int)rint(y*fYStep),c));
                }
              }
            }else{
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  LOOP_LOG("interpolateNN: x:"<< x << " y:" << y);
                  (*(poDst->asImg<iclfloat>()))(x,y,c)=static_cast<iclfloat>((*this)((int)rint(x*fXStep),(int)rint(y*fYStep),c));
                }
              }
            }
            break;
          
          case interpolateLIN: 
            if(poDst->getDepth()==depth8u){
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  (*(poDst->asImg<iclbyte>()))(x,y,c)=static_cast<iclbyte>(interpolate((fXStep/2)+ x*fXStep,(fYStep/2)+y*fYStep,c));
                }
              }
            }else{
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  (*(poDst->asImg<iclfloat>()))(x,y,c)=static_cast<iclfloat>(interpolate((fXStep/2)+ x*fXStep,(fYStep/2)+y*fYStep,c));
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
template<class Type> ImgI*
Img<Type>::deepCopyROI(ImgI *poDst) const
  // {{{ open
{
  FUNCTION_LOG("");

  if(!poDst){
    poDst = imgNew(getDepth(),getROISize(),getFormat(),getChannels());
  }else{
    poDst->setNumChannels(getChannels());
    poDst->setFormat(getFormat());
  }
  if(getROISize() != poDst->getROISize())
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
            for(Img8u::iterator s=asImg<iclbyte>()->begin(c),d=poDst->asImg<iclbyte>()->begin(c); s.inRegion();s.incRow(),d.incRow()){
              memcpy(&*d,&*s,s.getROIWidth()*sizeof(iclbyte));
            }
#else
            ippiCopy_8u_C1R(asImg<iclbyte>()->getROIData(c),getLineStep(),
                            poDst->asImg<iclbyte>()->getROIData(c),poDst->getLineStep(),
                            getROISize());
#endif
          }else{
#ifndef WITH_IPP_OPTIMIZATION
             for(Img32f::iterator s=asImg<iclfloat>()->begin(c),d=poDst->asImg<iclfloat>()->begin(c); s.inRegion();s.incRow(),d.incRow()){
              memcpy(&*d,&*s,s.getROIWidth()*sizeof(iclfloat));
            }
#else
             ippiCopy_32f_C1R(asImg<iclfloat>()->getROIData(c),getLineStep(),
                              poDst->asImg<iclfloat>()->getROIData(c),poDst->getLineStep(),
                              getROISize());
#endif
          }
        }
      else
        {
          if(m_eDepth == depth8u){
#ifndef WITH_IPP_OPTIMIZATION
            Img8u::iterator s=asImg<iclbyte>()->begin(c);
            Img32f::iterator d=poDst->asImg<iclfloat>()->begin(c);
            for(;s.inRegion();d++,s++){
              *d = static_cast<iclfloat>(*s);
            }
#else
            ippiConvert_8u32f_C1R(asImg<iclbyte>()->getROIData(c),getLineStep(),
                                  poDst->asImg<iclfloat>()->getROIData(c),poDst->getLineStep(),
                                  getROISize());
#endif
          }else{
#ifndef WITH_IPP_OPTIMIZATION
            Img32f::iterator s=asImg<iclfloat>()->begin(c);
            Img8u::iterator d=poDst->asImg<iclbyte>()->begin(c);
            for(;s.inRegion();d++,s++){
              *d = static_cast<iclbyte>(*s);
            }
#else
            ippiConvert_32f8u_C1R(asImg<iclfloat>()->getROIData(c),getLineStep(),
                                  poDst->asImg<iclbyte>()->getROIData(c),poDst->getLineStep(),
                                  getROISize(),ippRndNear);
#endif
          }
        }
    }
  return poDst;
}

// }}}
  
//----------------------------------------------------------------------------
template<class Type> ImgI*
Img<Type>::scaledCopyROI(ImgI *poDst, ScaleMode eScaleMode) const
  // {{{ open
{

  FUNCTION_LOG("");
  
  //---- deep copy case -----
  if(!poDst || getROISize() == poDst->getROISize()){
    return deepCopyROI(poDst); 
  }

  //---- type conversion case -----------
  if(getDepth() != poDst->getDepth())
    {
      ImgI *poTmp = imgNew(getDepth(),poDst->getROISize(),formatMatrix,poDst->getChannels());
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
          ippiResize_8u_C1R(asImg<iclbyte>()->getROIData(c),getSize(),getLineStep(),getROI(),
                            poDst->asImg<iclbyte>()->getROIData(c),poDst->getLineStep(),poDst->getROISize(),
                            (double)poDst->getROISize().width/getROISize().width,
                            (double)poDst->getROISize().height/getROISize().height,
                            (int)eScaleMode);
        }else{
          ippiResize_32f_C1R(asImg<iclfloat>()->getROIData(c),getSize(),getLineStep(),getROI(),
                             poDst->asImg<iclfloat>()->getROIData(c),poDst->getLineStep(),poDst->getROISize(),
                             (double)poDst->getROISize().width/getROISize().width,
                             (double)poDst->getROISize().height/getROISize().height,
                             (int)eScaleMode);
        }
    }   
#else
  /// _VERY_ slow fallback implementation
 
  ImgI * poROITmp = imgNew(getDepth(),iW,iH,formatMatrix,m_iChannels);
  ImgI * poDstTmp = imgNew(getDepth(),iDstW,iDstH,formatMatrix,m_iChannels);
  
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
Img<Type>::detach(int iIndex)
  // {{{ open
{
  FUNCTION_LOG("detach(" << iIndex << ")");
  ICLASSERT(iIndex < getChannels());
  
  //---- Make the whole Img independent ----
  for(int i=iIndex<0?0:iIndex, iEnd=iIndex<0?m_iChannels:iIndex+1;i<iEnd;i++) 
    {
      m_vecChannels[i] = createChannel (getData(i));
    }
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
Img<Type>::removeChannel(int iChannel)
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
Img<Type>::append(const Img<Type>& oSrc, int iIndex)
  // {{{ open
{
  FUNCTION_LOG("");
  ICLASSERT_RETURN(oSrc.getSize() == getSize() );
  
  for(int i = iIndex < 0 ? 0 : iIndex, iEnd = iIndex < 0 ? m_iChannels : iIndex+1;
      i < iEnd; i++) {
     m_vecChannels.push_back(oSrc.m_vecChannels[i]); 
  }
  m_iChannels = m_vecChannels.size();
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void 
Img<Type>::swapChannels(int iIndexA, int iIndexB)
  // {{{ open
{
  FUNCTION_LOG("swapChannels("<<iIndexA<<","<< iIndexB<< ")");
  ICLASSERT_RETURN(iIndexA >= 0 && iIndexA < getChannels());
  ICLASSERT_RETURN(iIndexB >= 0 && iIndexB < getChannels());

  std::swap(m_vecChannels[iIndexA], m_vecChannels[iIndexB]);
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
Img<Type>::scale(const Size &s,ScaleMode eScaleMode)
  // {{{ open

{  
  FUNCTION_LOG("");
  
  Size oNewSize(s.width<0?getSize().width:s.width, s.height<0?getSize().height:s.height);
  //---- estimate destination values in respect to defaults ----
  
  if(! isEqual(oNewSize, m_iChannels))
    {
      Img<Type> oTmp(oNewSize,m_eFormat,m_iChannels);
      scaledCopy(&oTmp,eScaleMode);
      (*this)=oTmp;
    }
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
Img<Type>::resize(const Size &s)
  // {{{ open

{
  FUNCTION_LOG("");
  
  Size oNewSize(s.width<0?getSize().width:s.width, s.height<0?getSize().height:s.height);
  //---- estimate destination values in respect to defaults ----
  
  if(!isEqual(oNewSize,m_iChannels))
    {
      m_oSize = oNewSize;
      for(int i=0;i<m_iChannels;i++)
        {
          m_vecChannels[i] = createChannel ();
        }
    }
  setFullROI();
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
Img<Type>::setNumChannels(int iNumNewChannels)
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
Img<Type>::renew(const Size &s, int iNewNumChannels)
  // {{{ open
{
  FUNCTION_LOG("");
  
  resize(Size(s.width<0?getSize().width:s.width, s.height<0?getSize().height:s.height));
  setNumChannels(iNewNumChannels<0?getChannels():iNewNumChannels);
}

// }}}

//----------------------------------------------------------------------------
template<class Type> inline void 
Img<Type>::replaceChannel(int iThisIndex, const Img<Type>& oSrc, int iOtherIndex) 
  // {{{ open
{
  FUNCTION_LOG("");
  ICLASSERT_RETURN(iThisIndex >= 0 && iThisIndex < getChannels());
  ICLASSERT_RETURN(iOtherIndex >= 0 && iOtherIndex < oSrc.getChannels());
  m_vecChannels[iThisIndex] = oSrc.m_vecChannels[iOtherIndex];
}
// }}}

// }}}

// {{{  Type converter: 

//--------------------------------------------------------------------------

/********************************************************************** evtl direkt nach IclI.cpp
template<class Type> Img32f*
Img<Type>::convertTo32Bit(Img32f *poDst) const
// {{{ open

{
  FUNCTION_LOG("convertTo32Bit(Img32f*)");
  
  ImgI *poDstBase = static_cast<ImgI*>(poDst);
  iclEnsureCompatible(&poDstBase,depth32f,m_oSize.width,m_oSize.height,
                      m_eFormat,m_iChannels, &m_oROIOffset, &m_oROISize);
  poDst = poDstBase->asImg<iclfloat>(); // should only be needed in case of of poDst == NULL

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
    return asImg<iclfloat>()->deepCopy(poDst)->asImg<iclfloat>();
  }  
}

  // }}}

//--------------------------------------------------------------------------
template<class Type> Img8u*
Img<Type>::convertTo8Bit(Img8u *poDst) const
// {{{ open
{

  FUNCTION_LOG("convertTo8Bit(Img8u*)");

  ImgI *poDstBase = static_cast<ImgI*>(poDst);
  iclEnsureCompatible(&poDstBase,depth8u,m_oSize.width,m_oSize.height,
                      m_eFormat,m_iChannels, &m_oROIOffset, &m_oROISize);
  poDst = poDstBase->asImg<iclbyte>(); // should only be needed in case of of poDst == NULL

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
      return asImg<iclbyte>()->deepCopy(poDst)->asImg<iclbyte>();
    }  
}

*****************************************************************************************************/
  // }}}

// }}} 

// {{{  Getter Functions: 

// ---------------------------------------------------------------------
template<class Type> Type 
Img<Type>::getMax(int iChannel) const
  // {{{ open
{
  FUNCTION_LOG("getMax(" << iChannel << ")");
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      iclbyte ucMax;
      ippiMax_8u_C1R(asImg<iclbyte>()->getROIData(iChannel),getLineStep(),getROISize(),&ucMax);
      return static_cast<Type>(ucMax);
    }
  else
    {
      iclfloat fMax;
      ippiMax_32f_C1R(asImg<iclfloat>()->getROIData(iChannel),getLineStep(),getROISize(),&fMax);
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
Img<Type>::getMin(int iChannel) const
  // {{{ open
{
  FUNCTION_LOG("getMin(" << iChannel<< ")");
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      iclbyte ucMin;
      ippiMin_8u_C1R(asImg<iclbyte>()->getROIData(iChannel),getLineStep(),getROISize(),&ucMin);
      return static_cast<Type>(ucMin);
    }
  else
    {
      iclfloat fMin;
      ippiMin_32f_C1R(asImg<iclfloat>()->getROIData(iChannel),getLineStep(),getROISize(),&fMin);
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
Img<Type>::getMinMax(int iChannel, Type &rtMin, Type &rtMax) const
  // {{{ open

{
  FUNCTION_LOG("getMinMax(" << iChannel << ",int&, int&)");
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      iclbyte ucMin, ucMax;
      ippiMinMax_8u_C1R(asImg<iclbyte>()->getROIData(iChannel),getLineStep(),getROISize(),&ucMin, &ucMax);
      rtMin = static_cast<Type>(ucMin);
      rtMax = static_cast<Type>(ucMax);
    }
  else
    {
      iclfloat fMin,fMax;
      ippiMinMax_32f_C1R(asImg<iclfloat>()->getROIData(iChannel),getLineStep(),getROISize(),&fMin, &fMax);
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
Img<Type>::interpolate(float fX, float fY, int iChannel) const
{
  // {{{ open
  LOOP_LOG("(" << fX << "," << fY << "," << iChannel << ")");

  //---- Variable initialization ----
  float fY1,fY2,fY3,fY4,fT,fU;
  float fReturn;
  
  fY1 = fY2 = fY3 = fY4 = fT = fU = 0;
  
  //---- interpolate  ----
  ICLASSERT_RETURN_VAL((int)ceil(fX) < getSize().width, 0);
  ICLASSERT_RETURN_VAL((int)ceil(fY) < getSize().height, 0);

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
SmartPtr<Type> Img<Type>::createChannel(Type *ptDataToCopy) const
{
  Type *ptNewData =  new Type[getDim()];
  if(ptDataToCopy){
    memcpy(ptNewData,ptDataToCopy,getDim()*sizeof(Type));
  }else{
    fill(ptNewData,ptNewData+getDim(),0);
  }
  return SmartPtr<Type>(ptNewData);
}

//--------------------------------------------------------------------------
template<class Type> void
Img<Type>::scaleRange(float fMin, float fMax, int iChannel)
  // {{{ open

{
  FUNCTION_LOG("scaleRange(" << fMin << "," << fMax << "," << iChannel << ")");
  Type tMin, tMax;
  getMinMax(iChannel,tMin,tMax);
  scaleRange(fMin,fMax,static_cast<float>(tMin),static_cast<float>(tMax),iChannel);
}

// }}}

template <class Type> void 
Img<Type>::scaleRange(float fNewMin,float fNewMax,float fMin,float fMax, int iChannel)
  // {{{ open
{
  FUNCTION_LOG("scaleRange(" << fNewMin << "," << fNewMax << ","<< fMin << "," << fMax << "," << iChannel << ")");
  
  float fScale  = (fNewMax - fNewMin) / (fMax - fMin);
  float fShift  = (fMax * fNewMin - fMin * fNewMax) / (fMax - fMin);
  iclfloat fPixel;
  int iChannelEnd = iChannel < 0 ? getChannels() : iChannel+1;
  for(int c = (iChannel<0) ? 0 : iChannel; c < iChannelEnd ;c++)
    {
      for(iterator p=getROIIterator(c); p.inRegion(); ++p)
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
void Img<Type>::clear(int iChannel, Type tValue) 
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

template class Img<iclbyte>;
template class Img<iclfloat>;

} //namespace icl
