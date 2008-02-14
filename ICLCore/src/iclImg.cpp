#include <iclImg.h>
#include <functional>

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

  // }}} 

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
      case 0: return Cast<float, Type>::cast (subPixelNN (fX, fY, iChannel));
      case 1: return Cast<float, Type>::cast (subPixelLIN (fX, fY, iChannel));
      default: 
        ERROR_LOG ("interpolation method not yet implemented!");
        return Cast<float, Type>::cast (subPixelLIN (fX, fY, iChannel));
    }
  }

  // }}}

  // }}} 

  // {{{  ensureCompatible<T>, and ensureDepth<T> utility templates
  //help function
  template<class Type> 
  inline Img<Type>* ensureCompatible (ImgBase** ppoDst, const ImgParams& p)
    // {{{ open
  {
    if (!ppoDst) return new Img<Type>(p);
    icl::ensureCompatible (ppoDst, icl::getDepth<Type>(), p);
    return (*ppoDst)->asImg<Type>();
  }
  // }}}

  //help function
  template<class Type>
  Img<Type>* ensureDepth(ImgBase **ppoDst){
    // {{{ open
    ImgBase *poDst = icl::ensureDepth(ppoDst,getDepth<Type>());
    return poDst->asImg<Type>();  
  }
  // }}}

  // }}}

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

  // }}}

  // }}}

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
  
  // }}}

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
      ICLASSERT_RETURN_VAL( poDst->getROISize() == getROISize(), NULL);
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
  
  // }}}
  
  // {{{  channel management: detach, append remove, swap,...

  template<class Type> void
  Img<Type>::detach(int iIndex){
    // {{{ open
    FUNCTION_LOG("index:" << iIndex );
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

  // }}}

  // {{{  inplace operations: scale, mirror

  //----------------------------------------------------------------------------
  template<class Type> void
  Img<Type>::scale(const Size &size, scalemode eScaleMode){  
    // {{{ open
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

        if (bInplace && oSize.height % 2) { // odd ROI height
          iRows++;
          e += oSize.width/2;
        }
        break;
    }
    eLine = iCols;

    return (iRows != 0 && iCols != 0);
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


#ifdef WITH_IPP_OPTIMIZATION
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

  // }}}
  
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

  // }}}

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
  Img<Type>::getMax(int iChannel, Point *coords) const {
    FUNCTION_LOG("iChannel: " << iChannel);
    ICLASSERT_RETURN_VAL( validChannel(iChannel), 0 );

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
  }
#ifdef WITH_IPP_OPTIMIZATION
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
  Img<Type>::getMin(int iChannel, Point *coords) const {
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
  }
#ifdef WITH_IPP_OPTIMIZATION
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

  // fallback for all types
  template<class Type> const Range<Type>
  Img<Type>::getMinMax(int iChannel, Point *minCoords, Point *maxCoords) const {
    FUNCTION_LOG("iChannel: " << iChannel);
    ICLASSERT_RETURN_VAL(validChannel(iChannel), Range<Type>());
    if(minCoords && !maxCoords || maxCoords && !minCoords){
      ERROR_LOG("please define minCoords AND maxCoords or do not define BOTH (returning 0)");
      return Range<Type>();
    }

    Range<Type> r;

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
        }else if(r.maxVal > v){
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
  }

#ifdef WITH_IPP_OPTIMIZATION
#define ICL_INSTANTIATE_DEPTH(T)                                                              \
template<> const Range<icl##T>                                                                \
Img<icl ## T>::getMinMax(int iChannel,Point *minCoords, Point *maxCoords) const {             \
   ICLASSERT_RETURN_VAL( validChannel(iChannel) ,Range<icl##T>());                            \
   if(minCoords && !maxCoords || maxCoords && !minCoords){                                    \
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

    // }}}

    // {{{  Auxillary  functions 

    template<class Type>
  SmartPtr<Type> Img<Type>::createChannel(Type *ptDataToCopy) const {
    // {{{ open
    FUNCTION_LOG("");
    int dim = getDim();
    if(!dim) return SmartPtr<Type>();

    Type *ptNewData = new Type[dim];
    if(ptDataToCopy){
      memcpy(ptNewData,ptDataToCopy,dim*sizeof(Type));
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
  float Img<Type>::subPixelRA(float fX, float fY, int iChannel) const {
    // {{{ open

    ERROR_LOG ("region average interpolation is not yet implemented!");
    return subPixelLIN (fX, fY, iChannel);
  }

  // }}}

  // }}}

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
  
  // }}}

  // {{{  normalize main methods

  template <class Type> void 
  Img<Type>::normalize(int iChannel, const Range<Type> &srcRange, const Range<Type> &dstRange){ 
    FUNCTION_LOG("");
    for(int c = getStartIndex(iChannel);c<getEndIndex(iChannel);c++){
      icl64f fScale  = (icl64f)(dstRange.getLength()) / (icl64f)(srcRange.getLength());
      icl64f fShift  = (icl64f)(srcRange.maxVal * dstRange.minVal - srcRange.minVal * dstRange.maxVal) / srcRange.getLength();
      for(iterator p=getROIIterator(c); p.inRegion(); ++p) {
        *p = Cast<icl64f,Type>::cast( icl::clip( fShift + (icl64f)(*p) * fScale, icl64f(dstRange.minVal),icl64f(dstRange.maxVal) ) );
      }
    }
  }

#ifdef WITH_IPP_OPTIMIZATION
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
     
  // }}}

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

  // scale channel ROI function for abitrary image scaling operations
  template<class T> 
  void scaledCopyChannelROI(const Img<T> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                            Img<T> *dst, int dstC, const Point &dstOffs, const Size &dstSize,
                            scalemode eScaleMode){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->validChannel(srcC) );
    ICLASSERT_RETURN( dst->validChannel(dstC) );
    ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0);
    ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() );
    ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );

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

    int xD = 0;
    int yD = 0;
    float yS = srcOffs.y + fSY * yD;
    for(; itDst.inRegion(); ++itDst) {
      *itDst = Cast<float, T>::cast ((src->*subPixelMethod)(srcOffs.x + fSX * xD, yS, srcC));
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
#ifdef WITH_IPP_OPTIMIZATION
  template<> inline void 
  scaledCopyChannelROI<icl8u>(const Img<icl8u> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                              Img<icl8u> *dst, int dstC, const Point &dstOffs, const Size &dstSize,
                              scalemode eScaleMode)
    // {{{ open 

  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->validChannel(srcC) );
    ICLASSERT_RETURN( dst->validChannel(dstC) );
    ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0);
    ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() );
    ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );
    
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
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->validChannel(srcC) );
    ICLASSERT_RETURN( dst->validChannel(dstC) );
    ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0);
    ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() );
    ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );
    
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
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( srcSize == dstSize );
    ICLASSERT_RETURN( src->validChannel(srcC) );
    ICLASSERT_RETURN( dst->validChannel(dstC) );
    ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0);
    ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() );
    ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );
  
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

