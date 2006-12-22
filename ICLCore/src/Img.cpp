/*
  Img.cpp

  Written by: Michael Götting (2004)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include <Img.h>

namespace icl {

// {{{  constructors and destructors

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const ImgParams &params):
  // {{{ open
  ImgBase(icl::getDepth<Type>(),params){
  FUNCTION_LOG("Img(params)");
  
  for(int i=0;i<getChannels();i++) {
    m_vecChannels.push_back(createChannel());
  }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Size &s,int iChannels):
  // {{{ open

  ImgBase(icl::getDepth<Type>(),ImgParams(s,iChannels)){
  FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," << iChannels << ")  this:" << this );
  
  for(int i=0;i<getChannels();i++) {
    m_vecChannels.push_back(createChannel());
  }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Size& s, format eFormat):
  // {{{ open
  ImgBase(icl::getDepth<Type>(),ImgParams(s,eFormat)){
  FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," << translateFormat(eFormat) << ")  this:" << this );
  
  for(int i=0;i<getChannels();i++) {
    m_vecChannels.push_back(createChannel());
  }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Size &s,int iChannels, format fmt):
  // {{{ open

  ImgBase(icl::getDepth<Type>(),ImgParams(s,iChannels,fmt)){
  FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," << 
               iChannels << "," << translateFormat(fmt) << ")  this:" << this );
  
  for(int i=0;i<getChannels();i++) {
    m_vecChannels.push_back(createChannel());
  }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Size &s, int channels, const std::vector<Type*>& vptData) :
  // {{{ open
  ImgBase(icl::getDepth<Type>(),ImgParams(s,channels)) {
  ICLASSERT_THROW (getChannels () <= (int) vptData.size(), InvalidImgParamException("channels"));
  FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," <<  channels << ",Type**)  this:" << this);
  
  typename std::vector<Type*>::const_iterator it = vptData.begin();
  for(int i=0; i<getChannels(); ++i, ++it) {
     m_vecChannels.push_back(SmartPtr<Type>(*it,false));
  }
}

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Size &s, int channels, format fmt, const std::vector<Type*>& vptData) :
  // {{{ open
  ImgBase(icl::getDepth<Type>(),ImgParams(s,channels,fmt)){
  ICLASSERT_THROW (getChannels () <= (int) vptData.size(), InvalidImgParamException("channels"));
  FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," <<  channels << 
               "," << translateFormat(fmt) << ",Type**)  this:" << this);
  
  typename std::vector<Type*>::const_iterator it = vptData.begin();
  for(int i=0; i<getChannels(); ++i, ++it) {
     m_vecChannels.push_back(SmartPtr<Type>(*it,false));
  }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Size &s, format eFormat, const std::vector<Type*>& vptData) :
  // {{{ open
  ImgBase(icl::getDepth<Type>(),ImgParams(s,eFormat)){
  ICLASSERT_THROW (getChannels () <= (int) vptData.size(), InvalidImgParamException("channels"));
  FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," << translateFormat(eFormat) << ",Type**)  this:" << this);
   
  typename std::vector<Type*>::const_iterator it = vptData.begin();
  for(int i=0; i<getChannels(); ++i, ++it) {
     m_vecChannels.push_back(SmartPtr<Type>(*it,false));
  }
} 

  // }}}

//----------------------------------------------------------------------------
template<class Type>
Img<Type>::Img(const Img<Type>& tSrc):
  // {{{ open

    ImgBase(tSrc.getDepth(),tSrc.getParams())
{
  FUNCTION_LOG("this: " << this);
  m_vecChannels = tSrc.m_vecChannels;
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
  m_oParams = tSrc.getParams ();
  m_vecChannels = tSrc.m_vecChannels;

  //take over timestamp
  this->setTime(tSrc.getTime());
  return *this;
}

// }}} 

// {{{  class organisation : 

//----------------------------------------------------------------------------
template<class Type> ImgBase*
Img<Type>::deepCopy(ImgBase* poDst) const
  // {{{ open

{
  FUNCTION_LOG("");
  
  if(!poDst) {
     poDst = imgNew(getDepth());
     poDst->setTime(this->getTime());
  }
  switch (poDst->getDepth()){
    case depth8u: return convertTo<icl8u>(poDst->asImg<icl8u>()); break;
    case depth16s: return convertTo<icl16s>(poDst->asImg<icl16s>()); break;
    case depth32s: return convertTo<icl32s>(poDst->asImg<icl32s>()); break;
    case depth32f: return convertTo<icl32f>(poDst->asImg<icl32f>()); break;
    case depth64f: return convertTo<icl64f>(poDst->asImg<icl64f>()); break;
    default: ICL_INVALID_DEPTH; break;
  }
}

  // }}}

//--------------------------------------------------------------------------
template<class Type> ImgBase*
Img<Type>::scaledCopy(ImgBase *poDst,scalemode eScaleMode) const
  // {{{ open

{
  FUNCTION_LOG("");
  
  if(!poDst || isEqual(poDst->getSize(),poDst->getChannels())){
    SECTION_LOG("deep copy case");
    return deepCopy(poDst); 
  }

  poDst->setFormat(getFormat());
  poDst->setTime(getTime());
  poDst->setChannels(getChannels());
  switch (poDst->getDepth()){
    case depth8u:
      for(int c=0;c<getChannels();c++){
        scaledCopyChannelROI<Type,icl8u>(this,c,Point::zero,getSize(),
                                         poDst->asImg<icl8u>(),c,Point::zero,poDst->getSize(),eScaleMode);
      } 
      break; 
    case depth16s:
      for(int c=0;c<getChannels();c++){
        scaledCopyChannelROI<Type,icl16s>(this,c,Point::zero,getSize(),
                                         poDst->asImg<icl16s>(),c,Point::zero,poDst->getSize(),eScaleMode);
      } 
      break;
    case depth32s:
      for(int c=0;c<getChannels();c++){
        scaledCopyChannelROI<Type,icl32s>(this,c,Point::zero,getSize(),
                                         poDst->asImg<icl32s>(),c,Point::zero,poDst->getSize(),eScaleMode);
      } 
      break;
    case depth32f:
      for(int c=0;c<getChannels();c++){
        scaledCopyChannelROI<Type,icl32f>(this,c,Point::zero,getSize(),
                                          poDst->asImg<icl32f>(),c,Point::zero,poDst->getSize(),eScaleMode);
      }
      break;
    case depth64f:
      for(int c=0;c<getChannels();c++){
        scaledCopyChannelROI<Type,icl64f>(this,c,Point::zero,getSize(),
                                          poDst->asImg<icl64f>(),c,Point::zero,poDst->getSize(),eScaleMode);
      } 
      break;
    default: ICL_INVALID_DEPTH; break;
  }
  
  float fScaleX = ((float)poDst->getSize().width)/(float)(getSize().width); 
  float fScaleY = ((float)poDst->getSize().height)/(float)(getSize().height);
  Rect roi = getROI();
  roi.x = (int)rint(fScaleX * roi.x);
  roi.y = (int)rint(fScaleY * roi.y); 
  roi.width  = (int)rint(fScaleX * roi.width);
  roi.height = (int)rint(fScaleY * roi.height);
  roi = roi & Rect (Point::zero, poDst->getSize());
  poDst->setROI (roi);
  poDst->getTime() = this->getTime();
  return poDst;
}

// }}}

//--------------------------------------------------------------------------
template<class Type> ImgBase*
Img<Type>::deepCopyROI(ImgBase *poDst) const
  // {{{ open

{
  FUNCTION_LOG("");

  if(!poDst){
    poDst = imgNew(getDepth(),
                   ImgParams(getROISize(),getChannels(),getFormat()));
  }else{
    poDst->setChannels(getChannels());
    poDst->setFormat(getFormat());
  }

  ICLASSERT_RETURN_VAL( getROISize() == poDst->getROISize() , poDst);
  switch (poDst->getDepth()){
    case depth8u:
      for(int c=0;c<getChannels();c++) {
        deepCopyChannelROI<Type,icl8u>(this,  c, getROIOffset(),       getROISize(),
                                       poDst->asImg<icl8u>(), c, poDst->getROIOffset(),poDst->getROISize());
      }
      break;
    case depth16s:
      for(int c=0;c<getChannels();c++) {
        deepCopyChannelROI<Type,icl16s>(this,  c, getROIOffset(),       getROISize(),
                                       poDst->asImg<icl16s>(), c, poDst->getROIOffset(),poDst->getROISize());
      }
      break;
    case depth32s:
      for(int c=0;c<getChannels();c++) {
        deepCopyChannelROI<Type,icl32s>(this,  c, getROIOffset(),       getROISize(),
                                       poDst->asImg<icl32s>(), c, poDst->getROIOffset(),poDst->getROISize());
      }
      break;
    case depth32f:
      for(int c=0;c<getChannels();c++) {
        deepCopyChannelROI<Type,icl32f>(this,  c, getROIOffset(),       getROISize(),
                                        poDst->asImg<icl32f>(), c, poDst->getROIOffset(),poDst->getROISize());
      }
      break;
    case depth64f:
      for(int c=0;c<getChannels();c++) {
        deepCopyChannelROI<Type,icl64f>(this,  c, getROIOffset(),       getROISize(),
                                       poDst->asImg<icl64f>(), c, poDst->getROIOffset(),poDst->getROISize());
      }

      break;
    default: ICL_INVALID_DEPTH; break;
  }
  return poDst;
}

  // }}}

//----------------------------------------------------------------------------
template<class Type> ImgBase*
Img<Type>::scaledCopyROI(ImgBase *poDst, scalemode eScaleMode) const
  // {{{ open

{
  FUNCTION_LOG("");
  
  if(!poDst || getROISize() == poDst->getROISize()){
    return deepCopyROI(poDst); 
  }

  poDst->setFormat(getFormat());
  poDst->setChannels(getChannels());
  switch (poDst->getDepth()){
    case depth8u:
      for(int c=0;c<getChannels();c++){
        scaledCopyChannelROI<Type,icl8u>(this,c,getROIOffset(),getROISize(),
                                         poDst->asImg<icl8u>(),c,poDst->getROIOffset(), poDst->getROISize(),
                                         eScaleMode);
      }
      break;
    case depth16s:
      for(int c=0;c<getChannels();c++){
        scaledCopyChannelROI<Type,icl16s>(this,c,getROIOffset(),getROISize(),
                                         poDst->asImg<icl16s>(),c,poDst->getROIOffset(), poDst->getROISize(),
                                         eScaleMode);
      }
      break;
    case depth32s:
      for(int c=0;c<getChannels();c++){
        scaledCopyChannelROI<Type,icl32s>(this,c,getROIOffset(),getROISize(),
                                         poDst->asImg<icl32s>(),c,poDst->getROIOffset(), poDst->getROISize(),
                                         eScaleMode);
      }
      break;
    case depth32f:
      for(int c=0;c<getChannels();c++){
        scaledCopyChannelROI<Type,icl32f>(this,c,getROIOffset(),getROISize(),
                                          poDst->asImg<icl32f>(),c,poDst->getROIOffset(), poDst->getROISize(),
                                          eScaleMode);
      }
      break;
    case depth64f:
      for(int c=0;c<getChannels();c++){
        scaledCopyChannelROI<Type,icl64f>(this,c,getROIOffset(),getROISize(),
                                         poDst->asImg<icl64f>(),c,poDst->getROIOffset(), poDst->getROISize(),
                                         eScaleMode);
      }
      break;
    default: ICL_INVALID_DEPTH; break;
  }
  return poDst;
}

// }}}

template<class Type> ImgBase*
Img<Type>::flippedCopyROI(ImgBase *poDst, axis eAxis) const
  // {{{ open

{
  FUNCTION_LOG("");
  if(!poDst){
    poDst = imgNew(getDepth(),
                   ImgParams(getROISize(),getFormat(),getChannels()));
  } else {
    ICLASSERT_RETURN_VAL( poDst->getROISize() == getROISize() ,poDst);  
    poDst->setChannels(getChannels());
  }

  if (poDst->getDepth() == this->getDepth()) {
     for(int c=0;c<getChannels();c++) {
        flippedCopyChannelROI (eAxis, this, c, getROIOffset(), getROISize(),
                               poDst->asImg<Type>(), c, poDst->getROIOffset(),poDst->getROISize());
     }
  } else {
    
    switch (poDst->getDepth()){
      case depth8u:{
        Img<icl8u> *pD = poDst->asImg<icl8u>();
        for(int c=0;c<getChannels();c++) {
           deepCopyChannelROI (this, c, getROIOffset(), getROISize(),
                               pD, c, poDst->getROIOffset(),poDst->getROISize());
           pD->mirror (eAxis, c, poDst->getROIOffset(),poDst->getROISize());
        }
        break;
      }
        break;
      case depth16s:{
        Img<icl16s> *pD = poDst->asImg<icl16s>();
        for(int c=0;c<getChannels();c++) {
          deepCopyChannelROI (this, c, getROIOffset(), getROISize(),
                               pD, c, poDst->getROIOffset(),poDst->getROISize());
          pD->mirror (eAxis, c, poDst->getROIOffset(),poDst->getROISize());
        }
        break;
      }
      case depth32s:{
        Img<icl32s> *pD = poDst->asImg<icl32s>();
        for(int c=0;c<getChannels();c++) {
          deepCopyChannelROI (this, c, getROIOffset(), getROISize(),
                               pD, c, poDst->getROIOffset(),poDst->getROISize());
          pD->mirror (eAxis, c, poDst->getROIOffset(),poDst->getROISize());
        }
        break;
      }
      case depth32f:{
        Img<icl32f> *pD = poDst->asImg<icl32f>();
        for(int c=0;c<getChannels();c++) {
          deepCopyChannelROI (this, c, getROIOffset(), getROISize(),
                               pD, c, poDst->getROIOffset(),poDst->getROISize());
          pD->mirror (eAxis, c, poDst->getROIOffset(),poDst->getROISize());
        }
        break;
      }
      case depth64f:{
        Img<icl64f> *pD = poDst->asImg<icl64f>();
        for(int c=0;c<getChannels();c++) {
          deepCopyChannelROI (this, c, getROIOffset(), getROISize(),
                               pD, c, poDst->getROIOffset(),poDst->getROISize());
          pD->mirror (eAxis, c, poDst->getROIOffset(),poDst->getROISize());
        }
        break;
      }
      default: ICL_INVALID_DEPTH; break;
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
  ICLASSERT_RETURN(iChannel >=0 && iChannel < getChannels());

  m_vecChannels.erase(m_vecChannels.begin()+iChannel);
  m_oParams.setChannels(m_vecChannels.size());
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
Img<Type>::append(Img<Type> *poSrc, int iIndex)
  // {{{ open

{
  FUNCTION_LOG("");
  ICLASSERT_RETURN( poSrc );
  // iIndex < 0 is ok and means all channels
  ICLASSERT_RETURN( iIndex < poSrc->getChannels() ); 
  ICLASSERT_RETURN( poSrc->getSize() == getSize() );

  std::copy (poSrc->m_vecChannels.begin() + poSrc->getStartIndex(iIndex),
             poSrc->m_vecChannels.begin() + poSrc->getEndIndex(iIndex),
             back_inserter(m_vecChannels));  
  m_oParams.setChannels(m_vecChannels.size());
}

// }}}

template<class Type> void
Img<Type>::append(Img<Type> *poSrc, const std::vector<int>& vChannels)
  // {{{ open

{
  FUNCTION_LOG("");
  ICLASSERT_RETURN( poSrc );
  ICLASSERT_RETURN( poSrc->getSize() == getSize() );

  const int iMaxChannels = poSrc->getChannels();
  for (std::vector<int>::const_iterator it=vChannels.begin(), end=vChannels.end();
       it != end; ++it) {
     if (*it < 0 || *it >= iMaxChannels) {
        ERROR_LOG ("channel index out of range: " << *it);
     } else {
        m_vecChannels.push_back (poSrc->m_vecChannels[*it]);
     }
  }
  m_oParams.setChannels(m_vecChannels.size());
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
Img<Type>::scale(const Size &size, scalemode eScaleMode)
  // {{{ open

{  
  FUNCTION_LOG("");
  ICLASSERT_RETURN (size.width > 0 && size.height > 0);

  if (!isEqual(size,getChannels()))
    {
      Img<Type> oTmp(size,getChannels(),getFormat());
      scaledCopy(&oTmp,eScaleMode);
      (*this)=oTmp;
    }
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
Img<Type>::mirror(axis eAxis, bool bOnlyROI)
  // {{{ open
{
  FUNCTION_LOG("");
   const Point& oOffset = bOnlyROI ? getROIOffset() : Point::zero;
   const Size&  oSize   = bOnlyROI ? getROISize() : getSize();
   for (int c=0; c < getChannels(); ++c) {
      this->mirror (eAxis, c, oOffset, oSize);
   }
}
// }}}

static inline void* getPointerOffset (const void* begin, int x, int y, int iByteSize, int iLineLen) {
  // {{{ open
  FUNCTION_LOG("");
  return ((char*)begin) + iByteSize * (x + y*iLineLen);
}

  // }}}
static bool getMirrorPointers (axis eAxis, bool bInplace, 
                               const void* srcBegin, const void* dstBegin, int iByteSize,
                               const Point& oSrcOffset, int iSrcLineStep, 
                               const Point& oDstOffset, int iDstLineStep, const Size& oSize,
                               void** pS, void** pD, void** pE, void** pELine,
                               int& iLineWarpS, int& iLineWarpD) {
  // {{{ open
  FUNCTION_LOG("");
   void *&s=*pS, *&d=*pD, *&e=*pE, *&eLine=*pELine;
   int iRows=0, iCols=0;
   int iSrcLineLen = iSrcLineStep / iByteSize;
   int iDstLineLen = iDstLineStep / iByteSize;
   switch (eAxis) {
      case axisHorz:
         /* .....................
            ....s->++++++++++l...
            ....+++++++++++++....
            ....e------------....
            ....*************....
            ....d->**********....
            .....................
         */

         iRows = bInplace ? oSize.height/2 : oSize.height; 
         iCols = oSize.width;
         iLineWarpS = iSrcLineLen - iCols;
         iLineWarpD = -(iDstLineLen+iCols);
         s = getPointerOffset (srcBegin, oSrcOffset.x, oSrcOffset.y, iByteSize, iSrcLineLen);
         d = getPointerOffset (dstBegin, oDstOffset.x, oDstOffset.y + oSize.height - 1, 
                               iByteSize, iDstLineLen);
         e = ((char*)s) + iRows * iSrcLineStep;
         break;
      case axisVert:
         /* .....................
            ....s->++++|l*<-d....
            ....+++++++|*****....
            ....+++++++|*****....
            ....+++++++|*****....
            ....+++++++|*****....
            ....e................
         */
         iRows = oSize.height;
         iCols = bInplace ? oSize.width/2 : oSize.width; 
         iLineWarpS = iSrcLineLen - iCols;
         iLineWarpD = iDstLineLen + iCols;
         s = getPointerOffset (srcBegin, oSrcOffset.x, oSrcOffset.y, iByteSize, iSrcLineLen);
         d = getPointerOffset (dstBegin, oDstOffset.x + oSize.width - 1, oDstOffset.y, 
                               iByteSize, iDstLineLen);
         e = ((char*)s) + iRows * iSrcLineStep;
         break;
      case axisBoth: 
         /* .....................
            ....s->++++++++++l...
            ....+++++++++++++....
            ....+++++++e*****....
            ....*************....
            ....**********<-d....
            .....................
         */

         iRows = bInplace ? oSize.height/2 : oSize.height; 
         iCols = oSize.width;
         iLineWarpS = iSrcLineLen - iCols;
         iLineWarpD = iCols - iDstLineLen;
         s = getPointerOffset (srcBegin, oSrcOffset.x, oSrcOffset.y, iByteSize, iSrcLineLen);
         d = getPointerOffset (dstBegin, 
                               oDstOffset.x + oSize.width - 1, 
                               oDstOffset.y + oSize.height - 1, 
                               iByteSize, iDstLineLen);
         e = ((char*)s) + iRows * iSrcLineStep;

         if (bInplace && oSize.height % 2) { // odd ROI height
            iRows++;
            e = (char*)e + iByteSize * (oSize.width/2);
         }
         break;
   }

   eLine = ((char*)s) + iCols * iByteSize;

   return (iRows != 0 && iCols != 0);
}

  // }}}

