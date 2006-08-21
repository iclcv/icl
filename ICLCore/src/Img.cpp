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
  FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," << iChannels << ")  this:" << this );
  
  //---- Img Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    {
      m_vecChannels.push_back(createChannel());
    }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Size& s, format eFormat, int iChannels):
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
Img<Type>::Img(const Size &s, format eFormat, int iChannels, Type** pptData):
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
  
  if(!poDst) poDst = imgNew(getDepth());
  if(poDst->getDepth() == depth8u){
    return convertTo<icl8u>(poDst->asImg<icl8u>());
  }else{
    return convertTo<icl32f>(poDst->asImg<icl32f>());
  }
}

  // }}}

//--------------------------------------------------------------------------
template<class Type> ImgI*
Img<Type>::scaledCopy(ImgI *poDst,scalemode eScaleMode) const
  // {{{ open
{
  FUNCTION_LOG("");
  
  if(!poDst || isEqual(poDst->getSize(),poDst->getChannels())){
    SECTION_LOG("deep copy case");
    return deepCopy(poDst); 
  }

  poDst->setFormat(getFormat());
  poDst->setChannels(getChannels());

  if(getDepth() == depth8u){
    if(poDst->getDepth() ==  depth8u){
      for(int c=0;c<getChannels();c++){
        scaleChannelROI<icl8u,icl8u>(asImg<icl8u>(),c,Point::zero,getSize(),poDst->asImg<icl8u>(),c,Point::zero,poDst->getSize(),eScaleMode);
      }
    }else{
      for(int c=0;c<getChannels();c++){
        scaleChannelROI<icl8u,icl32f>(asImg<icl8u>(),c,Point::zero,getSize(),poDst->asImg<icl32f>(),c,Point::zero,poDst->getSize(),eScaleMode);
      }
    }
  }else{
    if(poDst->getDepth() ==  depth8u){
      for(int c=0;c<getChannels();c++){
        scaleChannelROI<icl32f,icl8u>(asImg<icl32f>(),c,Point::zero,getSize(),poDst->asImg<icl8u>(),c,Point::zero,poDst->getSize(),eScaleMode);
      }
    }else{
      for(int c=0;c<getChannels();c++){
        scaleChannelROI<icl32f,icl32f>(asImg<icl32f>(),c,Point::zero,getSize(),poDst->asImg<icl32f>(),c,Point::zero,poDst->getSize(),eScaleMode);
      }
    }  
  }
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
    poDst->setChannels(getChannels());
    poDst->setFormat(getFormat());
  }
  ICLASSERT_RETURN_VAL( getROISize() == poDst->getROISize() , poDst);

  for(int c=0;c<m_iChannels;c++) {
    if(getDepth()==depth8u){
      if(poDst->getDepth()==depth8u){
        deepCopyChannelROI<icl8u,icl8u>(this->asImg<icl8u>(),  c, getROIOffset(),       getROISize(),
                                        poDst->asImg<icl8u>(), c, poDst->getROIOffset(),poDst->getROISize());
      }else{
        deepCopyChannelROI<icl8u,icl32f>(this->asImg<icl8u>(),  c, getROIOffset(),       getROISize(),
                                         poDst->asImg<icl32f>(), c, poDst->getROIOffset(),poDst->getROISize());
      }
    }else{
      if(poDst->getDepth()==depth8u){
        deepCopyChannelROI<icl32f,icl8u>(this->asImg<icl32f>(), c, getROIOffset(),       getROISize(),
                                        poDst->asImg<icl8u>(),  c, poDst->getROIOffset(),poDst->getROISize());
      }else{
        deepCopyChannelROI<icl32f,icl32f>(this->asImg<icl32f>(), c, getROIOffset(),       getROISize(),
                                         poDst->asImg<icl32f>(), c, poDst->getROIOffset(),poDst->getROISize());
      }
    }
  }
  return poDst;
}
  // }}}

