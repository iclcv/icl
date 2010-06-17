/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/Img.cpp                                    **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Michael Götting, Robert Haschke   **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLCore/Img.h>
#include <functional>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/StringUtils.h>

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
  Img<Type>::Img(const Size &s, int channels, const std::vector<Type*>& vptData, bool passOwnerShip) :
    // {{{ open

    ImgBase(icl::getDepth<Type>(),ImgParams(s,channels)) {
    ICLASSERT_THROW (getChannels () <= (int) vptData.size(), InvalidImgParamException("channels"));
    FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," <<  channels << ",Type**)  this:" << this);
  
    typename std::vector<Type*>::const_iterator it = vptData.begin();
    for(int i=0; i<getChannels(); ++i, ++it) {
      m_vecChannels.push_back(SmartArray<Type>(*it,passOwnerShip));
    }
  }

  // }}}

  //----------------------------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const Size &s, int channels, format fmt, const std::vector<Type*>& vptData, bool passOwnerShip) :
    // {{{ open
    ImgBase(icl::getDepth<Type>(),ImgParams(s,channels,fmt)){
    ICLASSERT_THROW (getChannels () <= (int) vptData.size(), InvalidImgParamException("channels"));
    FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," <<  channels << 
                 "," << translateFormat(fmt) << ",Type**)  this:" << this);
  
    typename std::vector<Type*>::const_iterator it = vptData.begin();
    for(int i=0; i<getChannels(); ++i, ++it) {
      m_vecChannels.push_back(SmartArray<Type>(*it,passOwnerShip));
    }
  } 

  // }}}

  //----------------------------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const Size &s, format eFormat, const std::vector<Type*>& vptData, bool passOwnerShip) :
    // {{{ open
    ImgBase(icl::getDepth<Type>(),ImgParams(s,eFormat)){
    ICLASSERT_THROW (getChannels () <= (int) vptData.size(), InvalidImgParamException("channels"));
    FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," << translateFormat(eFormat) << ",Type**)  this:" << this);
   
    typename std::vector<Type*>::const_iterator it = vptData.begin();
    for(int i=0; i<getChannels(); ++i, ++it) {
      m_vecChannels.push_back(SmartArray<Type>(*it,passOwnerShip));
    }
  } 

  // }}}

  //--- Copy constructor -------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const Img<Type>& tSrc) :
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

  template<class Type>
  static int get_channel_count(const DynMatrix<Type> &c1, 
                               const DynMatrix<Type> &c2, 
                               const DynMatrix<Type> &c3, 
                               const DynMatrix<Type> &c4, 
                               const DynMatrix<Type> &c5){
    if(c1.isNull()) return 0;
    else return 1 + (c2.isNull()?0:1) + (c3.isNull()?0:1) + (c4.isNull()?0:1) + (c5.isNull()?0:1); 
  }
  
  template<class Type>
  Img<Type>::Img(const DynMatrix<Type> &c1, 
                 const DynMatrix<Type> &c2, 
                 const DynMatrix<Type> &c3, 
                 const DynMatrix<Type> &c4, 
                 const DynMatrix<Type> &c5) throw (InvalidMatrixDimensionException):
    ImgBase(icl::getDepth<Type>(),ImgParams(Size(c1.cols(),c1.rows()),get_channel_count(c1,c2,c3,c4,c5))){
    
    if(c1.isNull()) return;
    m_vecChannels.reserve(getChannels());
    m_vecChannels.push_back(SmartArray<Type>(const_cast<Type*>(c1.begin()),false));
#define ADD_CHANNEL(i)                                                  \
    if(!c##i.isNull()){                                                 \
      ICLASSERT_THROW(c1.cols() == c##i.cols() && c1.rows() == c##i.rows(), InvalidMatrixDimensionException(__FUNCTION__)); \
      m_vecChannels.push_back(SmartArray<Type>(const_cast<Type*>(c##i.begin()),false)); \
    }
    ADD_CHANNEL(2)    ADD_CHANNEL(3)    ADD_CHANNEL(4)    ADD_CHANNEL(5)
#undef ADD_CHANNEL
    
    
  }
  

  // }}} constructors...
  

  // {{{  operators: "=", ()-(float,float,channel,scalemode)

  template<class Type>
  Img<Type>& Img<Type>::shallowCopy(const Img<Type>& tSrc)
  {
    // {{{ open

    FUNCTION_LOG("");
  
    //---- Assign new channels to Img ----
    m_oParams = tSrc.getParams ();
    m_vecChannels = tSrc.m_vecChannels;

    //take over timestamp
    this->setTime(tSrc.getTime());
    return *this;
  }

  // }}}

  template<class Type>
  Type Img<Type>::operator()(float fX, float fY, int iChannel, scalemode eScaleMode) const {
    // {{{ open

    switch(eScaleMode) {
      case 0: return clipped_cast<float, Type>(subPixelNN (fX, fY, iChannel));
      case 1: return clipped_cast<float, Type>(subPixelLIN (fX, fY, iChannel));
      default: 
        ERROR_LOG ("interpolation method not yet implemented!");
        return clipped_cast<float, Type>(subPixelLIN (fX, fY, iChannel));
    }
  }

  // }}}

  // }}} operators...

  // {{{  ensureCompatible<T>, and ensureDepth<T> utility templates

  template<class Type> 
  inline Img<Type>* ensureCompatible (ImgBase** ppoDst, const ImgParams& p)
    // {{{ open
  {
    if (!ppoDst) return new Img<Type>(p);
    icl::ensureCompatible (ppoDst, icl::getDepth<Type>(), p);
    return (*ppoDst)->asImg<Type>();
  }
  // }}}

  template<class Type>
  Img<Type>* ensureDepth(ImgBase **ppoDst){
    // {{{ open
    ImgBase *poDst = icl::ensureDepth(ppoDst,getDepth<Type>());
    return poDst->asImg<Type>();  
  }
  // }}}

  // }}} enshure ..

  // {{{  shallowCopy function


  template<class Type>
  Img<Type> *Img<Type>::shallowCopy(const Rect &roi, 
                                    const std::vector<int> &channelIndices,
                                    format fmt,
                                    Time t,
                                    ImgBase **ppoDst){
    // {{{ open
    ImgParams p(this->getSize(), 0);
    Img<Type> *poDst = ensureCompatible<Type>(ppoDst,p);
    /// Die ROi wird nicht übernommen ???
    *poDst = *this;  
    if(roi != Rect::null){
      poDst->setROI(roi);
    }
    if(channelIndices.size()){
      ICLASSERT_RETURN_VAL(fmt == formatMatrix || (int)channelIndices.size() == getChannelsOfFormat(fmt),0);
      poDst->setChannels(0);
      poDst->append(this,channelIndices);
    }
    poDst->setFormat(fmt);
    poDst->setTime(t);
    return poDst;  
  }

  // }}} ..

  // }}} shallow ..

  // {{{  copy functions: deepCopy, scaledCopy, flippedCopy convert (with and without ROI)  

  // {{{ copy-functions with ImgBase** argument

  template<class Type>
  Img<Type> *Img<Type>::deepCopy(ImgBase **ppoDst) const{
    // {{{ open

    FUNCTION_LOG("ptr:"<<ppoDst);
    return deepCopy( ensureCompatible<Type>(ppoDst,getParams()) );
  }

  // }}}
      
  template<class Type>
  Img<Type> *Img<Type>::scaledCopy( ImgBase **ppoDst, scalemode eScaleMode) const{
    // {{{ open

    FUNCTION_LOG("ptr:"<<ppoDst);
    if(!ppoDst) return deepCopy();
    return scaledCopy( ensureDepth<Type>(ppoDst), eScaleMode );
  }

  // }}}

  template<class Type>
  Img<Type> *Img<Type>::scaledCopy( const Size &newSize, scalemode eScaleMode) const{
    // {{{ open

    FUNCTION_LOG("new size:"<<newSize.width<<"x"<<newSize.height);
    return scaledCopy( new Img<Type>(newSize, getChannels(), getFormat()), eScaleMode);
  }

  // }}}

  template<class Type>
  Img<Type> *Img<Type>::deepCopyROI(ImgBase **ppoDst) const{
    // {{{ open
    FUNCTION_LOG("ptr:"<<ppoDst); 
    Img<Type> *tmp = ensureDepth<Type>(ppoDst);
    if(!ppoDst){
      tmp->setSize(getROISize());
      tmp->setFormat(getFormat());
      tmp->setChannels(getChannels());
    }
    return deepCopyROI( tmp );
  }

  // }}}
  
  template<class Type>
  Img<Type> *Img<Type>::scaledCopyROI(const Size &newSize, scalemode eScaleMode) const{
    // {{{ open

    FUNCTION_LOG("new size:"<<newSize.width<<"x"<<newSize.height);
    return scaledCopyROI( new Img<Type>(newSize, getChannels(),getFormat()), eScaleMode );
  }

  // }}}
  
  template<class Type>
  Img<Type> *Img<Type>::scaledCopyROI(ImgBase **ppoDst, scalemode eScaleMode) const{
    // {{{ open
      
    FUNCTION_LOG("ptr:"<<ppoDst);
    if(!ppoDst) return deepCopyROI();
    return scaledCopyROI( ensureDepth<Type>(ppoDst),eScaleMode );
  }

  // }}}
  
  // }}} with ImgBase**...

  // {{{ copy-functions with Img<Type>*-argument

  template<class Type>
  Img<Type> *Img<Type>::deepCopy(Img<Type> *poDst) const{
    // {{{ open

    FUNCTION_LOG("ptr:"<<poDst);
    if(!poDst) poDst = new Img<Type>(getParams());
    else poDst->setParams(getParams());
    poDst->setTime(getTime());

    for(int c=getChannels()-1; c>=0; --c){
      deepCopyChannel(this,c,poDst,c);
    }
    return poDst;
    
  }

  // }}}

  template<class Type>
  Img<Type> *Img<Type>::scaledCopy(Img<Type> *poDst, scalemode eScaleMode) const{
    // {{{ open

    FUNCTION_LOG("ptr:"<<poDst);
    if(!poDst) return deepCopy( (ImgBase**)0 );
    poDst->setChannels(getChannels());
    poDst->setFormat(getFormat());
    poDst->setTime(getTime());
    //    poDst->setFullROI(); 
    for(int c=getChannels()-1; c>=0; --c){
      scaledCopyChannelROI(this,c,Point::null,getSize(),poDst,c,Point::null,poDst->getSize(),eScaleMode);
    }
    return poDst;
  }

  // }}}
  
  template<class Type>
  Img<Type> *Img<Type>::deepCopyROI(Img<Type> *poDst) const{
    // {{{ open
    // NEW USING source ROI as well as destination images ROI
    FUNCTION_LOG("ptr:"<< poDst);
    if(!poDst){
      poDst = new Img<Type>(getROISize(),getChannels(),getFormat());
    }else{
      if(poDst->getSize().getDim()){
        ICLASSERT_RETURN_VAL( poDst->getROISize() == getROISize(), NULL);
      }else{
        poDst->setSize(getROISize());
      }
      poDst->setChannels(getChannels());
      poDst->setFormat(getFormat());
    }
    poDst->setTime(getTime());
    for(int c=getChannels()-1; c>=0; --c){
      deepCopyChannelROI(this,c, getROIOffset(), getROISize(), 
                         poDst,c, poDst->getROIOffset(), poDst->getROISize() );
    }
    return poDst;
    
  }

  // }}}
  
  template<class Type>
  Img<Type> *Img<Type>::scaledCopyROI(Img<Type> *poDst, scalemode eScaleMode) const{
    // {{{ open

    FUNCTION_LOG("ptr:"<<poDst);
    if(!poDst) return deepCopyROI();
    poDst->setChannels(getChannels());
    poDst->setFormat(getFormat());
    poDst->setTime(getTime());
    
    for(int c=getChannels()-1; c>=0; --c){
      scaledCopyChannelROI(this,c, getROIOffset(), getROISize(), 
                           poDst,c, poDst->getROIOffset(), poDst->getROISize() , eScaleMode);
    }
    return poDst;
  }

  // }}}
  
  // }}} with Img<Type>* ...
  
  // }}} copy f.. deepCopy,scaledCopy,flippedCopy,convert

  // {{{  channel management: detach, append remove, swap,...

  template<class Type> void
  Img<Type>::detach(int iIndex){
    // {{{ open
    FUNCTION_LOG("index:" << iIndex );
    ICLASSERT_RETURN(iIndex < getChannels());
    
    //---- Make the whole Img independent ----
    for(int i=getStartIndex(iIndex),iEnd=getEndIndex(iIndex);i<iEnd;i++){
      if(m_vecChannels[i].use_count() > 1){
        m_vecChannels[i] = createChannel (getData(i));
      }
    }
  }

  // }}}

  //----------------------------------------------------------------------------
  template<class Type> void
  Img<Type>::removeChannel(int iChannel)
    // {{{ open

  {
    FUNCTION_LOG("removeChannel(" << iChannel << ")");
    ICLASSERT_RETURN(validChannel(iChannel));

    m_vecChannels.erase(m_vecChannels.begin()+iChannel);
    m_oParams.setChannels(m_vecChannels.size());
  }

  // }}}

  //----------------------------------------------------------------------------
  template<class Type> void
  Img<Type>::append(const Img<Type> *poSrc, int iIndex)
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
  Img<Type>::append(const Img<Type> *poSrc, const std::vector<int>& vChannels)
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
    ICLASSERT_RETURN(validChannel(iIndexA));
    ICLASSERT_RETURN(validChannel(iIndexB));

    std::swap(m_vecChannels[iIndexA], m_vecChannels[iIndexB]);
  }

  // }}}

  //----------------------------------------------------------------------------
  template<class Type> inline void 
  Img<Type>::replaceChannel(int iThisIndex, Img<Type>* poSrc, int iOtherIndex) 
    // {{{ open
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN(validChannel(iThisIndex));
    ICLASSERT_RETURN(poSrc->validChannel(iOtherIndex));
    m_vecChannels[iThisIndex] = poSrc->m_vecChannels[iOtherIndex];
  }
  // }}}

  template<class Type>
  Img<Type> Img<Type>::extractChannelImg(int index){
    Img<Type> other(getSize(),0);
    other.append(this,index);
    other.setROI(getROI());
    other.setTime(getTime());
    other.setFormat(formatMatrix);
    return other;
  }
  
  template<class Type>
  const Img<Type> Img<Type>::extractChannelImg(int index) const{
    return const_cast<Img<Type>*>(this)->extractChannelImg(index);
  }
  
  template<class Type>
  Img<Type> Img<Type>::extractChannelImg(const std::vector<int> &indices){
    Img<Type> other(getSize(),0);
    other.append(this,indices);
    other.setROI(getROI());
    other.setTime(getTime());
    other.setFormat(formatMatrix);
    return other;
  }
  
  template<class Type>
  const Img<Type> Img<Type>::extractChannelImg(const std::vector<int> &indices) const{
    return const_cast<Img<Type>*>(this)->extractChannelImg(indices);
  }


  // }}} channel management...

  // {{{  inplace operations: scale, mirror

  //----------------------------------------------------------------------------
  template<class Type> void
  Img<Type>::scale(const Size &size, scalemode eScaleMode){  
    // {{{ open
    FUNCTION_LOG("");
    ICLASSERT_RETURN ((size.width > 0) && (size.height > 0));

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
    const Point& oOffset = bOnlyROI ? getROIOffset() : Point::null;
    const Size&  oSize   = bOnlyROI ? getROISize() : getSize();
    for (int c=0; c < getChannels(); ++c) {
      this->mirror (eAxis, c, oOffset, oSize);
    }
  }
  // }}}

  static inline int getPointerOffset (int x, int y, int iLineLen) {return (x + y*iLineLen);}
  static bool getMirrorPointerOffsets (axis eAxis, bool bInplace, 
                                       const Point& oSrcOffset, const int iSrcLineLen, 
                                       const Point& oDstOffset, const int iDstLineLen, const Size& oSize,
                                       int& s, int& d, int& e, int& eLine, int& iLineWarpS, int& iLineWarpD) {
    // {{{ open
    int iRows=0, iCols=0;
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
        s = getPointerOffset (oSrcOffset.x, oSrcOffset.y, iSrcLineLen);
        d = getPointerOffset (oDstOffset.x, oDstOffset.y + oSize.height - 1, iDstLineLen);
        e = iRows * iSrcLineLen;
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
        s = getPointerOffset (oSrcOffset.x, oSrcOffset.y, iSrcLineLen);
        d = getPointerOffset (oDstOffset.x + oSize.width - 1, oDstOffset.y, iDstLineLen);
        e = iRows * iSrcLineLen;
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
        s = getPointerOffset (oSrcOffset.x, oSrcOffset.y, iSrcLineLen);
        d = getPointerOffset (oDstOffset.x + oSize.width - 1, oDstOffset.y + oSize.height - 1, iDstLineLen);
        e = iRows * iSrcLineLen;

        if (bInplace && (oSize.height % 2)) { // odd ROI height
          iRows++;
          e += oSize.width/2;
        }
        break;
    }
    eLine = iCols;

    return ( (iRows != 0) && (iCols != 0));
  }
  // }}}

  template <typename Type>
  static inline bool getMirrorPointers (axis eAxis, bool bInplace, 
                                        const Type* const srcBegin, const Point& oSrcOffset, const int iSrcLineLen, 
                                        Type* const dstBegin, const Point& oDstOffset, const int iDstLineLen, const Size& oSize,
                                        const Type*& s, Type*& d, const Type*& e, const Type*& eLine, int& iLineWarpS, int& iLineWarpD) {
    // {{{ open
    int deltaSrc, deltaDst, deltaEnd, deltaLineEnd;
    if (!getMirrorPointerOffsets (eAxis, bInplace, oSrcOffset, iSrcLineLen, oDstOffset, iDstLineLen, oSize,
                                  deltaSrc, deltaDst, deltaEnd, deltaLineEnd, iLineWarpS, iLineWarpD))
      return false;
    s = srcBegin + deltaSrc;
    d = dstBegin + deltaDst;
    e = srcBegin + deltaEnd;
    eLine = srcBegin + deltaLineEnd;
    return true;
  }
  // }}}
  template <typename Type>
  static inline bool getMirrorPointers (axis eAxis, bool bInplace, 
                                        Type* const srcBegin, const Point& oSrcOffset, const int iSrcLineLen, 
                                        Type* const dstBegin, const Point& oDstOffset, const int iDstLineLen, const Size& oSize,
                                        Type*& s, Type*& d, const Type*& e, const Type*& eLine, int& iLineWarpS, int& iLineWarpD) {
    // {{{ open
    int deltaSrc, deltaDst, deltaEnd, deltaLineEnd;
    if (!getMirrorPointerOffsets (eAxis, bInplace, oSrcOffset, iSrcLineLen, oDstOffset, iDstLineLen, oSize,
                                  deltaSrc, deltaDst, deltaEnd, deltaLineEnd, iLineWarpS, iLineWarpD))
      return false;
    s = srcBegin + deltaSrc;
    d = dstBegin + deltaDst;
    e = srcBegin + deltaEnd;
    eLine = srcBegin + deltaLineEnd;
    return true;
  }
  // }}}

  template<class Type> void
  Img<Type>::mirror(axis eAxis, int iChannel, 
                    const Point& oOffset, const Size& oSize){  
    // {{{ open

    FUNCTION_LOG("");
  
    static const int aiDstStep[] = {1,-1,-1};
    int      iLineWarpS, iLineWarpD;
    register const Type *e=0, *eLine=0; /* end pointer, line end pointer */
    register Type *s=0, *d=0; /* source pointer, destination pointer */

    if (!getMirrorPointers (eAxis, true, 
                            getData(iChannel), oOffset, getWidth(),
                            getData(iChannel), oOffset, getWidth(), oSize,
                            s, d, e, eLine, iLineWarpS, iLineWarpD)) return;

    register int dir = aiDstStep[eAxis];
    do {
      std::swap (*s, *d);
      ++s; d += dir;
      if (s == eLine) {
        eLine += getWidth(); // end of line pointer jumps whole image width
        s += iLineWarpS;     // source pointer jumps iLineWarpS
        d += iLineWarpD;
      }
    } while (s != e);
  }


#ifdef HAVE_IPP
  template <>
  void Img<icl8u>::mirror(axis eAxis, int iChannel, const Point &oOffset, const Size &oSize) {
    ippiMirror_8u_C1IR(getROIData(iChannel,oOffset),getLineStep(), oSize, (IppiAxis) eAxis);
  }
  template <>
  void Img<icl16s>::mirror(axis eAxis, int iChannel, const Point &oOffset, const Size &oSize) {
    ippiMirror_16u_C1IR((Ipp16u*) getROIData(iChannel,oOffset), getLineStep(), oSize, (IppiAxis) eAxis);
  }
  template <>
  void Img<icl32s>::mirror(axis eAxis, int iChannel, const Point &oOffset, const Size &oSize) {
    ippiMirror_32s_C1IR( getROIData(iChannel,oOffset), getLineStep(), oSize, (IppiAxis) eAxis);
  }
  template <>
  void Img<icl32f>::mirror(axis eAxis, int iChannel, const Point &oOffset, const Size &oSize) {
    ippiMirror_32s_C1IR((Ipp32s*) getROIData(iChannel,oOffset), getLineStep(), oSize, (IppiAxis) eAxis);
  }
#endif

  // }}}

  // }}} inplace operations...
  
  // {{{  setter: setSize, setChannels

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

  // }}} setter...

  // {{{  Get Min/Max functions: 

  // {{{     getMax

  template<class Type> Type 
  Img<Type>::getMax() const{
    FUNCTION_LOG("");

    if (getChannels() == 0) return 0;
    Type tMax = getMax(0);
    for(int i=1;i<getChannels();i++)
      tMax = iclMax(tMax,getMax(i));
    return tMax;    
  }

  // fallback for all types
  template<class Type> Type 
  Img<Type>::getMax(int channel, Point *coords) const {
    FUNCTION_LOG("iChannel: " << channel);
    ICLASSERT_RETURN_VAL( validChannel(channel), 0 );
    ICLASSERT_RETURN_VAL( getROISize().getDim(), 0 );
    if(hasFullROI()){
      const_iterator it = std::max_element(begin(channel),end(channel));
      if(coords)*coords = getLocation(it,channel);
      return *it;
    }else{
      const_roi_iterator it = std::max_element(beginROI(channel),endROI(channel));
      if(coords)*coords = getLocation(&*it,channel);
      return *it;
    }
#if 0
    OLDER IMPLEMENTATION
    const_iterator it = getROIIterator(iChannel);
    
    if (!it.inRegion()) return 0; // empty region
    Type vMax = *it; 
    
    if(!coords){
      ++it;    
      for (; it.inRegion(); ++it) {
        vMax = std::max (vMax, *it);
      }
      return vMax;
    }else{
      coords->x = it.x();
      coords->y = it.y();
      ++it;
      for (; it.inRegion(); ++it) {
        if(*it > vMax){
          coords->x = it.x();
          coords->y = it.y();
          vMax = *it;
        }
      }
      return vMax;
    }
#endif
  }
#ifdef HAVE_IPP
#define ICL_INSTANTIATE_DEPTH(T)                                                                            \
template<> icl ## T                                                                                         \
Img<icl ## T>::getMax(int iChannel,Point *coords) const {                                                   \
   ICLASSERT_RETURN_VAL( validChannel(iChannel), 0 );                                                       \
   icl ## T vMax = 0;                                                                                       \
   if(coords){                                                                                              \
   ippiMaxIndx_ ## T ## _C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMax,&coords->x,&coords->y); \
   }else{                                                                                                   \
   ippiMax_ ## T ## _C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMax);                           \
   }                                                                                                        \
   return vMax;                                                                                             \
}

  ICL_INSTANTIATE_DEPTH(8u)
    ICL_INSTANTIATE_DEPTH(16s)
    ICL_INSTANTIATE_DEPTH(32f)