template<class Type> void
Img<Type>::mirror(axis eAxis, int iChannel, 
                  const Point& oOffset, const Size& oSize)
  // {{{ open
{  
  FUNCTION_LOG("");
  
  static const int aiDstStep[] = {1,-1,-1};
  int      iLineWarpS, iLineWarpD;
  register Type *s=0, *d=0, *e=0, *eLine=0; /* source pointer, destination pointer, 
                                             end pointer, line end pointer */
  if (!getMirrorPointers (eAxis, true, getData(iChannel), getData(iChannel), sizeof(Type), 
                          oOffset, getLineStep(), oOffset, getLineStep(), oSize,
                          (void**) &s, (void**) &d, (void**) &e, (void**) &eLine, 
                          iLineWarpS, iLineWarpD)) return;
  do {
     std::swap (*s, *d);
     ++s; d += aiDstStep[eAxis];
     if (s == eLine) {
        eLine += getWidth(); // end of line pointer jumps whole image width
        s += iLineWarpS;     // source pointer jumps iLineWarpS
        d += iLineWarpD;
     }
  } while (s != e);
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
Img<Type>::setSize(const Size &s)
  // {{{ open

{
  FUNCTION_LOG("");
  
  Size oNewSize(s.width<0?getSize().width:s.width, 
                s.height<0?getSize().height:s.height);

  //---- estimate destination values in respect to defaults ----
  if (oNewSize != getSize()) {
    m_oParams.setSize(oNewSize);
    for(int i=0;i<getChannels();i++) {
      m_vecChannels[i] = createChannel ();
    }
  }
}

// }}}

//----------------------------------------------------------------------------
template<class Type> void
Img<Type>::setChannels(int iNumNewChannels)
  // {{{ open
{
  FUNCTION_LOG("");
  ICLASSERT_RETURN(iNumNewChannels >= 0);
  if (iNumNewChannels == getChannels()) return;
  
  if(iNumNewChannels < getChannels()) {
    //---- reduce number of channels ----
    m_vecChannels.erase(m_vecChannels.begin() + iNumNewChannels, 
                        m_vecChannels.end());
  }else{
     //---- Extend number of channels ----
     m_vecChannels.reserve (iNumNewChannels);
     for(int i=getChannels();i < iNumNewChannels; ++i)
        m_vecChannels.push_back(createChannel());
  }
  m_oParams.setChannels(m_vecChannels.size());
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

// {{{  Get Min/Max functions: 

// {{{     getMax
template<class Type> Type 
Img<Type>::getMax() const
{
  FUNCTION_LOG("");

  if (getChannels() == 0) return 0;
  Type tMax = getMax(0);
  for(int i=1;i<getChannels();i++)
     tMax = std::max(tMax,getMax(i));
  return tMax;    
}

  //fallback for 64f and 32s
template<class Type> Type 
Img<Type>::getMax(int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN_VAL(0 <= iChannel && iChannel < getChannels(),0);
   return *std::max_element (getData(iChannel), getData(iChannel) + getDim());
}

template<> icl8u
Img<icl8u>::getMax(int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN_VAL(0 <= iChannel && iChannel < getChannels(),0);
   icl8u vMax;
   ippiMax_8u_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMax);
   return vMax;
}
template<> icl32f
Img<icl32f>::getMax(int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN_VAL(0 <= iChannel && iChannel < getChannels(),0);
   icl32f vMax;
   ippiMax_32f_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMax);
   return vMax;
}
template<> icl16s
Img<icl16s>::getMax(int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN_VAL(0 <= iChannel && iChannel < getChannels(),0);
   icl16s vMax;
   ippiMax_16s_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMax);
   return vMax;
}

