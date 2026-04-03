// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting, Robert Haschke

#include <ICLCore/Img.h>
#include <ICLCore/ImgOps.h>
#include <ICLCore/CoreFunctions.h>
#include <functional>
#include <limits>
#include <cstring>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::math;

namespace icl::core {
  //----------------------------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const ImgParams &params):

    ImgBase(icl::core::getDepth<Type>(),params){
    FUNCTION_LOG("Img(params)");

    for(int i=0;i<getChannels();i++) {
      m_vecChannels.push_back(createChannel());
    }
  }


  //----------------------------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const Size &s,int iChannels):

    ImgBase(icl::core::getDepth<Type>(),ImgParams(s,iChannels)){
    FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," << iChannels << ")  this:" << this );

    for(int i=0;i<getChannels();i++) {
      m_vecChannels.push_back(createChannel());
    }
  }


  //----------------------------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const Size& s, format eFormat):
    ImgBase(icl::core::getDepth<Type>(), ImgParams(s, eFormat)){

    for(int i=0;i<getChannels();i++) {
      m_vecChannels.push_back(createChannel());
    }
  }


  //----------------------------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const Size &s,int iChannels, format fmt):

    ImgBase(icl::core::getDepth<Type>(), ImgParams(s, iChannels, fmt)){
    for(int i=0;i<getChannels();i++) {
      m_vecChannels.push_back(createChannel());
    }
  }


  //----------------------------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const Size &s, int channels, const std::vector<Type*>& vptData, bool passOwnerShip) :

    ImgBase(icl::core::getDepth<Type>(),ImgParams(s,channels)) {
    ICLASSERT_THROW (getChannels () <= static_cast<int>(vptData.size()), InvalidImgParamException("channels"));
    FUNCTION_LOG("Img(" << s.width <<","<< s.height << "," <<  channels << ",Type**)  this:" << this);

    typename std::vector<Type*>::const_iterator it = vptData.begin();
    for(int i=0; i<getChannels(); ++i, ++it) {
      m_vecChannels.push_back(passOwnerShip ? std::shared_ptr<Type[]>(*it) : std::shared_ptr<Type[]>(*it, [](Type*){}));
    }
  }


  //----------------------------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const Size &s, int channels, format fmt, const std::vector<Type*>& vptData, bool passOwnerShip) :
    ImgBase(icl::core::getDepth<Type>(),ImgParams(s,channels,fmt)){
    ICLASSERT_THROW (getChannels () <= static_cast<int>(vptData.size()), InvalidImgParamException("channels"));

    typename std::vector<Type*>::const_iterator it = vptData.begin();
    for(int i=0; i<getChannels(); ++i, ++it) {
      m_vecChannels.push_back(passOwnerShip ? std::shared_ptr<Type[]>(*it) : std::shared_ptr<Type[]>(*it, [](Type*){}));
    }
  }


  //----------------------------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const Size &s, format eFormat, const std::vector<Type*>& vptData, bool passOwnerShip) :
    ImgBase(icl::core::getDepth<Type>(),ImgParams(s,eFormat)){
    ICLASSERT_THROW(getChannels() <= static_cast<int>(vptData.size()), InvalidImgParamException("channels"));

    typename std::vector<Type*>::const_iterator it = vptData.begin();
    for(int i=0; i<getChannels(); ++i, ++it) {
      m_vecChannels.push_back(passOwnerShip ? std::shared_ptr<Type[]>(*it) : std::shared_ptr<Type[]>(*it, [](Type*){}));
    }
  }


  //--- Initializer list constructors -------------------------------------------
  template<class Type>
  Img<Type>::Img(std::initializer_list<std::initializer_list<Type>> rows)
    : Img({rows}) {}

  template<class Type>
  Img<Type>::Img(std::initializer_list<std::initializer_list<std::initializer_list<Type>>> channels)
    : ImgBase(icl::core::getDepth<Type>(), ImgParams::null)
  {
    int nCh = static_cast<int>(channels.size());
    ICLASSERT_THROW(nCh > 0, InvalidImgParamException("empty initializer list"));

    auto chIt = channels.begin();
    int h = static_cast<int>(chIt->size());
    ICLASSERT_THROW(h > 0, InvalidImgParamException("empty channel in initializer list"));
    int w = static_cast<int>(chIt->begin()->size());
    ICLASSERT_THROW(w > 0, InvalidImgParamException("empty row in initializer list"));

    m_oParams = ImgParams(Size(w, h), nCh);

    for(const auto &ch : channels) {
      ICLASSERT_THROW(static_cast<int>(ch.size()) == h,
        InvalidImgParamException("inconsistent channel heights in initializer list"));
      m_vecChannels.push_back(std::shared_ptr<Type[]>(new Type[w * h]));
      Type *base = m_vecChannels.back().get();
      int y = 0;
      for(const auto &row : ch) {
        ICLASSERT_THROW(static_cast<int>(row.size()) == w,
          InvalidImgParamException("inconsistent row lengths in initializer list"));
        Type *dst = base + y * w;
        int x = 0;
        for(const auto &val : row) {
          dst[x++] = val;
        }
        ++y;
      }
    }
  }

  //--- Element-wise equality ---------------------------------------------------
  namespace {
    template<class T>
    bool pixels_equal(T a, T b) { return a == b; }

    template<> bool pixels_equal<icl32f>(icl32f a, icl32f b) {
      // 4 * machine epsilon (~4.8e-7) — tolerates a few ULPs of rounding
      return std::abs(a - b) <= 4 * std::numeric_limits<icl32f>::epsilon();
    }
    template<> bool pixels_equal<icl64f>(icl64f a, icl64f b) {
      return std::abs(a - b) <= 4 * std::numeric_limits<icl64f>::epsilon();
    }
  }

  template<class Type>
  bool Img<Type>::operator==(const Img<Type> &other) const {
    if(getSize() != other.getSize()) return false;
    if(getChannels() != other.getChannels()) return false;
    for(int c = 0; c < getChannels(); ++c) {
      const Type *a = getROIData(c);
      const Type *b = other.getROIData(c);
      int ls_a = getLineStep() / sizeof(Type);
      int ls_b = other.getLineStep() / sizeof(Type);
      int w = getROISize().width;
      int h = getROISize().height;
      for(int y = 0; y < h; ++y) {
        for(int x = 0; x < w; ++x) {
          if(!pixels_equal(a[x], b[x])) return false;
        }
        a += ls_a;
        b += ls_b;
      }
    }
    return true;
  }

  //--- Copy constructor -------------------------------------------------------
  template<class Type>
  Img<Type>::Img(const Img<Type>& tSrc) :

    ImgBase(tSrc.getDepth(),tSrc.getParams())
  {
    FUNCTION_LOG("this: " << this);
    m_vecChannels = tSrc.m_vecChannels;
    setTime(tSrc.getTime());
    setMetaData(tSrc.getMetaData());
  }


  //----------------------------------------------------------------------------
  template<class Type>
  Img<Type>::~Img()
  {
    FUNCTION_LOG("this: " << this);
  }


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
                 const DynMatrix<Type> &c5):
    ImgBase(icl::core::getDepth<Type>(),ImgParams(Size(c1.cols(),c1.rows()),get_channel_count(c1,c2,c3,c4,c5))){

    if(c1.isNull()) return;
    m_vecChannels.reserve(getChannels());
    m_vecChannels.push_back(std::shared_ptr<Type[]>(const_cast<Type*>(c1.begin()), [](Type*){}));
#define ADD_CHANNEL(i)                                                  \
    if(!c##i.isNull()){                                                 \
      ICLASSERT_THROW(c1.cols() == c##i.cols() && c1.rows() == c##i.rows(), InvalidMatrixDimensionException(__FUNCTION__)); \
      m_vecChannels.push_back(std::shared_ptr<Type[]>(const_cast<Type*>(c##i.begin()), [](Type*){})); \
    }
    ADD_CHANNEL(2)    ADD_CHANNEL(3)    ADD_CHANNEL(4)    ADD_CHANNEL(5)
#undef ADD_CHANNEL


    }





  template<class Type>
  Img<Type>& Img<Type>::shallowCopy(const Img<Type>& tSrc)
  {

    FUNCTION_LOG("");

    //---- Assign new channels to Img ----
    m_oParams = tSrc.getParams ();
    m_vecChannels = tSrc.m_vecChannels;

    //take over timestamp
    this->setTime(tSrc.getTime());
    this->setMetaData(tSrc.getMetaData());
    return *this;
  }


  template<class Type>
  Type Img<Type>::operator()(float fX, float fY, int iChannel, scalemode eScaleMode) const {

    switch(eScaleMode) {
      case interpolateNN: return clipped_cast<float, Type>(subPixelNN (fX, fY, iChannel));
      case interpolateLIN: return clipped_cast<float, Type>(subPixelLIN (fX, fY, iChannel));
      default:
        ERROR_LOG ("interpolation method not yet implemented!");
        return clipped_cast<float, Type>(subPixelLIN (fX, fY, iChannel));
    }
  }




  template<class Type>
  inline Img<Type>* ensureCompatible (ImgBase** ppoDst, const ImgParams& p)
  {
    if (!ppoDst) return new Img<Type>(p);
    icl::core::ensureCompatible (ppoDst, icl::core::getDepth<Type>(), p);
    return (*ppoDst)->asImg<Type>();
  }

  template<class Type>
  Img<Type>* ensureDepth(ImgBase **ppoDst){
    ImgBase *poDst = icl::core::ensureDepth(ppoDst,getDepth<Type>());
    return poDst->asImg<Type>();
  }




  template<class Type>
  Img<Type> *Img<Type>::shallowCopy(const Rect &roi,
                                    const std::vector<int> &channelIndices,
                                    format fmt,
                                    Time t,
                                    ImgBase **ppoDst){
    ImgParams p(this->getSize(), 0);
    Img<Type> *poDst = ensureCompatible<Type>(ppoDst,p);
    /// Die ROi wird nicht übernommen ???
    *poDst = *this;
    if(roi != Rect::null){
      poDst->setROI(roi);
    }
    if(channelIndices.size()){
      ICLASSERT_RETURN_VAL(fmt == formatMatrix || static_cast<int>(channelIndices.size()) == getChannelsOfFormat(fmt),0);
      poDst->setChannels(0);
      poDst->append(this,channelIndices);
    }
    poDst->setFormat(fmt);
    poDst->setTime(t);
    poDst->setMetaData(getMetaData());
    return poDst;
  }





  template<class Type>
  Img<Type> *Img<Type>::deepCopy(ImgBase **ppoDst) const{

    FUNCTION_LOG("ptr:"<<ppoDst);
    return deepCopy( ensureCompatible<Type>(ppoDst,getParams()) );
  }


  template<class Type>
  Img<Type> *Img<Type>::scaledCopy( ImgBase **ppoDst, scalemode eScaleMode) const{

    FUNCTION_LOG("ptr:"<<ppoDst);
    if(!ppoDst) return deepCopy();
    return scaledCopy( ensureDepth<Type>(ppoDst), eScaleMode );
  }


  template<class Type>
  Img<Type> *Img<Type>::scaledCopy( const Size &newSize, scalemode eScaleMode) const{

    FUNCTION_LOG("new size:"<<newSize.width<<"x"<<newSize.height);
    return scaledCopy( new Img<Type>(newSize, getChannels(), getFormat()), eScaleMode);
  }


  template<class Type>
  Img<Type> *Img<Type>::deepCopyROI(ImgBase **ppoDst) const{
    FUNCTION_LOG("ptr:"<<ppoDst);
    Img<Type> *tmp = ensureDepth<Type>(ppoDst);
    if(!ppoDst){
      tmp->setSize(getROISize());
      tmp->setFormat(getFormat());
      tmp->setChannels(getChannels());
    }
    return deepCopyROI( tmp );
  }


  template<class Type>
  Img<Type> *Img<Type>::scaledCopyROI(const Size &newSize, scalemode eScaleMode) const{

    FUNCTION_LOG("new size:"<<newSize.width<<"x"<<newSize.height);
    return scaledCopyROI( new Img<Type>(newSize, getChannels(),getFormat()), eScaleMode );
  }


  template<class Type>
  Img<Type> *Img<Type>::scaledCopyROI(ImgBase **ppoDst, scalemode eScaleMode) const{

    FUNCTION_LOG("ptr:"<<ppoDst);
    if(!ppoDst) return deepCopyROI();
    return scaledCopyROI( ensureDepth<Type>(ppoDst),eScaleMode );
  }




  template<class Type>
  Img<Type> *Img<Type>::deepCopy(Img<Type> *poDst) const{

    FUNCTION_LOG("ptr:"<<poDst);
    if(!poDst) poDst = new Img<Type>(getParams());
    else poDst->setParams(getParams());
    poDst->setTime(getTime());
    poDst->setMetaData(getMetaData());

    for(int c=getChannels()-1; c>=0; --c){
      deepCopyChannel(this,c,poDst,c);
    }
    return poDst;

  }


  template<class Type>
  Img<Type> *Img<Type>::scaledCopy(Img<Type> *poDst, scalemode eScaleMode) const{

    FUNCTION_LOG("ptr:"<<poDst);
    if(!poDst) return deepCopy( (ImgBase**)0 );
    poDst->setChannels(getChannels());
    poDst->setFormat(getFormat());
    poDst->setTime(getTime());
    poDst->setMetaData(getMetaData());
    //    poDst->setFullROI();
    for(int c=getChannels()-1; c>=0; --c){
      scaledCopyChannelROI(this,c,Point::null,getSize(),poDst,c,Point::null,poDst->getSize(),eScaleMode);
    }
    return poDst;
  }


  template<class Type>
  Img<Type> *Img<Type>::deepCopyROI(Img<Type> *poDst) const{
    // NEW USING source ROI as well as destination images ROI
    FUNCTION_LOG("ptr:"<< poDst);
    if(!poDst){
      poDst = new Img<Type>(getROISize(),getChannels(),getFormat());
    }else{
      if(poDst->getSize().getDim()){
        ICLASSERT_RETURN_VAL( poDst->getROISize() == getROISize(), nullptr);
      }else{
        poDst->setSize(getROISize());
      }
      poDst->setChannels(getChannels());
      poDst->setFormat(getFormat());
    }
    poDst->setTime(getTime());
    poDst->setMetaData(getMetaData());
    for(int c=getChannels()-1; c>=0; --c){
      deepCopyChannelROI(this,c, getROIOffset(), getROISize(),
                         poDst,c, poDst->getROIOffset(), poDst->getROISize() );
    }
    return poDst;

  }


  template<class Type>
  Img<Type> *Img<Type>::scaledCopyROI(Img<Type> *poDst, scalemode eScaleMode) const{

    FUNCTION_LOG("ptr:"<<poDst);
    if(!poDst) return deepCopyROI();
    poDst->setChannels(getChannels());
    poDst->setFormat(getFormat());
    poDst->setTime(getTime());
    poDst->setMetaData(getMetaData());

    for(int c=getChannels()-1; c>=0; --c){
      scaledCopyChannelROI(this,c, getROIOffset(), getROISize(),
                           poDst,c, poDst->getROIOffset(), poDst->getROISize() , eScaleMode);
    }
    return poDst;
  }





  template<class Type> void
  Img<Type>::detach(int iIndex){
    FUNCTION_LOG("index:" << iIndex );
    ICLASSERT_RETURN(iIndex < getChannels());

    //---- Make the whole Img independent ----
    for(int i=getStartIndex(iIndex),iEnd=getEndIndex(iIndex);i<iEnd;i++){
      if(m_vecChannels[i].use_count() > 1){
        m_vecChannels[i] = createChannel (getData(i));
      }
    }
  }


  //----------------------------------------------------------------------------
  template<class Type> void
  Img<Type>::removeChannel(int iChannel)

  {
    FUNCTION_LOG("removeChannel(" << iChannel << ")");
    ICLASSERT_RETURN(validChannel(iChannel));

    m_vecChannels.erase(m_vecChannels.begin()+iChannel);
    m_oParams.setChannels(m_vecChannels.size());
  }


  //----------------------------------------------------------------------------
  template<class Type> void
  Img<Type>::append(const Img<Type> *poSrc, int iIndex)

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


  template<class Type> void
  Img<Type>::append(const Img<Type> *poSrc, const std::vector<int>& vChannels)

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


  //----------------------------------------------------------------------------
  template<class Type> void
  Img<Type>::swapChannels(int iIndexA, int iIndexB)
  {
    FUNCTION_LOG("swapChannels("<<iIndexA<<","<< iIndexB<< ")");
    ICLASSERT_RETURN(validChannel(iIndexA));
    ICLASSERT_RETURN(validChannel(iIndexB));

    std::swap(m_vecChannels[iIndexA], m_vecChannels[iIndexB]);
  }


  //----------------------------------------------------------------------------
  template<class Type> inline void
  Img<Type>::replaceChannel(int iThisIndex, Img<Type>* poSrc, int iOtherIndex)
  {
    FUNCTION_LOG("");
    ICLASSERT_RETURN(validChannel(iThisIndex));
    ICLASSERT_RETURN(poSrc->validChannel(iOtherIndex));
    m_vecChannels[iThisIndex] = poSrc->m_vecChannels[iOtherIndex];
  }

  template<class Type>
  Img<Type> Img<Type>::extractChannelImg(int index){
    Img<Type> other(getSize(),0);
    other.append(this,index);
    other.setROI(getROI());
    other.setTime(getTime());
    other.setMetaData(getMetaData());
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
    other.setMetaData(getMetaData());
    other.setFormat(formatMatrix);
    return other;
  }

  template<class Type>
  const Img<Type> Img<Type>::extractChannelImg(const std::vector<int> &indices) const{
    return const_cast<Img<Type>*>(this)->extractChannelImg(indices);
  }




  //----------------------------------------------------------------------------

  template<class Type>
  Img<Type> *Img<Type>::lut(const Type *lut,Img<Type> *dst, int bits) const{
    if(!dst){
      dst = new Img<Type>(getSize(),getChannels(),getFormat());
      dst->setROI(getROI());
    }else{
      dst->setSize(getSize());
      dst->setChannels(getChannels());
      dst->setFormat(getFormat());
      if(dst->getROISize() != getROISize()) throw ICLException("Img<T>::lut source and destination ROI sizes differ");
    }
    dst->setTime(getTime());
    dst->setMetaData(getMetaData());

    ImgBase* self = const_cast<ImgBase*>(static_cast<const ImgBase*>(this));
    auto& sel = ImgOps::instance().getSelector<ImgOps::LutSig>(ImgOps::Op::lut);
    sel.resolveOrThrow(self)->apply(*self, static_cast<const void*>(lut), *dst, bits);
    return dst;
  }

  template<class Type> void
  Img<Type>::scale(const Size &size, scalemode eScaleMode){
    FUNCTION_LOG("");
    ICLASSERT_RETURN ((size.width > 0) && (size.height > 0));

    if (!isEqual(size,getChannels()))
      {
        Img<Type> oTmp(size,getChannels(),getFormat());
        scaledCopy(&oTmp,eScaleMode);
        (*this)=oTmp;
      }
  }


  //----------------------------------------------------------------------------
  template<class Type> void
  Img<Type>::mirror(axis eAxis, bool bOnlyROI)
  {
    FUNCTION_LOG("");
    auto& sel = ImgOps::instance().getSelector<ImgOps::MirrorSig>(ImgOps::Op::mirror);
    sel.resolveOrThrow(this)->apply(*this, eAxis, bOnlyROI);
  }

  // Mirror helpers (getPointerOffset, getMirrorPointerOffsets, getMirrorPointers)
  // and Img<T>::mirror(axis, int, Point, Size) moved to Img_Cpp.cpp






  //----------------------------------------------------------------------------
  template<class Type> void
  Img<Type>::setSize(const Size &s)

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


  //----------------------------------------------------------------------------
  template<class Type> void
  Img<Type>::setChannels(int iNumNewChannels)
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





  template<class Type> Type
  Img<Type>::getMax() const{
    FUNCTION_LOG("");

    if (getChannels() == 0) return 0;
    Type tMax = getMax(0);
    for(int i=1;i<getChannels();i++)
      tMax = iclMax(tMax,getMax(i));
    return tMax;
  }

  template<class Type> Type
  Img<Type>::getMax(int channel, Point *coords) const {
    FUNCTION_LOG("iChannel: " << channel);
    ICLASSERT_RETURN_VAL( validChannel(channel), 0 );
    ICLASSERT_RETURN_VAL( getROISize().getDim(), 0 );
    ImgBase* self = const_cast<ImgBase*>(static_cast<const ImgBase*>(this));
    auto& sel = ImgOps::instance().getSelector<ImgOps::GetMaxSig>(ImgOps::Op::getMax);
    return clipped_cast<icl64f, Type>(sel.resolveOrThrow(self)->apply(*self, channel, coords));
  }



  template<class Type> Type
  Img<Type>::getMin() const{
    FUNCTION_LOG("");

    if (getChannels() == 0) return 0;
    Type tMin = getMin(0);
    for(int i=1;i<getChannels();i++)
      tMin = iclMin(tMin,getMin(i));
    return tMin;
  }

  template<class Type> Type
  Img<Type>::getMin(int channel, Point *coords) const {
    FUNCTION_LOG("iChannel: " << channel);
    ICLASSERT_RETURN_VAL( validChannel(channel), 0 );
    ICLASSERT_RETURN_VAL( getROISize().getDim(), 0 );
    ImgBase* self = const_cast<ImgBase*>(static_cast<const ImgBase*>(this));
    auto& sel = ImgOps::instance().getSelector<ImgOps::GetMinSig>(ImgOps::Op::getMin);
    return clipped_cast<icl64f, Type>(sel.resolveOrThrow(self)->apply(*self, channel, coords));
  }



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

  // get_min_and_max_element_util moved to Img_Cpp.cpp

  template<class Type> const Range<Type>
  Img<Type>::getMinMax(int channel, Point *minCoords, Point *maxCoords) const {
    ICLASSERT_RETURN_VAL( validChannel(channel), Range<Type>() );
    ICLASSERT_RETURN_VAL( getROISize().getDim(), Range<Type>() );
    if((minCoords && !maxCoords) || (maxCoords && !minCoords)){
      ERROR_LOG("please define minCoords AND maxCoords or do not define BOTH (returning (0,0))");
      return Range<Type>(0,0);
    }
    ImgBase* self = const_cast<ImgBase*>(static_cast<const ImgBase*>(this));
    auto& sel = ImgOps::instance().getSelector<ImgOps::GetMinMaxSig>(ImgOps::Op::getMinMax);
    icl64f mn, mx;
    sel.resolveOrThrow(self)->apply(*self, channel, &mn, &mx, minCoords, maxCoords);
    return Range<Type>(clipped_cast<icl64f, Type>(mn), clipped_cast<icl64f, Type>(mx));
  }





  template<class Type>
  std::shared_ptr<Type[]> Img<Type>::createChannel(Type *ptDataToCopy) const {
    FUNCTION_LOG("");
    int dim = getDim();
    if(!dim) return std::shared_ptr<Type[]>();

    Type *ptNewData = new Type[dim];
    if(ptDataToCopy){
      memcpy(ptNewData,ptDataToCopy,dim*sizeof(Type));
    }else{
      std::fill(ptNewData,ptNewData+dim,0);
    }
    return std::shared_ptr<Type[]>(ptNewData);
  }


  // sub-pixel access using linear interpolation
  template<class Type>
  float Img<Type>::subPixelLIN(float fX, float fY, int iChannel) const {

    float fX0 = fX - floor(fX), fX1 = 1.0 - fX0;
    float fY0 = fY - floor(fY), fY1 = 1.0 - fY0;
    int xll = static_cast<int>(fX);
    int yll = static_cast<int>(fY);

    const Type* pLL = getData(iChannel) + xll + yll * getWidth();
    float a = *pLL;        //  a b
    float b = *(++pLL);    //  c d
    pLL += getWidth();
    float d = *pLL;
    float c = *(--pLL);

    // return fX1*fY1*a + fX0*fY1*b + fX0*fY0*d + fX1*fY0*c;
    return fX1 * (fY1*a + fY0*c) + fX0 * (fY1*b + fY0*d);
  }


  // sub-pixel access using region average interpolation
  template<class Type>
  float Img<Type>::subPixelRA(float fX, float fY, float w, float h, int iChannel) const{
    unsigned int xB = floorf(fX);
    unsigned int xE = ceilf((fX+w)-1);
    unsigned int yB = floorf(fY);
    unsigned int yE = ceilf((fY+h)-1);

    Rect r(xB, yB, xE-xB, yE-yB);
    if(!(getImageRect().contains(r))){
      ERROR_LOG("given rect extends image bounds");
      return 0;
    }

    // factor for the first and last column
    float xBMul = 1.0f - (fX-xB);
    float xEMul = 1.0f - (xE-(fX+w-1));
    // factor for the first and last row
    float yBMul = 1.0f - (fY-yB);
    float yEMul = 1.0f - (yE-(fY+h-1));

    const Type *d = getData(iChannel);
    unsigned int width = getWidth();
    float sum = 0.0f;

    // sum of the first row
    sum += (*(d + xB + yB*width))*xBMul;
    for (unsigned int x = xB+1; x < xE; ++x) {
            sum += (*(d + x + yB*width));
    }
    sum = (sum + (*(d + xE + yB*width))*xEMul) * yBMul;

    for (unsigned int y = yB+1; y < yE; ++y) {
            sum += (*(d + xB + y*width))*xBMul;
            for (unsigned int x = xB+1; x < xE; ++x) {
                    sum += *(d + x + y*width);
            }
            sum += (*(d + xE + y*width))*xEMul;
    }

    // sum of the last row
    float psum = (*(d + xB + yE*width))*xBMul;
    for (unsigned int x = xB+1; x < xE; ++x) {
            psum += (*(d + x + yE*width));
    }
    sum += ((psum + (*(d + xE + yE*width))*xEMul) * yEMul);

    return sum / (w*h);
  }


  template<class Type>
  float Img<Type>::subPixelRA(const unsigned int xB, const unsigned int xE,
                    const unsigned int yB, const unsigned int yE,
                    const float xBMul, const float xEMul,
                    const float yBMul, const float yEMul,
                    const Type *d, const unsigned int w) const {
      float sum = 0.0f;

      // sum of the first row
      sum += (*(d + xB + yB*w))*xBMul;
      for (unsigned int x = xB+1; x < xE; ++x) {
              sum += (*(d + x + yB*w));
      }
      sum = (sum + (*(d + xE + yB*w))*xEMul) * yBMul;

      for (unsigned int y = yB+1; y < yE; ++y) {
              sum += (*(d + xB + y*w))*xBMul;
              for (unsigned int x = xB+1; x < xE; ++x) {
                      sum += *(d + x + y*w);
              }
              sum += (*(d + xE + y*w))*xEMul;
      }

      // sum of the last row
      float psum = (*(d + xB + yE*w))*xBMul;
      for (unsigned int x = xB+1; x < xE; ++x) {
              psum += (*(d + x + yE*w));
      }
      sum += ((psum + (*(d + xE + yE*w))*xEMul) * yEMul);

      return sum;
  }





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



  template <class Type> void
  Img<Type>::normalize(int iChannel, const Range<Type> &srcRange, const Range<Type> &dstRange){
    FUNCTION_LOG("");
    auto& sel = ImgOps::instance().getSelector<ImgOps::NormalizeSig>(ImgOps::Op::normalize);
    sel.resolveOrThrow(this)->apply(*this, iChannel,
      static_cast<icl64f>(srcRange.minVal), static_cast<icl64f>(srcRange.maxVal),
      static_cast<icl64f>(dstRange.minVal), static_cast<icl64f>(dstRange.maxVal));
  }


  template<class Type>
  void Img<Type>::clear(int iIndex, Type tValue, bool bROIOnly)
  {
    //---- Log Message ----
    FUNCTION_LOG("clear(" << iIndex << "," << tValue << ")");
    ICLASSERT_RETURN( iIndex < getChannels() );

    Point offs = bROIOnly ? getROIOffset() : Point::null;
    Size size = bROIOnly ? getROISize() : getSize();
    auto& sel = ImgOps::instance().getSelector<ImgOps::ClearChannelROISig>(ImgOps::Op::clearChannelROI);
    for(int i=getStartIndex(iIndex),iEnd=getEndIndex(iIndex);i<iEnd;i++){
      sel.resolveOrThrow(this)->apply(*this, i, static_cast<icl64f>(tValue), offs, size);
    }
  }




  template<class ImgType>
  const ImgType* combineImages (const std::vector<const ImgType*>& vec, ImgBase** ppoDst) {

    FUNCTION_LOG("");
    // find first non-zero element
    typename std::vector<const ImgType*>::const_iterator
    first = std::find_if (vec.begin(), vec.end(),
                          [](const ImgType *p){ return p != nullptr; }),
    end   = vec.end();
    // check for empty vector
    if (first == vec.end()) return 0;

    // create image with parameters of first image in vector
    icl::core::ensureCompatible (ppoDst, *first);
    ImgType* poDst = static_cast<ImgType*>(*ppoDst);
    // some cache variables
    const Size& dstSize = poDst->getSize();
    const Rect& dstROI  = poDst->getROI();
    icl::core::format fmt = (*first)->getFormat();
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


  // file local (i.e. private) function to mediate between ImgBase and Img<T> variants
  template<typename T>
  const Img<T>* __combineImages (const std::vector<const ImgBase*>& vec, ImgBase** ppoDst) {

    std::vector<const Img<T>*> vecTyped;
    // create correctly typed vector
    for (std::vector<const ImgBase*>::const_iterator it=vec.begin(), end=vec.end();
         it != end; ++it) {
      const ImgBase *pImgBase = *it;
      if (pImgBase->getDepth () == icl::core::getDepth<T>())
        vecTyped.push_back (pImgBase->asImg<T>());
      else ERROR_LOG ("image depth doesn't match");
    }
    return combineImages (vecTyped, ppoDst);
  }


  template<>
  const ImgBase* combineImages<ImgBase> (const std::vector<const ImgBase*>& vec, ImgBase** ppoDst) {

    FUNCTION_LOG("ImgBase");
    // find first non-zero element
    std::vector<const ImgBase*>::const_iterator
    first = std::find_if (vec.begin(), vec.end(),
                          [](const ImgBase *p){ return p != nullptr; });
    // check for empty vector
    if (first == vec.end()) {
      // remove all channels from *ppoDst
      if (ppoDst && *ppoDst) {(*ppoDst)->setChannels(0); return *ppoDst;}
      // or return Null pointer directly
      else return 0;
    }
    switch ((*first)->getDepth()) {
#define ICL_INSTANTIATE_DEPTH(T)                                        \
      case depth ## T: return __combineImages<icl ## T> (vec, ppoDst); break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
      }
    return 0;
  }



#define CHECK_VALUES(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize) \
  FUNCTION_LOG("");                                                     \
  ICLASSERT_RETURN( src && dst );                                       \
  ICLASSERT_RETURN( srcSize == dstSize );                               \
  ICLASSERT_RETURN( src->validChannel(srcC) );                          \
  ICLASSERT_RETURN( dst->validChannel(dstC) );                          \
  ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0); \
  ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() ); \
  ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );

#define CHECK_VALUES_NO_SIZE(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize) \
  FUNCTION_LOG("");                                                     \
  ICLASSERT_RETURN( src && dst );                                       \
  ICLASSERT_RETURN( src->validChannel(srcC) );                          \
  ICLASSERT_RETURN( dst->validChannel(dstC) );                          \
  ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0); \
  ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() ); \
  ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );

  // Dispatch scaled copy through ImgOps backend (C++ / Accelerate / IPP).
  template<class T>
  void scaledCopyChannelROI(const Img<T> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                            Img<T> *dst, int dstC, const Point &dstOffs, const Size &dstSize,
                            scalemode eScaleMode){
    CHECK_VALUES_NO_SIZE(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize);
    static auto *impl = ImgOps::instance()
        .getSelector<ImgOps::ScaledCopySig>(ImgOps::Op::scaledCopy)
        .resolveOrThrow();
    impl->apply(*src, srcC, srcOffs, srcSize,
                 *dst, dstC, dstOffs, dstSize, eScaleMode);
  }