#undef ICL_INSTANTIATE_DEPTH
#endif

    // }}}

    // {{{     getMin

    template<class Type> Type 
  Img<Type>::getMin() const{
    FUNCTION_LOG("");

    if (getChannels() == 0) return 0;
    Type tMin = getMin(0);
    for(int i=1;i<getChannels();i++)
      tMin = iclMin(tMin,getMin(i));
    return tMin;    
  }

  // fallback for all types
  template<class Type> Type 
  Img<Type>::getMin(int channel, Point *coords) const {
 FUNCTION_LOG("iChannel: " << channel);
    ICLASSERT_RETURN_VAL( validChannel(channel), 0 );
    ICLASSERT_RETURN_VAL( getROISize().getDim(), 0 );
    if(hasFullROI()){
      const_iterator it = std::min_element(begin(channel),end(channel));
      if(coords)*coords = getLocation(it,channel);
      return *it;
    }else{
      const_roi_iterator it = std::min_element(beginROI(channel),endROI(channel));
      if(coords)*coords = getLocation(&*it,channel);
      return *it;
    }
    
#if 0 
    OLDER IMPL...
    FUNCTION_LOG("iChannel: " << iChannel);
    ICLASSERT_RETURN_VAL( validChannel(iChannel), 0 );

    const_iterator it = getROIIterator(iChannel);
    if (!it.inRegion()) return 0; // empty region

    Type vMin = *it; 
    
    if(!coords){
      ++it;
      for (; it.inRegion(); ++it) {
        vMin = std::min (vMin, *it);
      }
      return vMin;
    }else{
      coords->x = it.x();
      coords->y = it.y();
      ++it;
      for (; it.inRegion(); ++it) {
        if(*it < vMin){
          coords->x = it.x();
          coords->y = it.y();
          vMin = *it;
        }
      }
    }
    return vMin;
#endif
  }