// }}}

// {{{     getMin
template<class Type> Type 
Img<Type>::getMin() const
{
  FUNCTION_LOG("");

  if (getChannels() == 0) return 0;
  Type tMin = getMin(0);
  for(int i=1;i<getChannels();i++)
     tMin = std::min(tMin,getMin(i));
  return tMin;    
}

template<class Type> Type 
Img<Type>::getMin(int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN_VAL(0 <= iChannel && iChannel < getChannels(),0);
   return *std::min_element (getData(iChannel), getData(iChannel) + getDim());
}
template<> icl8u
Img<icl8u>::getMin(int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN_VAL(0 <= iChannel && iChannel < getChannels(),0);
   icl8u vMin;
   ippiMin_8u_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMin);
   return vMin;
}
template<> icl32f
Img<icl32f>::getMin(int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN_VAL(0 <= iChannel && iChannel < getChannels(),0);
   icl32f vMin;
   ippiMin_32f_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMin);
   return vMin;
}
template<> icl16s
Img<icl16s>::getMin(int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN_VAL(0 <= iChannel && iChannel < getChannels(),0);
   icl16s vMin;
   ippiMin_16s_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMin);
   return vMin;
}
// }}}
  
// {{{     getMinMax
template<class Type> void
Img<Type>::getMinMax(Type &rtMin, Type &rtMax) const
{
  FUNCTION_LOG("");

  if (getChannels() == 0) {
     rtMin=rtMax=0;
     return;
  }

  Type tMin, tMax; 
  getMinMax(rtMin, rtMax, 0);
  for(int i=1;i<getChannels();i++) {
     getMinMax(tMin, tMax, i);
     rtMin = std::min(rtMin,tMin);
     rtMax = std::max(rtMax,tMax);
  }
}