//----------------------------------------------------------------------------
template<class Type> ImgI*
Img<Type>::scaledCopyROI(ImgI *poDst, scalemode eScaleMode) const
  // {{{ open
{
  FUNCTION_LOG("");
  
  if(!poDst || getROISize() == poDst->getROISize()){
    return deepCopyROI(poDst); 
  }

  poDst->setFormat(getFormat());
  poDst->setChannels(getChannels());

  if(getDepth() == depth8u){
    if(poDst->getDepth() ==  depth8u){
      for(int c=0;c<getChannels();c++){
        scaleChannelROI<icl8u,icl8u>(asImg<icl8u>(),c,getROIOffset(),getROISize(),
                                     poDst->asImg<icl8u>(),c,poDst->getROIOffset(), poDst->getROISize(),
                                     eScaleMode);
      }
    }else{
      for(int c=0;c<getChannels();c++){
        scaleChannelROI<icl8u,icl32f>(asImg<icl8u>(),c,getROIOffset(),getROISize(),
                                      poDst->asImg<icl32f>(),c,poDst->getROIOffset(), poDst->getROISize(),
                                      eScaleMode);
      }
    }
  }else{
    if(poDst->getDepth() ==  depth8u){
      for(int c=0;c<getChannels();c++){
        scaleChannelROI<icl32f,icl8u>(asImg<icl32f>(),c,getROIOffset(),getROISize(),
                                      poDst->asImg<icl8u>(),c,poDst->getROIOffset(), poDst->getROISize(),
                                      eScaleMode);
      }
    }else{
      for(int c=0;c<getChannels();c++){
        scaleChannelROI<icl32f,icl32f>(asImg<icl32f>(),c,getROIOffset(),getROISize(),
                                       poDst->asImg<icl32f>(),c,poDst->getROIOffset(), poDst->getROISize(),
                                       eScaleMode);
      }
    }  
  }
  return poDst;
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
Img<Type>::detach(int iIndex)
  // {{{ open
{
  FUNCTION_LOG("detach(" << iIndex << ")");
  ICLASSERT_RETURN(iIndex < getChannels());
  
  //---- Make the whole Img independent ----
  for(int i=getStartIndex(iIndex),iEnd=getEndIndex(iIndex);i<iEnd;i++)
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

  if(getFormat() != formatMatrix){
    WARNING_LOG("format was set to formatMatrix to ensure compability");
    setFormat(formatMatrix);
  }
  
  m_vecChannels.erase(m_vecChannels.begin()+iChannel);
  m_iChannels--;
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
Img<Type>::append(Img<Type> *poSrc, int iIndex)
  // {{{ open
{
  FUNCTION_LOG("");
  ICLASSERT_RETURN( poSrc );
  ICLASSERT_RETURN( poSrc->getSize() == getSize() );

  if(getFormat() != formatMatrix){
    WARNING_LOG("format was set to formatMatrix to ensure compability");
    setFormat(formatMatrix);
  }
  
  for(int i=getStartIndex(iIndex),iEnd=getEndIndex(iIndex);i<iEnd;i++)
    {
      m_vecChannels.push_back(poSrc->m_vecChannels[i]); 
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
Img<Type>::scale(const Size &s,scalemode eScaleMode)
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
Img<Type>::setChannels(int iNumNewChannels)
  // {{{ open
{
  FUNCTION_LOG("");
  ICLASSERT_RETURN(iNumNewChannels >= 0);
  
  if(getChannels() != iNumNewChannels && getFormat() != formatMatrix){
    WARNING_LOG("format was set to formatMatrix to ensure compability");
    setFormat(formatMatrix);
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
  setChannels(iNewNumChannels<0?getChannels():iNewNumChannels);
}

// }}}

//----------------------------------------------------------------------------
template<class Type> inline void 
Img<Type>::replaceChannel(int iThisIndex, Img<Type>* poSrc, int iOtherIndex) 
  // {{{ open
{
  FUNCTION_LOG("");
  ICLASSERT_RETURN(iThisIndex >= 0 && iThisIndex < getChannels());
  ICLASSERT_RETURN(iOtherIndex >= 0 && iOtherIndex < poSrc->getChannels());
  m_vecChannels[iThisIndex] = poSrc->m_vecChannels[iOtherIndex];
}
// }}}

// }}}

// {{{  Getter Functions: 

// ---------------------------------------------------------------------
template<class Type> Type 
Img<Type>::getMax(int iChannel) const
  // {{{ open
{
  FUNCTION_LOG("getMax(" << iChannel << ")");
  ICLASSERT_RETURN_VAL( iChannel < getChannels() ,0);
  ICLASSERT_RETURN_VAL( getChannels() > 0 ,0);
  if(iChannel < 0 ){
    Type tMax = getMax(0);
    for(int i=1;i<getChannels();i++){
      tMax = std::max(tMax,getMax(i));
    }
    return tMax;    
  }
  
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      icl8u ucMax;
      ippiMax_8u_C1R(asImg<icl8u>()->getROIData(iChannel),getLineStep(),getROISize(),&ucMax);
      return static_cast<Type>(ucMax);
    }
  else
    {
      icl32f fMax;
      ippiMax_32f_C1R(asImg<icl32f>()->getROIData(iChannel),getLineStep(),getROISize(),&fMax);
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
  ICLASSERT_RETURN_VAL( iChannel < getChannels() ,0);
  ICLASSERT_RETURN_VAL( getChannels() > 0 ,0);

  if(iChannel < 0 ){
    Type tMin = getMin(0);
    for(int i=1;i<getChannels();i++){
      tMin = std::min(tMin,getMin(i));
    }
    return tMin;    
  }


#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      icl8u ucMin;
      ippiMin_8u_C1R(asImg<icl8u>()->getROIData(iChannel),getLineStep(),getROISize(),&ucMin);
      return static_cast<Type>(ucMin);
    }
  else
    {
      icl32f fMin;
      ippiMin_32f_C1R(asImg<icl32f>()->getROIData(iChannel),getLineStep(),getROISize(),&fMin);
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
Img<Type>::getMinMax(Type &rtMin, Type &rtMax, int iChannel) const
  // {{{ open
{
  FUNCTION_LOG("getMinMax(" << iChannel << ",int&, int&)");
  ICLASSERT_RETURN( iChannel < getChannels() );
  ICLASSERT_RETURN( getChannels() > 0 );

  if(iChannel < 0 ){
    Type tMin,tMax;
    getMinMax(tMin,tMax,0);
    for(int i=1;i<getChannels();i++){
      Type tMinC,tMaxC;
      getMinMax(tMinC,tMaxC,i);
      tMin = std::min(tMin,tMinC);
      tMax = std::max(tMax,tMaxC);
    }
    return;
  }


#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      icl8u ucMin, ucMax;
      ippiMinMax_8u_C1R(asImg<icl8u>()->getROIData(iChannel),getLineStep(),getROISize(),&ucMin, &ucMax);
      rtMin = static_cast<Type>(ucMin);
      rtMax = static_cast<Type>(ucMax);
    }
  else
    {
      icl32f fMin,fMax;
      ippiMinMax_32f_C1R(asImg<icl32f>()->getROIData(iChannel),getLineStep(),getROISize(),&fMin, &fMax);
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

// {{{  Auxillary  functions 

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

  // }}}

// {{{  Basic image manipulation functions
//--------------------------------------------------------------------------
template<class Type> void
Img<Type>::scaleRange(float fMin, float fMax, int iChannel)
  // {{{ open

{
  FUNCTION_LOG("scaleRange(" << fMin << "," << fMax << "," << iChannel << ")");
  Type tMin, tMax;
  getMinMax(tMin,tMax,iChannel);
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
  icl32f fPixel;
  int iChannelEnd = iChannel < 0 ? getChannels() : iChannel+1;
  for(int c = (iChannel<0) ? 0 : iChannel; c < iChannelEnd ;c++)
    {
      for(iterator p=getROIIterator(c); p.inRegion(); ++p)
        {
          fPixel = static_cast<icl32f>(*p);
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
void Img<Type>::clear(int iIndex, Type tValue) 
  // {{{ open
{
  //---- Log Message ----
  FUNCTION_LOG("clear(" << iIndex << "," << tValue << ")");
  ICLASSERT_RETURN( iIndex < getChannels() );  
  
  for(int i=getStartIndex(iIndex),iEnd=getEndIndex(iIndex);i<iEnd;i++){
    clearChannelROI(this,i,tValue,Point::zero,getSize());
  }
}
  // }}}

// }}}

// {{{ Global functions


// utility function used internally for type save access to image
// pixels (nearest neighbour interpolation) 
template<class S,class D>
inline D _elemNN(S* ps, int x, int y, float fSX, float fSY, int w)
{
  return Cast<S,D>::cast(ps[(int)((round(fSX*x)) + round(fSY*y*w))]);
}

// utility function used internally for type save access to image
// pixels (linear interpolation) 
template<class S,class D>
inline D _elemLIN(S* ps, int x, int y, float fSX, float fSY, int w)
{
  int xll = (int)floor(fSX*x);
  int yll = (int)floor(fSY*y);

  float fT=fSX*x-xll;
  float fU=fSY*y-yll;

  float a = (float)ps[xll+xll*w];
  float b = (float)ps[xll+(xll+1)*w];
  float c = (float)ps[(xll+1)+(xll+1)*w];
  float d = (float)ps[(xll+1)+xll*w];
  
  return Cast<float,D>::cast((1-fT)*(1-fU)*a+ fT*(1-fU)*b + fT*fU*c + (1-fT)*fU*d);
}

// utility function used internally for type save access to image
// pixels (region average interpolation) 
template<class S, class D>
inline D _elemRA(S* ps, int x, int y, float fSX, float fSY, int w)
{
  ERROR_LOG("region average interpolation is not yet implemented!");
  return (D)0;
}

// scale channel ROI function for abitrary image scaling operations
template<class S,class D> 
void scaleChannelROI(Img<S> *src,int srcC, const Point &srcOffs, const Size &srcSize,
                     Img<D> *dst,int dstC, const Point &dstOffs, const Size &dstSize,
                     scalemode eScaleMode)
{
  FUNCTION_LOG("");
  ICLASSERT_RETURN( src && dst );
  
  int iSrcW = src->getSize().width;
  float fSX = ((float)srcSize.width-1)/(float)(dstSize.width); 
  float fSY = ((float)srcSize.height-1)/(float)(dstSize.height);

  ImgIterator<D> itDst(dst->getROIData(dstC),dst->getSize().width,Rect(dstOffs,dstSize));
  S* ps = src->getROIData(srcC,srcOffs);

  switch(eScaleMode) 
    {
      case interpolateNN:
        for(; itDst.inRegion();++itDst) *itDst = _elemNN<S,D>(ps,itDst.y(),itDst.y(),fSX,fSY,iSrcW);
        break;
      case interpolateLIN:
        for(; itDst.inRegion();++itDst) *itDst = _elemLIN<S,D>(ps,itDst.y(),itDst.y(),fSX,fSY,iSrcW);        
        break;
      case interpolateRA:
        for(; itDst.inRegion();++itDst) *itDst = _elemRA<S,D>(ps,itDst.y(),itDst.y(),fSX,fSY,iSrcW);        
        break;
      default:
        ERROR_LOG("unknown interpoation method!");
    }
}


#ifndef WITH_IPP_OPTIMIZATION
template void scaleChannelROI<icl8u,icl8u>(Img<icl8u>    *src,int srcC, const Point &srcOffs, const Size &srcSize,
                                           Img<icl8u>    *dst,int dstC, const Point &dstOffs, const Size &dstSize,
                                           scalemode eScaleMode);
template void scaleChannelROI<icl32f,icl32f>(Img<icl32f> *src,int srcC, const Point &srcOffs, const Size &srcSize,
                                             Img<icl32f> *dst,int dstC, const Point &dstOffs, const Size &dstSize,
                                             scalemode eScaleMode);
#endif

template void scaleChannelROI<icl8u,icl32f>(Img<icl8u>   *src,int srcC, const Point &srcOffs, const Size &srcSize,
                                            Img<icl32f>  *dst,int dstC, const Point &dstOffs, const Size &dstSize,
                                            scalemode eScaleMode);
template void scaleChannelROI<icl32f,icl8u>(Img<icl32f>  *src,int srcC, const Point &srcOffs, const Size &srcSize,
                                            Img<icl8u>   *dst,int dstC, const Point &dstOffs, const Size &dstSize,
                                            scalemode eScaleMode);

// }}}


template class Img<icl8u>;
template class Img<icl32f>;

} //namespace icl