#ifdef HAVE_IPP
#define ICL_INSTANTIATE_DEPTH(T)                                                                         \
template<> icl##T                                                                                        \
Img<icl ## T>::getMin(int iChannel, Point *coords) const {                                               \
   ICLASSERT_RETURN_VAL( validChannel(iChannel), 0 );                                                    \
   icl##T vMin = 0;                                                                                      \
   if(coords){                                                                                           \
   ippiMinIndx_##T##_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMin,&coords->x,&coords->y);  \
   }else{                                                                                                \
   ippiMin_##T##_C1R (getROIData(iChannel),getLineStep(),getROISize(),&vMin);                            \
   }                                                                                                     \
   return vMin; }  


  ICL_INSTANTIATE_DEPTH(8u)
    ICL_INSTANTIATE_DEPTH(16s)
    ICL_INSTANTIATE_DEPTH(32f)
#undef ICL_INSTANTIATE_DEPTH
#endif

    // }}}

    // {{{     getMinMax

    template<class Type> const Range<Type>
  Img<Type>::getMinMax() const
  {
    FUNCTION_LOG("");

    if (getChannels() == 0) {
      return Range<Type>();
    }
   
    Range<Type> r = getMinMax(0);
    for(int i=1;i<getChannels();i++) {
      Range<Type> k = getMinMax(i);
      r.minVal = iclMin(r.minVal,k.minVal);
      r.maxVal = iclMax(r.maxVal,k.maxVal);
    }
    return r;
  }
  
  template<class iterator>
  std::pair<iterator,iterator> get_min_and_max_element_util(iterator begin, iterator end){
    std::pair<iterator,iterator> mm;
    if(begin == end) return mm;
    mm.first = begin;
    mm.second = begin;
    for(++begin; begin != end; ++begin){
      if(*begin < *mm.first) mm.first = begin;
      if(*begin > *mm.second) mm.second = begin;
    }
    return mm;
  }

  // fallback for all types
  template<class Type> const Range<Type>
  Img<Type>::getMinMax(int channel, Point *minCoords, Point *maxCoords) const {
    ICLASSERT_RETURN_VAL( validChannel(channel), Range<Type>() );
    ICLASSERT_RETURN_VAL( getROISize().getDim(), Range<Type>() );
    if(hasFullROI()){
      std::pair<const_iterator,const_iterator> its = get_min_and_max_element_util(begin(channel),end(channel));
      if(minCoords){
        *minCoords = getLocation(its.first,channel);
        *maxCoords = getLocation(its.second,channel);
      }
      return Range<Type>(*its.first,*its.second);
    }else{
      std::pair<const_roi_iterator,const_roi_iterator> its = get_min_and_max_element_util(beginROI(channel),endROI(channel));
      if(minCoords){
        *minCoords = getLocation(&*its.first,channel);
        *maxCoords = getLocation(&*its.second,channel);
      }
      return Range<Type>(*its.first,*its.second);
    }
#if 0
    older version ...

    FUNCTION_LOG("iChannel: " << iChannel);
    ICLASSERT_RETURN_VAL(validChannel(iChannel), Range<Type>());
    if( (minCoords && !maxCoords) || (maxCoords && !minCoords) ){
      ERROR_LOG("please define minCoords AND maxCoords or do not define BOTH (returning 0)");
      return Range<Type>();
    }
    ICLASSERT_RETURN_VAL(getROISize().getDim(),Range<Type>());

    Range<Type> r;

    Type *minPtr=0;
    type *maxPtr=0;
    if(hasFullROI()){
      const_iterator it = begin(iChannel);
      const_iterator itEnd = end(iChannel);
      Type min = *it, max=*it;
      it++;
      for(;it!=itEnd;++it){
        min = iclMin(min,*it);
        max = iclMax(max,*it);
      }
      
    }
    
    const_iterator it = getROIIterator(iChannel);
    if (!it.inRegion()) return Range<Type>(); // empty region: return with 0, 0
    r.minVal = r.maxVal = *it;
    if(minCoords){
      minCoords->x = maxCoords->x = it.x();
      minCoords->y = maxCoords->y = it.y();
      ++it;
      for (; it.inRegion(); ++it) {
        Type v = *it;
        if(r.minVal < v){
          r.minVal = v;
          minCoords->x = it.x();
          minCoords->y = it.y();
        }
        if(r.maxVal > v){
          r.maxVal = v;
          maxCoords->x = it.x();
          maxCoords->y = it.y();
        }
      }
    }else{
      ++it;
      for (; it.inRegion(); ++it) {
        r.minVal = std::min (r.minVal, *it);
        r.maxVal = std::max (r.maxVal, *it);
      }
    }
    return r;
#endif

  }