// fallback for 32s and 64f
template<class Type> void
Img<Type>::getMinMax(Type &rtMin, Type &rtMax, int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN (0 <= iChannel && iChannel < getChannels());

   Type *ptData = getData(iChannel);
   Type *ptDataEnd = ptData+getDim();
   if (ptData == ptDataEnd){
      rtMin = rtMax = 0;
      return;
   }
   rtMin = rtMax = *ptData; ++ptData;
   for (++ptData; ptData != ptDataEnd; ++ptData) {
      rtMin = std::min(rtMin,*ptData);
      rtMax = std::max(rtMin,*ptData);
   }
}

template<> void
Img<icl8u>::getMinMax(icl8u &vMin, icl8u &vMax, int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN(0 <= iChannel && iChannel < getChannels());
   ippiMinMax_8u_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMin,&vMax);
}
template<> void
Img<icl32f>::getMinMax(icl32f &vMin, icl32f &vMax, int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN(0 <= iChannel && iChannel < getChannels());
   ippiMinMax_32f_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMin,&vMax);
}
template<> void
Img<icl16s>::getMinMax(icl16s &vMin, icl16s &vMax, int iChannel) const {
   FUNCTION_LOG("iChannel: " << iChannel);
   ICLASSERT_RETURN(0 <= iChannel && iChannel < getChannels());
   ippiMinMax_16s_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMin,&vMax);
}
// }}}