#define ICL_INSTANTIATE_DEPTH(D)  template ICLCore_API void scaledCopyChannelROI<icl##D> \
  (const Img<icl##D>*,int,const Point&,const Size&,                     \
   Img<icl##D>*,int,const Point&,const Size&,scalemode);

  // Scaling now dispatched via ImgOps (C++ / Accelerate / IPP backends).
  // C++ backend in Img_Cpp.cpp, Accelerate backend in Img_Accelerate.cpp.
  ICL_INSTANTIATE_ALL_DEPTHS

#undef ICL_INSTANTIATE_DEPTH




  // Dispatch wrapper — C++ implementation in Img_Cpp.cpp, IPP in Img_Ipp.cpp
  template<class T>
  void flippedCopyChannelROI(axis eAxis,
                             const Img<T> *src, int srcC, const Point &srcOffs, const Size &srcSize,
                             Img<T> *dst, int dstC, const Point &dstOffs, const Size &dstSize){
    FUNCTION_LOG("");
    CHECK_VALUES(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize);
    ImgBase* srcPtr = const_cast<ImgBase*>(static_cast<const ImgBase*>(src));
    auto& sel = ImgOps::instance().getSelector<ImgOps::FlippedCopySig>(ImgOps::Op::flippedCopy);
    sel.resolveOrThrow(static_cast<ImgBase*>(dst))
      ->apply(eAxis, *srcPtr, srcC, srcOffs, srcSize,
              *static_cast<ImgBase*>(dst), dstC, dstOffs, dstSize);
  }

#define ICL_INSTANTIATE_DEPTH(D) template ICLCore_API void flippedCopyChannelROI<icl##D>(axis eAxis, \
                        const Img<icl##D> *src, int srcC, const Point &srcOffs, const Size &srcSize, \
                        Img<icl##D> *dst, int dstC, const Point &dstOffs, const Size &dstSize);
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH






  void flippedCopy(axis eAxis, const ImgBase *poSrc, ImgBase **ppoDst){

    ICLASSERT_RETURN(poSrc);

  //    SHOW(*poSrc);
    //if(ppoDst) SHOW(*ppoDst);

    ImgBase *poDst = ensureCompatible(ppoDst,poSrc->getDepth(),poSrc->getSize(),poSrc->getChannels(),poSrc->getFormat());
  //SHOW(poDst);
  // SHOW(*poDst);

    poDst->setTime(poSrc->getTime());
    poDst->setMetaData(poSrc->getMetaData());



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
    delete poFullSrc;
    delete poFullDst;
  }


  void flippedCopyROI(axis eAxis, const ImgBase *poSrc, ImgBase **ppoDst){

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
    poDst->setMetaData(poSrc->getMetaData());
    for(int c=poSrc->getChannels()-1; c>=0; --c){
      switch(poSrc->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
        case depth##D :flippedCopyChannelROI(eAxis,poSrc->asImg<icl##D>(),c, poSrc->getROIOffset(), poSrc->getROISize(), \
                                             poDst->asImg<icl##D>(),c, poDst->getROIOffset(), poDst->getROISize() ); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }
  }



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
            printf(fmtFull.c_str(),static_cast<icl64f>((*this)(x,y,i)),(r.contains(x,y)?'r':' '));
          }else{
            printf(fmtFull.c_str(),static_cast<icl64f>((*this)(x,y,i)));
          }
        }
        std::cout << "|" << std::endl;
      }
      std::cout << "--------------------" << std::endl;
    }
  }





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
    int offs = static_cast<int>(p-getData(channel));
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
      //      if(roi.bottom()<im.bottom()){
      //  srcOffs.y = roi.bottom();
      //  for(Point p(srcOffs.x,roi.bottom()+1);p.y<im.height;p.y++){
      //    deepCopyChannelROI(this,c, srcOffs, srcDstSize, this,c,p,srcDstSize);
      //  }
      //}
      // bottom
      if(roi.bottom()<im.bottom()){
        srcOffs.y = roi.bottom()-1;
        for(Point p(srcOffs.x,roi.bottom());p.y<im.height;p.y++){
          deepCopyChannelROI(this,c, srcOffs, srcDstSize,this,c,p,srcDstSize);
        }
      }

    }
    if(setFullROI) this->setFullROI();
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
    if(setFullROI) this->setFullROI();
  }

  template<class Type>
  void Img<Type>::fillBorder(const std::vector<icl64f> &vals, bool setFullROI){
    ICLASSERT_RETURN( !hasFullROI() );
    ICLASSERT_RETURN(static_cast<int>(vals.size()) >= getChannels());
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
    if(setFullROI) this->setFullROI();
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
#define ICL_INSTANTIATE_DEPTH(D)                                        \
      case depth##D:                                                    \
        for(int c=0;c<getChannels();c++){                               \
          for(int i=0;i<4;++i){                                         \
            convertChannelROI<icl##D,Type>(src->asImg<icl##D>(),c,offs[i],size[i],this,c,offs[i],size[i]); \
          }                                                             \
        }                                                               \
        break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
      }

    if(setFullROI) this->setFullROI();
  }




  template<class Type>
  void Img<Type>::extractChannels(Channel<Type> *dst){
    ICLASSERT_RETURN(dst);
    for(int i=0;i<getChannels();++i){
      dst[i] = (*this)[i];
    }
  }

  template<class Type>
  void Img<Type>::extractPointers(Type **dst){
    for(int i=0;i<getChannels();++i){
      dst[i] = begin(i);
    }
  }

  template<class Type>
  void Img<Type>::extractPointers(const Type **dst) const{
    for(int i=0;i<getChannels();++i){
      dst[i] = begin(i);
    }
  }

  template<class Type>
  Img<Type> *Img<Type>::reinterpretChannels(format newFmt, Img<Type> *poDst){
    ImgBase *poDstBase = poDst;
    return shallowCopy(getROI(),std::vector<int>(),newFmt,getTime(),&poDstBase);
  }

  template<class Type>
  const Img<Type> *Img<Type>::reinterpretChannels(format newFmt){
    return shallowCopy(getROI(),std::vector<int>(),newFmt,getTime());
  }

  template<class Type>
  Img<Type>* Img<Type>::shallowCopy(const Rect &roi, Img<Type>* poDst){
    ImgBase *poDstBase = poDst;
    return shallowCopy(roi,std::vector<int>(),getFormat(),getTime(),&poDstBase);
  }

  template<class Type>
  Img<Type>* Img<Type>::selectChannels(const std::vector<int>& channelIndices, Img<Type>* poDst){
    ImgBase *poDstBase = poDst;
    return shallowCopy(getROI(),channelIndices,formatMatrix,getTime(),&poDstBase);
  }

  template<class Type>
  Img<Type> *Img<Type>::selectChannel(int channelIndex, Img<Type> *poDst){
    ICLASSERT_RETURN_VAL(validChannel(channelIndex), 0);
    std::vector<int> v(1); v[0]= channelIndex;
    return selectChannels(v,poDst);
  }

  template<class Type>
  Img<Type> Img<Type>::detached() const {
    Img<Type> detachedCopy = *this;
    detachedCopy.detach();
    return detachedCopy;
  }

  template<class Type>
  void Img<Type>::append(Img<Type> *src, int iChannel) {
    // call private const-version
    this->append(static_cast<const Img<Type>*>(src), iChannel);
  }

  template<class Type>
  void Img<Type>::append(Img<Type> *src, const std::vector<int>& vChannels) {
    // call private const-version
    this->append(static_cast<const Img<Type>*>(src), vChannels);
  }



  template<class T>
  void deepCopyChannel(const Img<T> *src, int srcC, Img<T> *dst, int dstC){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getSize() == dst->getSize() );
    ICLASSERT_RETURN( src->validChannel(srcC) );
    ICLASSERT_RETURN( dst->validChannel(dstC) );
    icl::core::copy<T>(src->getData(srcC),src->getData(srcC)+src->getDim(),dst->getData(dstC));
  }

  template<class S,class D>
  void convertChannel(const Img<S> *src, int srcC, Img<D> *dst, int dstC){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getSize() == dst->getSize() );
    ICLASSERT_RETURN( src->validChannel(srcC) );
    ICLASSERT_RETURN( dst->validChannel(dstC) );
    icl::core::convert<S,D>(src->getData(srcC),src->getData(srcC)+src->getDim(),dst->getData(dstC));
  }

  template <class T>
  void deepCopyChannelROI(const Img<T> *src, int srcC, const Point &srcOffs,
                          const Size &srcSize,
                          Img<T> *dst,int dstC, const Point &dstOffs,
                          const Size &dstSize) {
    CHECK_VALUES(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize);

    const ImgIterator<T> itSrc(const_cast<T*>(src->getData(srcC)),
                               src->getSize().width,
                               Rect(srcOffs,srcSize));
    ImgIterator<T> itDst(dst->getData(dstC),dst->getSize().width,Rect(dstOffs,dstSize));
    const ImgIterator<T> itSrcEnd = ImgIterator<T>::create_end_roi_iterator(src->getData(srcC),
                                                                            src->getWidth(),
                                                                            Rect(srcOffs,srcSize));

    for(;itSrc != itSrcEnd;itSrc.incRow(),itDst.incRow()){
      icl::core::copy<T>(&*itSrc,&*itSrc+srcSize.width,&*itDst);
    }
  }

  template <class S,class D>
  void convertChannelROI(const Img<S> *src, int srcC, const Point &srcOffs,
                         const Size &srcROISize,
                         Img<D> *dst,int dstC, const Point &dstOffs,
                         const Size &dstROISize)
  {
    FUNCTION_LOG("");
    CHECK_VALUES(src,srcC,srcOffs,srcROISize,dst,dstC,dstOffs,dstROISize);

    const ImgIterator<S> itSrc(const_cast<S*>(src->getData(srcC)),
                               src->getSize().width,
                               Rect(srcOffs,srcROISize));
    ImgIterator<D> itDst(dst->getData(dstC),dst->getSize().width,
                         Rect(dstOffs,dstROISize));
    const ImgIterator<S> itSrcEnd = ImgIterator<S>::create_end_roi_iterator(src->getData(srcC),
                                                                            src->getWidth(),
                                                                            Rect(srcOffs,srcROISize));
    for(;itSrc != itSrcEnd ;itSrc.incRow(),itDst.incRow()){
      icl::core::convert<S,D>(&*itSrc,&*itSrc+srcROISize.width,&*itDst);
    }
  }

  // Explicit instantiation of free template functions