#ifdef HAVE_IPP

#define ICL_INSTANTIATE_DEPTH(T)                                                              \
template<> const Range<icl##T>                                                                \
Img<icl ## T>::getMinMax(int iChannel,Point *minCoords, Point *maxCoords) const {             \
   ICLASSERT_RETURN_VAL( validChannel(iChannel) ,Range<icl##T>());                            \
   if((minCoords && !maxCoords) || (maxCoords && !minCoords)){                                \
     ERROR_LOG("please define minCoords AND maxCoords or do not define BOTH (returning (0,0))");  \
     return Range<icl##T>(0,0);                                                               \
   }                                                                                          \
   if(minCoords){                                                                             \
   Range<icl32f> r;                                                                           \
   ippiMinMaxIndx_ ## T ## _C1R (getROIData(iChannel),getLineStep(),                          \
                                 getROISize(), &(r.minVal), &(r.maxVal),                      \
                                 minCoords,maxCoords);                                        \
   return r.castTo<icl##T>();                                                                 \
   }else{                                                                                     \
   Range<icl##T> r;                                                                           \
   ippiMinMax_ ## T ## _C1R (getROIData(iChannel),getLineStep(),                              \
                             getROISize(), &(r.minVal), &(r.maxVal));                         \
   return r;                                                                                  \
   }                                                                                          \
}

  ICL_INSTANTIATE_DEPTH(8u)
  ICL_INSTANTIATE_DEPTH(32f)