// }}}

// {{{  Auxillary  functions 

template<class Type>
SmartPtr<Type> Img<Type>::createChannel(Type *ptDataToCopy) const
  // {{{ open
{
  FUNCTION_LOG("");
  int dim = getDim();
  if(!dim){
    return SmartPtr<Type>();
  }
  Type *ptNewData =  new Type[dim];
  if(ptDataToCopy){
    memcpy(ptNewData,ptDataToCopy,getDim()*sizeof(Type));
  }else{
    std::fill(ptNewData,ptNewData+dim,0);
  }
  return SmartPtr<Type>(ptNewData);
}

  // }}}


// sub-pixel access using linear interpolation
template<class Type>
float Img<Type>::subPixelLIN(float fX, float fY, int iChannel) const {
  // {{{ open

   float fX0 = fX - floor(fX), fX1 = 1.0 - fX0;
   float fY0 = fY - floor(fY), fY1 = 1.0 - fY0;
   int xll = (int) fX;
   int yll = (int) fY;
   
   Type* pLL = getData(iChannel) + xll + yll * getWidth();
   float a = *pLL;        //  a b
   float b = *(++pLL);    //  c d
   pLL += getWidth();
   float d = *pLL;
   float c = *(--pLL);
   
// return fX1*fY1*a + fX0*fY1*b + fX0*fY0*d + fX1*fY0*c;
   return fX1 * (fY1*a + fY0*c) + fX0 * (fY1*b + fY0*d);
}

  // }}}
  