#ifdef WITH_IPP_OPTIMIZATION
  /// IPP-OPTIMIZED specialization for icl8u image flipping
  template <>
  inline void flippedCopyChannelROI<icl8u>(axis eAxis, 
                                           const Img<icl8u> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                                           Img<icl8u> *dst, int dstC, const Point &dstOffs, const Size &dstSize) {
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( srcSize == dstSize );
    ICLASSERT_RETURN( src->validChannel(srcC) );
    ICLASSERT_RETURN( dst->validChannel(dstC) );
    ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0);
    ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() );
    ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );
   
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

    FUNCTION_LOG("");
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( srcSize == dstSize );
    ICLASSERT_RETURN( src->validChannel(srcC) );
    ICLASSERT_RETURN( dst->validChannel(dstC) );
    ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0);
    ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() );
    ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );
   
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




  // }}}

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
    
  // }}}

  // }}}

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
      if(o) delete o;
      return;
    }
    if(r && !o){
      ERROR_LOG("Result image is NULL");
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
      printf("convert: \n");
      r->convert(o);
      printf("del \n");
      delete r;
      printf("ret \n");
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
  ImgBasePtrPtr<T>::ImgBasePtrPtr(Img<T> *i){
    // {{{ open

    if(!i){ 
      o = 0; 
      r = 0;
      rbef = 0;
    }else{
      r = new Img<T>(*i);
      o = i;
      rbef = r;
    }
  }

  // }}}

#define ICL_INSTANTIATE_DEPTH(D) template struct ImgBasePtrPtr<icl##D>;
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH


} //namespace icl