#undef ICL_INSTANTIATE_DEPTH
#endif


    // }}}

    // }}} Get Min/Max...

    // {{{  Auxillary  functions 

    template<class Type>
  SmartArray<Type> Img<Type>::createChannel(Type *ptDataToCopy) const {
    // {{{ open
    FUNCTION_LOG("");
    int dim = getDim();
    if(!dim) return SmartArray<Type>();

    Type *ptNewData = new Type[dim];
    if(ptDataToCopy){
      memcpy(ptNewData,ptDataToCopy,dim*sizeof(Type));
    }else{
      std::fill(ptNewData,ptNewData+dim,0);
    }
    return SmartArray<Type>(ptNewData);
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
   
    const Type* pLL = getData(iChannel) + xll + yll * getWidth();
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
  float Img<Type>::subPixelRA(float x, float y, float w, float h, int iChannel) const{
    // {{{ open

    ERROR_LOG ("region average interpolation is not yet implemented!");
    return subPixelLIN (x, y, iChannel);

    Rect32f r(x,y,w,h);
    Rect o(floor(x),floor(y),ceil(x+w)-floor(x),ceil(y+h)-floor(y));
    if(o != (o & getImageRect()) ){
      ERROR_LOG("given rect extends image bounds");
      return 0;
    }
    /*
        float UL = (Rect32f(o.x,o.y,1,1) & r).getDim();    
        float UR = (Rect32f(o.right()-1,o.y,1,1) & r).getDim();
        float LL = (Rect32f(o.x,o.bottom()-1,1,1) & r).getDim();    
        float LR = (Rect32f(o.right()-1,o.bottom()-1,1,1) & r).getDim();    
        
        float U=0,R=0,B=0,L=0;
        //    ... hmmmm
     */

     ERROR_LOG ("region average interpolation is not yet implemented!");
    return subPixelLIN (x, y, iChannel);
  }

  // }}}

  // }}} Auxillary...

  // {{{  normalize and clear

  // {{{  normalize wrappers

  template<class Type> void
  Img<Type>::normalizeAllChannels(const Range<Type> &dstRange){
    FUNCTION_LOG("");
    for (int c=0;c<getChannels();c++) {
      normalizeChannel(c, dstRange);
    }
  }
  
  template<class Type> void
  Img<Type>::normalizeChannel(int iChannel, const Range<Type> &srcRange, const Range<Type> &dstRange){ 
    FUNCTION_LOG("");
    normalize(iChannel, srcRange,dstRange);
  }

  template<class Type> void
  Img<Type>::normalizeChannel(int iChannel,const Range<Type> &dstRange) {
    FUNCTION_LOG("");
    normalize(iChannel, getMinMax(iChannel),dstRange);
  }

  template<class Type> void
  Img<Type>::normalizeImg(const Range<Type> &srcRange, const Range<Type> &dstRange){
    FUNCTION_LOG("");
    for (int c=0;c<getChannels();c++) {
      normalizeChannel(c, srcRange,dstRange);
    }
  }
  
  template<class Type> void
  Img<Type>::normalizeImg(const Range<Type> &dstRange) {
    FUNCTION_LOG("");
    for (int c=0;c<getChannels();c++) {
      normalizeChannel(c, getMinMax(),dstRange);
    }
  }
  
  // }}} ..

  // {{{  normalize main methods

  template <class Type> void 
  Img<Type>::normalize(int iChannel, const Range<Type> &srcRange, const Range<Type> &dstRange){ 
    FUNCTION_LOG("");
    for(int c = getStartIndex(iChannel);c<getEndIndex(iChannel);c++){
      icl64f fScale  = (icl64f)(dstRange.getLength()) / (icl64f)(srcRange.getLength());
      icl64f fShift  = (icl64f)(srcRange.maxVal * dstRange.minVal - srcRange.minVal * dstRange.maxVal) / srcRange.getLength();
      const_roi_iterator e = endROI(c);
      for(roi_iterator p=beginROI(c);p!=e; ++p) {
        *p = clipped_cast<icl64f,Type>( icl::clip( fShift + (icl64f)(*p) * fScale, icl64f(dstRange.minVal),icl64f(dstRange.maxVal) ) );
      }
    }
  }

#ifdef HAVE_IPP
  template <> void 
  Img<icl32f>::normalize(int iChannel, const Range<icl32f> &srcRange, const Range<icl32f> &dstRange){
    FUNCTION_LOG("");
    for(int c = getStartIndex(iChannel);c<getEndIndex(iChannel);c++){
      icl32f fScale  = dstRange.getLength()/srcRange.getLength();
      icl32f fShift  = (srcRange.maxVal * dstRange.minVal - srcRange.minVal * dstRange.maxVal)/srcRange.getLength();
  
      ippiMulC_32f_C1IR (fScale, getROIData(c), getLineStep(),getROISize());
    
      if (fShift != 0) {
        ippiAddC_32f_C1IR (fShift, getROIData(c), getLineStep(),getROISize());
      }
    }
  }
#endif

  // }}}

  template<class Type>
  void Img<Type>::clear(int iIndex, Type tValue, bool bROIOnly) 
    // {{{ open
  {
    //---- Log Message ----
    FUNCTION_LOG("clear(" << iIndex << "," << tValue << ")");
    ICLASSERT_RETURN( iIndex < getChannels() );  
  
    Point offs = bROIOnly ? getROIOffset() : Point::null;
    Size size = bROIOnly ? getROISize() : getSize();
    for(int i=getStartIndex(iIndex),iEnd=getEndIndex(iIndex);i<iEnd;i++){
      clearChannelROI(this,i,tValue,offs,size);
    }
  }
  // }}}
     
  // }}} normalize and clear...

  // {{{  Global functions: combineImages , scaledCopyChannelROI


  template<class ImgType>
  const ImgType* combineImages (const std::vector<const ImgType*>& vec, ImgBase** ppoDst) {
    // {{{ open

    FUNCTION_LOG("");
    // find first non-zero element
    typename std::vector<const ImgType*>::const_iterator 
    first = std::find_if (vec.begin(), vec.end(), 
                          std::bind2nd(std::not_equal_to<const ImgType*>(), 
                                       reinterpret_cast<const ImgType*>(0))),
    end   = vec.end();
    // check for empty vector
    if (first == vec.end()) return 0;

    // create image with parameters of first image in vector
    icl::ensureCompatible (ppoDst, *first);
    ImgType* poDst = static_cast<ImgType*>(*ppoDst);
    // some cache variables
    const Size& dstSize = poDst->getSize();
    const Rect& dstROI  = poDst->getROI();
    icl::format fmt = (*first)->getFormat();
    bool bKeepROI=true;
    unsigned int nCount = 0;
    for (; first != end; ++first) {
      if ((*first)->getSize() == dstSize) {
        if ((*first)->getROI() != dstROI) bKeepROI = false;
        poDst->append (*first);
        ++nCount;
      } else ERROR_LOG ("image size doesn't match");
    }
    if (nCount == 1) poDst->setFormat (fmt); // keep format of single source image
    if (!bKeepROI) poDst->setFullROI (); // reset ROI if subimages' ROIs do not match
    return poDst;
  }

  // }}}

  // file local (i.e. private) function to mediate between ImgBase and Img<T> variants
  template<typename T>
  const Img<T>* __combineImages (const std::vector<const ImgBase*>& vec, ImgBase** ppoDst) {
    // {{{ open

    std::vector<const Img<T>*> vecTyped;
    // create correctly typed vector
    for (std::vector<const ImgBase*>::const_iterator it=vec.begin(), end=vec.end();
         it != end; ++it) {
      const ImgBase *pImgBase = *it;
      if (pImgBase->getDepth () == icl::getDepth<T>())
        vecTyped.push_back (pImgBase->asImg<T>());
      else ERROR_LOG ("image depth doesn't match");
    }
    return combineImages (vecTyped, ppoDst);
  }

  // }}}

  template<>
  const ImgBase* combineImages<ImgBase> (const std::vector<const ImgBase*>& vec, ImgBase** ppoDst) {
    // {{{ open

    FUNCTION_LOG("ImgBase");
    // find first non-zero element
    std::vector<const ImgBase*>::const_iterator 
    first = std::find_if (vec.begin(), vec.end(), 
                          std::bind2nd(std::not_equal_to<const ImgBase*>(), 
                                       reinterpret_cast<const icl::ImgBase*>(0)));
    // check for empty vector
    if (first == vec.end()) { 
      // remove all channels from *ppoDst
      if (ppoDst && *ppoDst) {(*ppoDst)->setChannels(0); return *ppoDst;}
      // or return Null pointer directly
      else return 0;
    }
    switch ((*first)->getDepth()) {
#define ICL_INSTANTIATE_DEPTH(T) \
     case depth ## T: return __combineImages<icl ## T> (vec, ppoDst); break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
      }
    return 0;
  }

  // }}}


#define CHECK_VALUES(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize)                                             \
  FUNCTION_LOG("");                                                                                                 \
  ICLASSERT_RETURN( src && dst );                                                                                   \
  ICLASSERT_RETURN( srcSize == dstSize );                                                                           \
  ICLASSERT_RETURN( src->validChannel(srcC) );                                                                      \
  ICLASSERT_RETURN( dst->validChannel(dstC) );                                                                      \
  ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0);                          \
  ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() );   \
  ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );   