// sub-pixel access using region average interpolation
template<class Type>
float Img<Type>::subPixelRA(float fX, float fY, int iChannel) const {
  // {{{ open

   ERROR_LOG ("region average interpolation is not yet implemented!");
   return subPixelLIN (fX, fY, iChannel);
}

  // }}}

template<class Type>
Type Img<Type>::operator()(float fX, float fY, int iChannel, scalemode eScaleMode) const {
  // {{{ open

   switch(eScaleMode) {
     case 0: return Cast<float, Type>::cast (subPixelNN (fX, fY, iChannel));
     case 1: return Cast<float, Type>::cast (subPixelLIN (fX, fY, iChannel));
     default: 
        ERROR_LOG ("interpolation method not yet implemented!");
        return Cast<float, Type>::cast (subPixelLIN (fX, fY, iChannel));
   }
}

  // }}}

// }}}

// {{{  Basic image manipulation functions

// {{{   scaleRange wrappers

template<class Type> void
Img<Type>::scaleRange(float fNewMin, float fNewMax) {
   Type tMin, tMax;
   getMinMax(tMin,tMax);
   scaleRange(fNewMin, fNewMax, tMin,tMax);
}
template<class Type> void
Img<Type>::scaleRange(float fNewMin,float fNewMax, float fMin,float fMax) {
   for (int c=0; c < getChannels(); ++c)
      scaleRange(fNewMin,fNewMax, fMin,fMax, c);
}

template<class Type> void
Img<Type>::scaleRange(float fNewMin, float fNewMax, int iChannel) {
   FUNCTION_LOG("");
   ICLASSERT_RETURN(iChannel >= 0 && iChannel < getChannels());

   Type tMin, tMax;
   getMinMax(tMin,tMax,iChannel);
   scaleRange(fNewMin,fNewMax, tMin,tMax, iChannel);
}

// }}}

// {{{   scaleRange main methods

// fallback for all up to icl8u and icl32f
template <class Type> void 
Img<Type>::scaleRange(float fNewMin, float fNewMax,
                      float fMin, float fMax, int iChannel) {
   float fScale  = (fNewMax - fNewMin) / (fMax - fMin);
   float fShift  = (fMax * fNewMin - fMin * fNewMax) / (fMax - fMin);
   float fPixel;
   for(iterator p=getROIIterator(iChannel); p.inRegion(); ++p) {
      fPixel = fShift + (float)(*p) * fScale;
      if (fPixel <= fNewMin) fPixel=fNewMin;
      else if(fPixel >= fNewMax) fPixel=fNewMax;
      
      *p = Cast<float, Type>::cast (fPixel);
   }
}