#define ICL_INSTANTIATE_DEPTH(D) \
  template ICLCore_API void deepCopyChannel(const Img<icl##D>*, int, Img<icl##D>*, int);
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(D) \
  template ICLCore_API void deepCopyChannelROI(const Img<icl##D>*, int, const Point&, const Size&, \
                                               Img<icl##D>*, int, const Point&, const Size&);
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(S, D) \
  template ICLCore_API void convertChannel(const Img<icl##S>*, int, Img<icl##D>*, int);
  ICL_INSTANTIATE_ALL_DEPTHS_2
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(S, D) \
  template ICLCore_API void convertChannelROI(const Img<icl##S>*, int, const Point&, const Size&, \
                                              Img<icl##D>*, int, const Point&, const Size&);
  ICL_INSTANTIATE_ALL_DEPTHS_2
#undef ICL_INSTANTIATE_DEPTH


  template<class Type>
  const Img<Type> Img<Type>::null;

  template<class T>
  ImgBasePtrPtr<T>::~ImgBasePtrPtr(){
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


  template<class T>
  ImgBasePtrPtr<T>::ImgBasePtrPtr(Img<T> &i){
    r = new Img<T>(i);
    o = &i;
    rbef = r;
  }

  template<class T>
  ImgBasePtrPtr<T>::ImgBasePtrPtr(Img<T> *inputImage){
    ICLASSERT(inputImage != nullptr);

    if(inputImage){
      r = new Img<T>(*inputImage);
      o = inputImage;
      rbef = r;
    }else{
      r = o = rbef = 0;
    }
  }




#define ICL_INSTANTIATE_DEPTH(D)           \
template class ICLCore_API Img<icl##D>; \
template struct ICLCore_API ImgBasePtrPtr<icl##D>;
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH




  } // namespace icl::core