#define CHECK_VALUES_NO_SIZE(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize)                                     \
  FUNCTION_LOG("");                                                                                                 \
  ICLASSERT_RETURN( src && dst );                                                                                   \
  ICLASSERT_RETURN( src->validChannel(srcC) );                                                                      \
  ICLASSERT_RETURN( dst->validChannel(dstC) );                                                                      \
  ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0);                          \
  ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() );   \
  ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );   

  // scale channel ROI function for abitrary image scaling operations
  template<class T> 
  void scaledCopyChannelROI(const Img<T> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                            Img<T> *dst, int dstC, const Point &dstOffs, const Size &dstSize,
                            scalemode eScaleMode){
    // {{{ open

    CHECK_VALUES_NO_SIZE(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize);
 
    float fSX = ((float)srcSize.width)/(float)(dstSize.width); 
    float fSY = ((float)srcSize.height)/(float)(dstSize.height);

    float (Img<T>::*subPixelMethod)(float fX, float fY, int iChannel) const;
    switch(eScaleMode) {
      case interpolateNN:
        subPixelMethod = &Img<T>::subPixelNN;
        break;
      case interpolateLIN:
        fSX = ((float)srcSize.width-1)/(float)(dstSize.width); 
        fSY = ((float)srcSize.height-1)/(float)(dstSize.height);
        subPixelMethod = &Img<T>::subPixelLIN;
        break;
      default:
        ERROR_LOG("unknown interpoation method!");
        subPixelMethod = &Img<T>::subPixelNN;
        break;
    }

    ImgIterator<T> itDst(dst->getData(dstC),dst->getSize().width,Rect(dstOffs,dstSize));
    const ImgIterator<T> itDstEnd = ImgIterator<T>::create_end_roi_iterator(dst->getData(dstC),dst->getWidth(),Rect(dstOffs,dstSize));
    int xD = 0;
    int yD = 0;
    float yS = srcOffs.y + fSY * yD;
    for(; itDst != itDstEnd ; ++itDst) {
      *itDst = clipped_cast<float, T>((src->*subPixelMethod)(srcOffs.x + fSX * xD, yS, srcC));
      if (++xD == dstSize.width) {
        yS = srcOffs.y + fSY * ++yD;
        xD = 0;
      }
    }
  }

  // }}}