template<> void 
Img<icl8u>::scaleRange(float fNewMin, float fNewMax,
                       float fMin, float fMax, int iChannel) {
   icl8u tFac   = Cast<float, icl8u>::cast(fNewMax - fNewMin);
   icl8u tNorm  = Cast<float, icl8u>::cast(fMax - fMin);
   icl8u tShift = Cast<float, icl8u>::cast((fMax * fNewMin - fMin * fNewMax) / 
                                           (fNewMax - fNewMin));

   ippiMulC_8u_C1IRSfs (tFac, getROIData(iChannel), getLineStep(), 
                        getROISize(), tNorm);
   if (tShift != 0)
      ippiAddC_8u_C1IRSfs (tShift, getROIData(iChannel), getLineStep(), 
                           getROISize(), 1);
}
template <> void 
Img<icl32f>::scaleRange(float fNewMin, float fNewMax,
                        float fMin, float fMax, int iChannel) {
  icl32f tFac   = (fNewMax - fNewMin) / (fMax - fMin);
  icl32f tShift = (fMax * fNewMin - fMin * fNewMax) / (fMax - fMin);

  ippiMulC_32f_C1IR (tFac, getROIData(iChannel), getLineStep(), 
                     getROISize());
  
  if (tShift != 0) {
    ippiAddC_32f_C1IR (tShift, getROIData(iChannel), getLineStep(), 
                       getROISize());
  }
}

// }}}

// ---------------------------------------------------------------------
template<class Type>
void Img<Type>::clear(int iIndex, Type tValue, bool bROIOnly) 
  // {{{ open
{
  //---- Log Message ----
  FUNCTION_LOG("clear(" << iIndex << "," << tValue << ")");
  ICLASSERT_RETURN( iIndex < getChannels() );  
  
  Point offs = bROIOnly ? getROIOffset() : Point::zero;
  Size size = bROIOnly ? getROISize() : getSize();
  for(int i=getStartIndex(iIndex),iEnd=getEndIndex(iIndex);i<iEnd;i++){
    clearChannelROI(this,i,tValue,offs,size);
  }
}
  // }}}

// }}}

// {{{  Global functions

// {{{    scaledCopyChannelROI

// scale channel ROI function for abitrary image scaling operations
template<class S,class D> 
void scaledCopyChannelROI(const Img<S> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                          Img<D> *dst, int dstC, const Point &dstOffs, const Size &dstSize,
                          scalemode eScaleMode)
{
  FUNCTION_LOG("");
  ICLASSERT_RETURN( src && dst );
  float fSX = ((float)srcSize.width)/(float)(dstSize.width); 
  float fSY = ((float)srcSize.height)/(float)(dstSize.height);

  float (Img<S>::*subPixelMethod)(float fX, float fY, int iChannel) const;
  switch(eScaleMode) {
     case interpolateNN:
        subPixelMethod = &Img<S>::subPixelNN;
        break;
     case interpolateLIN:
        fSX = ((float)srcSize.width-1)/(float)(dstSize.width); 
        fSY = ((float)srcSize.height-1)/(float)(dstSize.height);
        subPixelMethod = &Img<S>::subPixelLIN;
        break;
     default:
        ERROR_LOG("unknown interpoation method!");
        subPixelMethod = &Img<S>::subPixelNN;
        break;
  }

  ImgIterator<D> itDst(dst->getData(dstC),dst->getSize().width,Rect(dstOffs,dstSize));

  int xD = 0;
  int yD = 0;
  float yS = srcOffs.y + fSY * yD;
  for(; itDst.inRegion(); ++itDst) {
    *itDst = Cast<float, D>::cast ((src->*subPixelMethod)(srcOffs.x + fSX * xD, yS, srcC));
    if (++xD == dstSize.width) {
      yS = srcOffs.y + fSY * ++yD;
      xD = 0;
    }
  }
}

// explicit template instantiation
#ifndef WITH_IPP_OPTIMIZATION  
template void scaledCopyChannelROI<icl8u,icl8u>(const Img<icl8u>*,int,const Point&,const Size&,Img<icl8u>*,int,const Point&,const Size&,scalemode);
#endif
template void scaledCopyChannelROI<icl8u,icl16s>(const Img<icl8u>*,int,const Point&,const Size&,Img<icl16s>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl8u,icl32s>(const Img<icl8u>*,int,const Point&,const Size&,Img<icl32s>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl8u,icl32f>(const Img<icl8u>*,int,const Point&,const Size&,Img<icl32f>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl8u,icl64f>(const Img<icl8u>*,int,const Point&,const Size&,Img<icl64f>*,int,const Point&,const Size&,scalemode);

template void scaledCopyChannelROI<icl16s,icl8u>(const Img<icl16s>*,int,const Point&,const Size&,Img<icl8u>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl16s,icl16s>(const Img<icl16s>*,int,const Point&,const Size&,Img<icl16s>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl16s,icl32s>(const Img<icl16s>*,int,const Point&,const Size&,Img<icl32s>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl16s,icl32f>(const Img<icl16s>*,int,const Point&,const Size&,Img<icl32f>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl16s,icl64f>(const Img<icl16s>*,int,const Point&,const Size&,Img<icl64f>*,int,const Point&,const Size&,scalemode);

template void scaledCopyChannelROI<icl32s,icl8u>(const Img<icl32s>*,int,const Point&,const Size&,Img<icl8u>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl32s,icl16s>(const Img<icl32s>*,int,const Point&,const Size&,Img<icl16s>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl32s,icl32s>(const Img<icl32s>*,int,const Point&,const Size&,Img<icl32s>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl32s,icl32f>(const Img<icl32s>*,int,const Point&,const Size&,Img<icl32f>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl32s,icl64f>(const Img<icl32s>*,int,const Point&,const Size&,Img<icl64f>*,int,const Point&,const Size&,scalemode);

template void scaledCopyChannelROI<icl32f,icl8u>(const Img<icl32f>*,int,const Point&,const Size&,Img<icl8u>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl32f,icl16s>(const Img<icl32f>*,int,const Point&,const Size&,Img<icl16s>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl32f,icl32s>(const Img<icl32f>*,int,const Point&,const Size&,Img<icl32s>*,int,const Point&,const Size&,scalemode);
#ifndef WITH_IPP_OPTIMIZATION  
template void scaledCopyChannelROI<icl32f,icl32f>(const Img<icl32f>*,int,const Point&,const Size&,Img<icl32f>*,int,const Point&,const Size&,scalemode);
#endif
template void scaledCopyChannelROI<icl32f,icl64f>(const Img<icl32f>*,int,const Point&,const Size&,Img<icl64f>*,int,const Point&,const Size&,scalemode);

template void scaledCopyChannelROI<icl64f,icl8u>(const Img<icl64f>*,int,const Point&,const Size&,Img<icl8u>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl64f,icl16s>(const Img<icl64f>*,int,const Point&,const Size&,Img<icl16s>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl64f,icl32s>(const Img<icl64f>*,int,const Point&,const Size&,Img<icl32s>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl64f,icl32f>(const Img<icl64f>*,int,const Point&,const Size&,Img<icl32f>*,int,const Point&,const Size&,scalemode);
template void scaledCopyChannelROI<icl64f,icl64f>(const Img<icl64f>*,int,const Point&,const Size&,Img<icl64f>*,int,const Point&,const Size&,scalemode);

// }}}

// {{{    flippedCopyChannelROI


// mirror copy ROI of one image to the ROI of the other (for selected channel)
template<class T>
void flippedCopyChannelROI(axis eAxis,
                           const Img<T> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                           Img<T> *dst, int dstC, const Point &dstOffs, const Size &dstSize)
{  
  FUNCTION_LOG("");
  ICLASSERT_RETURN( src && dst );
  ICLASSERT_RETURN( srcSize == dstSize );
  
  static const int aiDstStep[] = {1,-1,-1};
  int      iLineWarpS, iLineWarpD;
  register T *s=0, *d=0, *e=0, *eLine=0; /* source pointer, destination pointer, 
                                            end pointer, line end pointer */
  if (!getMirrorPointers (eAxis, false, src->getData(srcC), dst->getData(dstC), sizeof(T), 
                          srcOffs, src->getLineStep(), dstOffs, dst->getLineStep(), srcSize,
                          (void**)&s, (void**)&d, (void**) &e, (void**) &eLine, 
                          iLineWarpS, iLineWarpD)) return;

  if (eAxis == axisHorz) {
     int iSrcStep = src->getSize().width, iDstStep = dst->getSize().width;
     int nBytes = sizeof(T) * srcSize.width;
     // line-wise memcpy is possible
     for (; s != e; s += iSrcStep, d -= iDstStep)
        memcpy (d, s, nBytes);
     return;
  }

  do {
     *d = *s;
     ++s; d += aiDstStep[eAxis];
     if (s == eLine) {
        eLine += src->getSize().width; // end of line pointer jumps whole image width
        s += iLineWarpS;               // source pointer jumps iLineWarpS
        d += iLineWarpD;
     }
  } while (s != e);
}


// falls ndef ipp alle
// sonst alle bis auf icl8u und icl32f

#ifndef WITH_IPP_OPTIMIZATION
// explicit template instantiation  ** only for this two are optimized using ipp
template void 
flippedCopyChannelROI<icl8u>(axis eAxis, 
                             const Img<icl8u> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                             Img<icl8u> *dst, int dstC, const Point &dstOffs, const Size &dstSize);
template void 
flippedCopyChannelROI<icl32f>(axis eAxis,
                              const Img<icl32f> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                              Img<icl32f> *dst, int dstC, const Point &dstOffs, const Size &dstSize);
#endif

template void 
flippedCopyChannelROI<icl16s>(axis eAxis,
                              const Img<icl16s> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                              Img<icl16s> *dst, int dstC, const Point &dstOffs, const Size &dstSize);
template void 
flippedCopyChannelROI<icl32s>(axis eAxis,
                              const Img<icl32s> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                              Img<icl32s> *dst, int dstC, const Point &dstOffs, const Size &dstSize);
template void 
flippedCopyChannelROI<icl64f>(axis eAxis,
                              const Img<icl64f> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                              Img<icl64f> *dst, int dstC, const Point &dstOffs, const Size &dstSize);



// }}}

// }}}


template class Img<icl8u>;
template class Img<icl32f>;
template class Img<icl16s>;
template class Img<icl32s>;
template class Img<icl64f>;

} //namespace icl