#define ICL_INSTANTIATE_DEPTH(D)  template void scaledCopyChannelROI<icl##D>            \
                                  (const Img<icl##D>*,int,const Point&,const Size&,     \
                                  Img<icl##D>*,int,const Point&,const Size&,scalemode); 

  /// IPP-OPTIMIZED specialization for icl8u to icl8u ROI sclaing (using ippiResize)
#ifdef HAVE_IPP
  template<> inline void 
  scaledCopyChannelROI<icl8u>(const Img<icl8u> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                              Img<icl8u> *dst, int dstC, const Point &dstOffs, const Size &dstSize,
                              scalemode eScaleMode)
    // {{{ open 

  {
    CHECK_VALUES_NO_SIZE(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize);

    // attention: for source image IPP wants indeed the *image* origin
    ippiResize_8u_C1R(src->getData(srcC),src->getSize(),src->getLineStep(),Rect(srcOffs,srcSize),
                      dst->getROIData(dstC,dstOffs),dst->getLineStep(),dstSize,
                      (float)dstSize.width/(float)srcSize.width,
                      (float)dstSize.height/(float)srcSize.height,(int)eScaleMode);
  }

  // }}}

  /// IPP-OPTIMIZED specialization for icl32f to icl32f ROI sclaing (using ippiResize)
  template<> inline void 
  scaledCopyChannelROI<icl32f>(const Img<icl32f> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                               Img<icl32f> *dst, int dstC, const Point &dstOffs, const Size &dstSize,
                               scalemode eScaleMode)
    // {{{ open

  {
    FUNCTION_LOG("");
    CHECK_VALUES_NO_SIZE(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize);
    
    // attention: for source image IPP wants indeed the *image* origin
    ippiResize_32f_C1R(src->getData(srcC),src->getSize(),src->getLineStep(),Rect(srcOffs,srcSize),
                       dst->getROIData(dstC,dstOffs),dst->getLineStep(),dstSize,
                       (float)dstSize.width/(float)srcSize.width,
                       (float)dstSize.height/(float)srcSize.height,(int)eScaleMode);
  }

  // }}}

  // ipp case: do not instantiate the already specialized functions 8u and 32f
  ICL_INSTANTIATE_DEPTH(16s)
    ICL_INSTANTIATE_DEPTH(32s)
    ICL_INSTANTIATE_DEPTH(64f)
#else
    // no-ipp case instantiate all functions
    ICL_INSTANTIATE_ALL_DEPTHS
#endif

#undef ICL_INSTANTIATE_DEPTH


  // {{{    flippedCopyChannelROI


  // mirror copy ROI of one image to the ROI of the other (for selected channel)
  template<class T>
  void flippedCopyChannelROI(axis eAxis,
                             const Img<T> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                             Img<T> *dst, int dstC, const Point &dstOffs, const Size &dstSize){
    // {{{ open

    FUNCTION_LOG("");
    CHECK_VALUES(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize);
  
    static const int aiDstStep[] = {1,-1,-1};
    int      iLineWarpS, iLineWarpD;
    register const T *s=0, *e=0, *eLine=0; /* source pointer, end pointer, line end pointer */
    register T *d=0; /* destination pointer */

    if (!getMirrorPointers (eAxis, false, 
                            src->getData(srcC), srcOffs, src->getWidth(),
                            dst->getData(dstC), dstOffs, dst->getWidth(), srcSize,
                            s, d, e, eLine, iLineWarpS, iLineWarpD)) return;

    if (eAxis == axisHorz) {
      int iSrcStep = src->getSize().width, iDstStep = dst->getSize().width;
      //     int nBytes = sizeof(T) * srcSize.width;
      // line-wise memcpy is possible
      for (; s != e; s += iSrcStep, d -= iDstStep)
        icl::copy<T>(s,s+srcSize.width,d);//memcpy (d, s, nBytes);
      return;
    }

    register int dir = aiDstStep[eAxis];
    do {
      *d = *s;
      ++s; d += dir;
      if (s == eLine) {
        eLine += src->getSize().width; // end of line pointer jumps whole image width
        s += iLineWarpS;               // source pointer jumps iLineWarpS
        d += iLineWarpD;
      }
    } while (s != e);
  }

  // }}}


#define ICL_INSTANTIATE_DEPTH(D) template void flippedCopyChannelROI<icl##D>(axis eAxis,                        \
                                 const Img<icl##D> *src, int srcC, const Point &srcOffs, const Size &srcSize,   \
                                 Img<icl##D> *dst, int dstC, const Point &dstOffs, const Size &dstSize);

#ifdef HAVE_IPP
  /// IPP-OPTIMIZED specialization for icl8u image flipping
  template <>
  inline void flippedCopyChannelROI<icl8u>(axis eAxis, 
                                           const Img<icl8u> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                                           Img<icl8u> *dst, int dstC, const Point &dstOffs, const Size &dstSize) {
    // {{{ open

    CHECK_VALUES(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize);
   
    ippiMirror_8u_C1R(src->getROIData(srcC,srcOffs),src->getLineStep(),
                      dst->getROIData(dstC,dstOffs),dst->getLineStep(),srcSize,(IppiAxis) eAxis);
  }

  // }}}

  /// IPP-OPTIMIZED specialization for icl8u image flipping
  template <>
  inline void flippedCopyChannelROI<icl32f>(axis eAxis, 
                                            const Img<icl32f> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                                            Img<icl32f> *dst, int dstC, const Point &dstOffs, const Size &dstSize) {
    // {{{ open

    CHECK_VALUES(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize);
   
    ippiMirror_32s_C1R((Ipp32s*) src->getROIData(srcC,srcOffs),src->getLineStep(),
                       (Ipp32s*) dst->getROIData(dstC,dstOffs),dst->getLineStep(),srcSize,(IppiAxis) eAxis);
  }

  // }}}
  // ipp case: do not instantiate the already specialized functions 8u and 32f
  ICL_INSTANTIATE_DEPTH(16s)
    ICL_INSTANTIATE_DEPTH(32s)
    ICL_INSTANTIATE_DEPTH(64f)

#else
    // no-ipp case instantiate all functions
    ICL_INSTANTIATE_ALL_DEPTHS
#endif

#undef ICL_INSTANTIATE_DEPTH




  // }}}  flippedCopyChannelROI...

  // {{{    flippedCopy / flippedCopyROI

  void flippedCopy(axis eAxis, const ImgBase *poSrc, ImgBase **ppoDst){
    // {{{ open

    ICLASSERT_RETURN(poSrc);
    
    ImgBase *poDst = ensureCompatible(ppoDst,poSrc->getDepth(),poSrc->getSize(),poSrc->getChannels(),poSrc->getFormat());
    poDst->setTime(poSrc->getTime());
    


    // imagewidth = 300, roioffs=0, roiwidth=100
    if(poSrc->getROISize() != poSrc->getSize()){
      Size rs = poSrc->getROISize();
      Size is = poSrc->getSize();
      Point o = poSrc->getROIOffset();
      Point newO = o;
      switch(eAxis){
        case axisHorz:
          newO.y = is.height-o.y-rs.height;
          break;
        case axisVert:
          newO.x = is.width-o.x-rs.width; 
          break;
        case axisBoth:
          newO.x = is.width-o.x-rs.width; 
          newO.y = is.height-o.y-rs.height;
          break;
      }
      poDst->setROI(newO,rs);
    }
    const ImgBase *poFullSrc = poSrc->shallowCopy(Rect(Point::null,poSrc->getSize()));
    ImgBase *poFullDst = poDst->shallowCopy(Rect(Point::null,poDst->getSize()));
    flippedCopyROI(eAxis,poFullSrc,&poFullDst);                 
  }

  // }}}

  void flippedCopyROI(axis eAxis, const ImgBase *poSrc, ImgBase **ppoDst){
    // {{{ open

    ICLASSERT_RETURN(poSrc);
    ImgBase *poDst = 0;
    if(!ppoDst){
      poDst = imgNew(poSrc->getDepth(),poSrc->getROISize(),poSrc->getChannels(),poSrc->getFormat());
    }else if(! *ppoDst){
      poDst = imgNew(poSrc->getDepth(),poSrc->getROISize(),poSrc->getChannels(),poSrc->getFormat());
      *ppoDst = poDst;
    }else{
      poDst = ensureDepth(ppoDst,poSrc->getDepth());
      ICLASSERT_RETURN( poDst->getROISize() == poSrc->getROISize());
      poDst->setChannels(poSrc->getChannels());
      poDst->setFormat(poSrc->getFormat());
    }
    poDst->setTime(poSrc->getTime()); 
    for(int c=poSrc->getChannels()-1; c>=0; --c){
      switch(poSrc->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                                                                           \
        case depth##D :flippedCopyChannelROI(eAxis,poSrc->asImg<icl##D>(),c, poSrc->getROIOffset(), poSrc->getROISize(),   \
                                             poDst->asImg<icl##D>(),c, poDst->getROIOffset(), poDst->getROISize() ); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }
  }

  // }}}
    
  // }}} flippedCopy / flippedCopyROI

  // {{{ printAsMatrix
  template<class Type>
  void Img<Type>::printAsMatrix(const std::string &fmt, bool visROI) const{
    std::cout << "image matrix:    size: " << getSize() << std::endl;
    std::cout << "             channels: " << getChannels() << std::endl;
    std::cout << "                  ROI: " << (hasFullROI() ? std::string("full") : str(getROI())) << std::endl;
    std::string fmtFull="%";
    visROI = visROI && !hasFullROI();
    if(visROI){
      fmtFull+=fmt+"f%c";
    }else{
      fmtFull+=fmt+"f ";
    }
    Rect r = getROI();
    r.width-=1;
    r.height-=1;
    for(int i=0;i<getChannels();++i){
      std::cout << "channel " << i << std::endl;

      for(int y=0;y<getHeight();++y){
        std::cout << "| ";
        for(int x=0;x<getWidth();++x){
          if(visROI){
            printf(fmtFull.c_str(),(icl64f)(*this)(x,y,i),(r.contains(x,y)?'r':' '));
          }else{
            printf(fmtFull.c_str(),(icl64f)(*this)(x,y,i));
          }
        }
        std::cout << "|" << std::endl;
      }
      std::cout << "--------------------" << std::endl;
    }
  }
  

  
  // }}}


  template<class T>
  bool Img<T>::isIndependent() const{
    for(int i=0;i<getChannels();++i){
      if(m_vecChannels[i].use_count() > 1){
        return false;
      }
    }
    return true;
  }
  
  template<class Type>
  Point Img<Type>::getLocation(const Type *p, int channel, bool relToROI) const{
    ICLASSERT_RETURN_VAL(validChannel(channel), Point::null);
    ICLASSERT_RETURN_VAL(getDim(), Point::null);
    int offs = (int)(p-getData(channel));
    int x = offs%getWidth();
    int y = offs/getWidth();
    if(relToROI){
      return Point(x-getROI().x,y-getROI().y);
    }else{
      return Point(x,y);
    }
  }


  
  
  template<class Type>
  void Img<Type>::fillBorder(bool setFullROI){
    ICLASSERT_RETURN( !hasFullROI() );
    Rect im = getImageRect();
    Rect roi = getROI();
    
    Rect rs[4] = { 
      Rect(0,            0,             roi.x+1               ,roi.y+1),                  // upper left
      Rect(roi.right()-1,0,             im.width-roi.right()+1,roi.y+1),                  // upper right
      Rect(roi.right()-1,roi.bottom()-1,im.width-roi.right()+1,im.height-roi.bottom()+1), // lower right
      Rect(0,            roi.bottom()-1,roi.x+1,               im.height-roi.bottom()+1)  // lower left
    };
    

    for(int c=0;c<getChannels();c++){      
      Type ps[4]={
        (*this)(roi.x,         roi.y,          c),// = 255; // upper left
        (*this)(roi.right()-1, roi.y,          c),// = 255; // upper right
        (*this)(roi.right()-1, roi.bottom()-1, c),// = 255; // lower right
        (*this)(roi.x,         roi.bottom()-1, c)// = 255; // lower left
      };
      // clear the corners
      for(int i=0;i<4;i++){
        clearChannelROI<Type>(this,c,ps[i],rs[i].ul(),rs[i].getSize());
      }
      // left
      Point srcOffs = Point(roi.x,roi.y+1);
      Size srcDstSize = Size(1,roi.height-2);
      if(roi.x>0){
        for(Point p(0,roi.y+1);p.x!=roi.x;p.x++){
          deepCopyChannelROI(this,c, srcOffs, srcDstSize, this,c,p,srcDstSize);
        }
      }
      // right
      srcOffs.x=roi.right()-1;
      if(roi.right() < im.right()){
        for(Point p(srcOffs.x+1,srcOffs.y);p.x<im.width;p.x++){
          deepCopyChannelROI(this,c, srcOffs, srcDstSize, this,c,p,srcDstSize);
        }
      }
      // top
      srcOffs = Point(roi.x+1,roi.y);
      srcDstSize = Size(roi.width-2,1);
      if(roi.y>0){
        for(Point p(srcOffs.x,roi.y+1);p.y>=0;p.y--){
          deepCopyChannelROI(this,c, srcOffs, srcDstSize, this,c,p,srcDstSize);
        }
      }
      // bottom
      if(roi.bottom()<im.bottom()){
        srcOffs.y = roi.bottom();
        for(Point p(srcOffs.x,roi.bottom()+1);p.y<im.height;p.y++){
          deepCopyChannelROI(this,c, srcOffs, srcDstSize, this,c,p,srcDstSize);
        }
      }
    }
  }

  template<class Type>
  void Img<Type>::fillBorder(icl64f val, bool setFullROI){
    ICLASSERT_RETURN( !hasFullROI() );
    Rect roi = getROI();
    Size s = getSize();
    for(int c=0;c<getChannels();c++){
      // top
      clearChannelROI<Type>(this,c,val, Point::null,        
                         Size(s.width,roi.top()));
      // bottom
      clearChannelROI<Type>(this,c,val, Point(0,roi.bottom()),
                         Size(s.width,s.height-roi.bottom()));
      // left
      clearChannelROI<Type>(this,c,val, Point(0,roi.top()),
                         Size(roi.left(),roi.height));
      // right
      clearChannelROI<Type>(this,c,val, roi.ur(),
                         Size(s.width-roi.right(),roi.height) );
    }    
  }

  template<class Type>
  void Img<Type>::fillBorder(const std::vector<icl64f> &vals, bool setFullROI){
    ICLASSERT_RETURN( !hasFullROI() );
    ICLASSERT_RETURN((int)vals.size() >= getChannels());
    Rect roi = getROI();
    Size s = getSize();
    for(int c=0;c<getChannels();c++){
      // top
      clearChannelROI<Type>(this,c,vals[c], Point::null,        
                         Size(s.width,roi.top()));
      // bottom
      clearChannelROI<Type>(this,c,vals[c], Point(0,roi.bottom()),
                         Size(s.width,s.height-roi.bottom()));
      // left
      clearChannelROI<Type>(this,c,vals[c], Point(0,roi.top()),
                         Size(roi.left(),roi.height));
      // right
      clearChannelROI<Type>(this,c,vals[c], roi.ur(),
                         Size(s.width-roi.right(),roi.height) );
    }    
  }

  template<class Type>
  void Img<Type>::fillBorder(const ImgBase *src, bool setFullROI){
    ICLASSERT_RETURN( !hasFullROI() );
    ICLASSERT_RETURN( src );
    ICLASSERT_RETURN( getChannels() <= src->getChannels() );

    Rect roi = getROI();
    Size s = getSize();
    
    Point offs[4] = { 
      Point::null,              // top
      Point(0,roi.bottom()),    // bottom
      Point(0,roi.top()),       // left
      roi.ur()                  // right
    };
    Size size[4] = {                
      Size(s.width,roi.top()),               // top
      Size(s.width,s.height-roi.bottom()),   // bottom
      Size(roi.left(),roi.height),           // left
      Size(s.width-roi.right(),roi.height)   // right
    };
    
    Rect sroi = src->getROI();
    for(int i=0;i<4;++i){
      ICLASSERT_RETURN(sroi.contains(Rect(offs[i],size[i])));
    }

    switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                \
      case depth##D:                            \
      for(int c=0;c<getChannels();c++){         \
        for(int i=0;i<4;++i){                   \
          convertChannelROI<icl##D,Type>(src->asImg<icl##D>(),c,offs[i],size[i],this,c,offs[i],size[i]); \
        }                                       \
      }                                         \
      break;
ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
  }


  // }}} Global functions ..

  template<class Type>
  const Img<Type> Img<Type>::null;

  
  // {{{  explicit instantiation of the Img<T> classes 

#define ICL_INSTANTIATE_DEPTH(D) template class Img<icl##D>;
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

  // }}}

  template<class T>
  ImgBasePtrPtr<T>::~ImgBasePtrPtr(){
    // {{{ open
    if(!r){
      ERROR_LOG("Result image is NULL");
      //if(o) delete o; this is not allowed because o could be allocated on the stack
      return;
    }
    if(r && !o){
      ERROR_LOG("bpp() function got a NULL-pointer which is not allowed!");
      delete r;
      return;
    }
    /// r and o is given !!
    if(r != rbef ){
      ERROR_LOG("Detected a pointer reallocation in Implicit Img<T> to ImgBase** cast operation\n"
                "This warning implicates, that a local ImgBase* was reallocated instead of using\n"
                "the given Img<T>. To enshure a maximum compability, the new result images data\n"
                "will be converted interanlly into the given image. To avoid this warning and to\n"
                "enhance performance, DO NOT USE THE \"bpp(..)\"-FUNCTION\n");
      r->convert(o);
      delete r;
      return;
    }
    if(o->getParams() != r->getParams()){
      // shallow copy back
      ImgBase *poBase = o;
      
      r->shallowCopy(&poBase);
      //      *o = *r;
      delete r;
      return;
    }
    
    delete r;
  }

  // }}}

  template<class T>
  ImgBasePtrPtr<T>::ImgBasePtrPtr(Img<T> &i){
    r = new Img<T>(i);
    o = &i;
    rbef = r;
  }
  
  template<class T>
  ImgBasePtrPtr<T>::ImgBasePtrPtr(Img<T> *inputImage){
    // {{{ open
    ICLASSERT(inputImage != NULL);

    if(inputImage){
      r = new Img<T>(*inputImage);
      o = inputImage;
      rbef = r;
    }else{
      r = o = rbef = 0;
    }
  }

  // }}}



#define ICL_INSTANTIATE_DEPTH(D) template struct ImgBasePtrPtr<icl##D>;
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH


} //namespace icl

