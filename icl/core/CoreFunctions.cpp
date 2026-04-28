// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting, Robert Haschke

#include <icl/core/CoreFunctions.h>
#include <icl/core/ImgOps.h>
#include <icl/math/MathFunctions.h>
#include <icl/utils/ClippedCast.h>
#include <icl/utils/Exception.h>
#include <icl/core/Img.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/detail/simd/SSETypes.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <numeric>
#include <type_traits>
#include <vector>

using namespace icl::utils;
using namespace icl::math;

namespace icl::core {
  ImgBase *imgNew(depth d, const Size& size, format fmt, const Rect &roi){
    return imgNew(d, ImgParams(size, fmt, roi));
  }
  ImgBase *imgNew(depth d, const Size& size, int channels, const Rect &roi){
    return imgNew(d, ImgParams(size, channels, roi));
  }
  ImgBase *imgNew(depth d, const Size& size, int channels, format fmt, const Rect &roi){
    return imgNew(d, ImgParams(size, channels, fmt, roi));
  }

  ImgBase *imgNew(depth d, const ImgParams &params){

    switch (d){
      case depth8u:  return new Img8u(params); break;
      case depth16s: return new Img16s(params); break;
      case depth32s: return new Img32s(params); break;
      case depth32f: return new Img32f(params); break;
      case depth64f: return new Img64f(params); break;
      default: ICL_INVALID_DEPTH; break;
    }
  }


  int getChannelsOfFormat(format eFormat){

    switch (eFormat){
      case formatRGB:
      case formatHLS:
      case formatLAB:
      case formatYUV:
        return 3;

      case formatChroma:
        return 2;

      case formatGray:
      case formatMatrix:
      default:
        return 1;
    }
  }



  /// puts a string representation of format into the given stream
  std::ostream &operator<<(std::ostream &s,const format &f){
    if( (static_cast<int>(f)<0) || (static_cast<int>(f))>=7) return s << "formatUnknown";
    static const char *fmts[7] = {
      "formatGray",
      "formatRGB",
      "formatHLS",
      "formatYUV",
      "formatLAB",
      "formatChroma",
      "formatMatrix"
    };
    return s << fmts[f];
  }

  /// puts a string representation of depth into the given stream
  std::ostream &operator<<(std::ostream &s,const depth &d){
    if( (static_cast<int>(d)<0) || (static_cast<int>(d))>=5) return s << "depthUnknown";
    static const char *depths[7] = {
      "depth8u",
      "depth16s",
      "depth32s",
      "depth32f",
      "depth64f"
    };
    return s << depths[d];

  }

  /// puts a string representation of format into the given stream
  std::istream &operator>>(std::istream &s, format &f){
    char cs[7]={0};

    s >> cs[0];
    s.unget();
    if(cs[0] == 'f'){
      for(int i=0;i<6;++i)s>>cs[i];
      ICLASSERT(str(cs) == "format");
    }else{
      // nothing, this is just a compability mode for
      // someone forgetting the format prefix!
    }
    std::fill(cs,cs+7,'\0');
    s >> cs[0];
    cs[0] = tolower(cs[0]);
    int rest = 2;
    std::string expect;
    switch(cs[0]){
      case 'g':
        rest = 3;
        expect="gray";
        f = formatGray;
        break;
      case 'r':
        expect="rgb";
        f = formatRGB;
        break;
      case 'h':
        expect="hls";
        f = formatHLS;
        break;
      case 'y':
        expect="yuv";
        f = formatYUV;
        break;
      case 'l':
        expect="lab";
        f = formatLAB;
        break;
      case 'c':
        rest = 5;
        f = formatChroma;
        expect="chroma";
        break;
      case 'm':
        rest = 5;
        f = formatMatrix;
        expect="matrix";
        break;
      default:
        ERROR_LOG("unable to parse format-type");
        return s;
    }
    for(int i=0;i<rest;++i){
      s >> cs[i+1];
      cs[i+1] = tolower(cs[i+1]);
    }
    if(expect != cs){
      ERROR_LOG("unabled t parse format: found: " << cs << " expected:" << expect);
    }
    return s;
  }

  /// puts a string representation of depth into the given stream
  std::istream &operator>>(std::istream &s, depth &d){
    char cs[6]={0};
    s >> cs[0];
    s.unget();
    if(cs[0] == 'd'){
      for(int i=0;i<5;++i) s>>cs[i];
      ICLASSERT(str(cs) == "depth");
    }else{
      // compability mode for someone forgetting the depth-prefix
    }
    std::fill(cs,cs+6,'\0');
    s >> cs[0];
    switch(cs[0]){
      case '8':
        s >> cs[1];
        ICLASSERT(str(cs) == "8u");
        d = depth8u;
        return s;
      case '1':
        s >> cs[1] >> cs[2];
        ICLASSERT(str(cs) == "16s");
        d = depth16s;
        return s;
      case '3':
        s >> cs[1] >> cs[2];
        if(cs[2] == 's'){
          ICLASSERT(str(cs) == "32s");
          d = depth32s;
        }else{
          ICLASSERT(str(cs) == "32f");
          d = depth32f;
        }
        return s;
      case '6':
        s >> cs[1] >> cs[2];
        ICLASSERT(str(cs) == "64f");
        d = depth64f;
        return s;
      default:
        ERROR_LOG("error parsing depth-type");
        return s;
    }
  }




  ImgBase *ensureDepth(ImgBase **ppoImage, depth d){
    if(!ppoImage){
      return imgNew(d);
    }
    if(!*ppoImage){
      *ppoImage = imgNew(d);
    }
    else if((*ppoImage)->getDepth() != d){
      ImgBase *poNew = imgNew(d,(*ppoImage)->getParams());
      delete *ppoImage;
      *ppoImage = poNew;
    }
    return *ppoImage;
  }


  ImgBase *ensureCompatible(ImgBase **ppoDst, depth d, const ImgParams &params){
    if(!ppoDst){
      return imgNew(d,params);
    }
    if(!*ppoDst){
      *ppoDst = imgNew(d,params);
    }else{
      ensureDepth(ppoDst,d);
      (*ppoDst)->setParams(params);
    }
    return *ppoDst;
  }


  ImgBase *ensureCompatible(ImgBase **dst, depth d, const Size& size, int channels, const Rect &roi){
    return ensureCompatible(dst, d, ImgParams(size, channels, roi));
  }
  ImgBase *ensureCompatible(ImgBase **dst, depth d, const Size& size, format fmt, const Rect &roi){
    return ensureCompatible(dst, d, ImgParams(size, fmt, roi));
  }

  ImgBase* ensureCompatible(ImgBase **dst, depth d, const Size &size, int channels, format fmt, const Rect &roi){

    FUNCTION_LOG("");
    if(fmt != formatMatrix && getChannelsOfFormat(fmt) != channels){
      ensureCompatible(dst,d,size,channels,roi);
      throw InvalidImgParamException("channels and format");
      return 0;
    }else{
      ImgBase *ret = ensureCompatible(dst,d,size,channels,roi);
      (*dst)->setFormat(fmt);
      return ret;
    }
  }


  ImgBase *ensureCompatible(ImgBase **dst, const ImgBase *src){
    return ensureCompatible(dst,src->getDepth(),src->getParams());
  }

  unsigned int getSizeOf(depth eDepth){
    static unsigned int s_aiSizeTable[]= { sizeof(icl8u),
                                           sizeof(icl16s),
                                           sizeof(icl32s),
                                           sizeof(icl32f),
                                           sizeof(icl64f) };
    return s_aiSizeTable[eDepth];
  }







// channelMean C++ backend moved to Img_Cpp.cpp

  std::vector<double> mean(const ImgBase *poImg, int iChannel, bool roiOnly){
    FUNCTION_LOG("");
    std::vector<double> vecMean;
    ICLASSERT_RETURN_VAL(poImg,vecMean);

    int firstChannel = iChannel<0 ? 0 : iChannel;
    int lastChannel = iChannel<0 ? poImg->getChannels()-1 : firstChannel;
    ImgBase* self = const_cast<ImgBase*>(poImg);
    auto& sel = ImgOps::instance().getSelector<ImgOps::ChannelMeanSig>(ImgOps::Op::channelMean);
    for(int i=firstChannel;i<=lastChannel;++i){
      vecMean.push_back(sel.resolveOrThrow(self)->apply(*self, i, roiOnly));
    }
    return vecMean;
  }





 namespace{
    template<class T>
    double channel_var_with_mean(const Img<T> &image, int channel,double mean,bool empiricMean, bool roiOnly){
      if(roiOnly && !image.hasFullROI()){
        return math::variance(image.beginROI(channel),image.endROI(channel),mean,empiricMean);
      }else{
        return math::variance(image.begin(channel),image.end(channel),mean,empiricMean);
      }
    }
    // no IPP function available with given mean
  }

  std::vector<double> variance(const ImgBase *poImg, const std::vector<double> &mean, bool empiricMean,  int iChannel, bool roiOnly){
    FUNCTION_LOG("");
    std::vector<double> vecVar;
    ICLASSERT_RETURN_VAL(poImg,vecVar);

    int firstChannel = iChannel<0 ? 0 : iChannel;
    int lastChannel = iChannel<0 ? poImg->getChannels()-1 : firstChannel;

    switch(poImg->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                                                           \
      case depth##D:                                                                                       \
        for(int i=firstChannel,j=0;i<=lastChannel;++i,++j){                                                \
          ICLASSERT_RETURN_VAL(j<static_cast<int>(mean.size()),vecVar);                                                 \
          vecVar.push_back(channel_var_with_mean(*poImg->asImg<icl##D>(),i,mean[j],empiricMean,roiOnly));  \
        }                                                                                                  \
      break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
    return vecVar;


  }

  /// Compute the variance value of an image a \ingroup MATH
  /** @param poImg input imge
      @param iChannel channel index (-1 for all channels)
      @return The variance value form the vector
      */
  std::vector<double> variance(const ImgBase *poImg, int iChannel, bool roiOnly){
    return variance(poImg,mean(poImg,iChannel,roiOnly),true,iChannel,roiOnly);
  }


  std::vector<double> stdDeviation(const ImgBase *poImage, int iChannel, bool roiOnly){
    std::vector<double> v = variance(poImage,iChannel,roiOnly);
    for(unsigned int i=0;i<v.size();++i){
      v[i] = ::sqrt(v[i]);
    }
    return v;
  }

  /// Compute the deviation of an image with given channel means
  /** @param poImage input image
      @param iChannel channel index (all channels if -1)
  */
  std::vector<double> stdDeviation(const ImgBase *poImage, const std::vector<double> mean, bool empiricMean, int iChannel, bool roiOnly){
    std::vector<double> v = variance(poImage,mean,empiricMean, iChannel,roiOnly);

    for(unsigned int i=0;i<v.size();++i){
      v[i] = ::sqrt(v[i]);
    }
    return v;
  }

  std::vector< std::pair<double,double> > meanAndStdDev(const ImgBase *image,
                                                        int iChannel,
                                                        bool roiOnly){
    std::vector<double> channelMeans = mean(image,iChannel,roiOnly);
    std::vector<double> channelStdDevs = stdDeviation(image,channelMeans,true,iChannel,roiOnly);

    std::vector<std::pair<double,double> > md(channelMeans.size());
    for(unsigned int i=0;i<channelMeans.size();++i){
      md[i].first = channelMeans[i];
      md[i].second = channelStdDevs[i];
    }
    return md;
  }


  namespace{

    template<class T>
    void compute_default_histo_256(const Img<T> &image, int c, std::vector<int> &h, bool roiOnly){
      ICLASSERT_RETURN(h.size() == 256);
      if(roiOnly && !image.hasFullROI()){
        const ImgIterator<T> it = image.beginROI(c);
        const ImgIterator<T> itEnd = image.endROI(c);
        for(;it!=itEnd;++it){
          h[clipped_cast<T,icl8u>(*it)]++;
        }
      }else{
        const T* p = image.getData(c);
        const T* pEnd = p+image.getDim();
        while(p<pEnd){
          h[clipped_cast<T,icl8u>(*p++)]++;
        }
      }
    }


    template<class T>
    inline void histo_entry(T v, double m, std::vector<int> &h, unsigned int n, double r){
      // todo check 1000 times
      h[ floor( n*(v-m)/(r+1)) ]++;
      //      h[ ceil( n*(v-m)/r) ]++; problem at v=255
    }

    template<class T>
    void compute_complex_histo(const Img<T> &image, int c, std::vector<int> &h, bool roiOnly){
      const Range<T> range = image.getMinMax(c);
      double r = range.getLength();
      unsigned int n = h.size();

      if(roiOnly && !image.hasFullROI()){
        const ImgIterator<T> it = image.beginROI(c);
        const ImgIterator<T> itEnd = image.endROI(c);
        for(;it!=itEnd;++it){
          histo_entry(*it,range.minVal,h,n,r);
        }
      }else{
        const T* p = image.begin(c);
        const T* pEnd = image.end(c);
        while(p<pEnd){
          histo_entry(*p++,range.minVal,h,n,r);
        }
      }
    }

    template<class T>
    void compute_channel_histo(const Img<T> &image, int c, std::vector<int> &h, bool roiOnly){
      if(image.getFormat() != formatMatrix && h.size() == 256){
        compute_default_histo_256(image,c,h,roiOnly);
      }else{
        compute_complex_histo(image,c,h,roiOnly);
      }
    }
  }

  std::vector<int> channelHisto(const ImgBase *image,int channel, int levels, bool roiOnly){
    ICLASSERT_RETURN_VAL(image && image->getChannels()>channel, std::vector<int>());
    ICLASSERT_RETURN_VAL(levels > 1,std::vector<int>());

    std::vector<int> h(levels);
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: compute_channel_histo(*image->asImg<icl##D>(),channel,h,roiOnly); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
    return h;
  }


  std::vector<std::vector<int> > hist(const ImgBase *image, int levels, bool roiOnly){
    ICLASSERT_RETURN_VAL(image && image->getChannels(), std::vector<std::vector<int> >());
    std::vector<std::vector<int> > h(image->getChannels());
    for(int i=0;i<image->getChannels();i++){
      h[i] = channelHisto(image,i,levels,roiOnly);
    }
    return h;
  }



  
  // ================================================================
  // copy<T>: memcpy-based array copy
  // ================================================================

  template<class T>
  void copy(const T *src, const T *srcEnd, T *dst){
    memcpy(dst, src, static_cast<size_t>(srcEnd - src) * sizeof(T));
  }

  template void copy(const icl8u*,  const icl8u*,  icl8u*);
  template void copy(const icl16s*, const icl16s*, icl16s*);
  template void copy(const icl32s*, const icl32s*, icl32s*);
  template void copy(const icl32f*, const icl32f*, icl32f*);
  template void copy(const icl64f*, const icl64f*, icl64f*);


  // ================================================================
  // convert<S,D>: generic clipped_cast conversion
  // ================================================================

  template<class srcT, class dstT>
  void convert(const srcT *poSrcStart, const srcT *poSrcEnd, dstT *poDst){
    if constexpr (std::is_same_v<srcT, dstT>){
      copy(poSrcStart, poSrcEnd, poDst);
    } else {
      std::transform(poSrcStart, poSrcEnd, poDst, utils::clipped_cast<srcT, dstT>);
    }
  }

  // ================================================================
  // SSE2-optimized convert specializations (must precede explicit
  // instantiation so the compiler sees the specialization first)
  // ================================================================

#ifdef ICL_HAVE_SSE2

  // --- icl8u → icl32f (16 pixels/iteration) ---
  template<> void convert<icl8u,icl32f>(const icl8u *s, const icl8u *e, icl32f *d) {
    for(; (reinterpret_cast<uintptr_t>(d) & 15) && s < e; ++s, ++d)
      *d = static_cast<icl32f>(*s);
    const __m128i z = _mm_setzero_si128();
    for(; s < e-15; s += 16, d += 16){
      __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s));
      __m128i lo = _mm_unpacklo_epi8(v, z), hi = _mm_unpackhi_epi8(v, z);
      _mm_store_ps(d,    _mm_cvtepi32_ps(_mm_unpacklo_epi16(lo, z)));
      _mm_store_ps(d+4,  _mm_cvtepi32_ps(_mm_unpackhi_epi16(lo, z)));
      _mm_store_ps(d+8,  _mm_cvtepi32_ps(_mm_unpacklo_epi16(hi, z)));
      _mm_store_ps(d+12, _mm_cvtepi32_ps(_mm_unpackhi_epi16(hi, z)));
    }
    for(; s < e; ++s, ++d) *d = static_cast<icl32f>(*s);
  }

  // --- icl32f → icl8u (16 pixels/iteration, with saturation) ---
  template<> void convert<icl32f,icl8u>(const icl32f *s, const icl32f *e, icl8u *d) {
    for(; (reinterpret_cast<uintptr_t>(s) & 15) && s < e; ++s, ++d)
      *d = utils::clipped_cast<icl32f,icl8u>(*s);
    for(; s < e-15; s += 16, d += 16){
      __m128i a = _mm_cvttps_epi32(_mm_load_ps(s));
      __m128i b = _mm_cvttps_epi32(_mm_load_ps(s+4));
      __m128i c = _mm_cvttps_epi32(_mm_load_ps(s+8));
      __m128i f = _mm_cvttps_epi32(_mm_load_ps(s+12));
      _mm_storeu_si128(reinterpret_cast<__m128i*>(d),
        _mm_packus_epi16(_mm_packs_epi32(a, b), _mm_packs_epi32(c, f)));
    }
    for(; s < e; ++s, ++d) *d = utils::clipped_cast<icl32f,icl8u>(*s);
  }

  // --- icl16s → icl32s (8 pixels/iteration) ---
  template<> void convert<icl16s,icl32s>(const icl16s *s, const icl16s *e, icl32s *d) {
    for(; (reinterpret_cast<uintptr_t>(d) & 15) && s < e; ++s, ++d)
      *d = static_cast<icl32s>(*s);
    const __m128i z = _mm_setzero_si128();
    for(; s < e-7; s += 8, d += 8){
      __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s));
      _mm_store_si128(reinterpret_cast<__m128i*>(d),   _mm_unpacklo_epi16(v, z));
      _mm_store_si128(reinterpret_cast<__m128i*>(d+4), _mm_unpackhi_epi16(v, z));
    }
    for(; s < e; ++s, ++d) *d = static_cast<icl32s>(*s);
  }

  // --- icl16s → icl32f (8 pixels/iteration) ---
  template<> void convert<icl16s,icl32f>(const icl16s *s, const icl16s *e, icl32f *d) {
    for(; (reinterpret_cast<uintptr_t>(d) & 15) && s < e; ++s, ++d)
      *d = static_cast<icl32f>(*s);
    const __m128i z = _mm_setzero_si128();
    for(; s < e-7; s += 8, d += 8){
      __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s));
      _mm_store_ps(d,   _mm_cvtepi32_ps(_mm_unpacklo_epi16(v, z)));
      _mm_store_ps(d+4, _mm_cvtepi32_ps(_mm_unpackhi_epi16(v, z)));
    }
    for(; s < e; ++s, ++d) *d = static_cast<icl32f>(*s);
  }

  // --- icl16s → icl64f (8 pixels/iteration) ---
  template<> void convert<icl16s,icl64f>(const icl16s *s, const icl16s *e, icl64f *d) {
    for(; (reinterpret_cast<uintptr_t>(d) & 15) && s < e; ++s, ++d)
      *d = static_cast<icl64f>(*s);
    const __m128i z = _mm_setzero_si128();
    for(; s < e-7; s += 8, d += 8){
      __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s));
      __m128i lo = _mm_unpacklo_epi16(v, z), hi = _mm_unpackhi_epi16(v, z);
      _mm_store_pd(d,   _mm_cvtepi32_pd(lo));
      _mm_store_pd(d+2, _mm_cvtepi32_pd(_mm_shuffle_epi32(lo, _MM_SHUFFLE(1,0,3,2))));
      _mm_store_pd(d+4, _mm_cvtepi32_pd(hi));
      _mm_store_pd(d+6, _mm_cvtepi32_pd(_mm_shuffle_epi32(hi, _MM_SHUFFLE(1,0,3,2))));
    }
    for(; s < e; ++s, ++d) *d = static_cast<icl64f>(*s);
  }

  // --- icl32s → icl16s (8 pixels/iteration, saturating) ---
  template<> void convert<icl32s,icl16s>(const icl32s *s, const icl32s *e, icl16s *d) {
    for(; (reinterpret_cast<uintptr_t>(s) & 15) && s < e; ++s, ++d)
      *d = utils::clipped_cast<icl32s,icl16s>(*s);
    for(; s < e-7; s += 8, d += 8){
      __m128i v1 = _mm_load_si128(reinterpret_cast<const __m128i*>(s));
      __m128i v2 = _mm_load_si128(reinterpret_cast<const __m128i*>(s+4));
      _mm_storeu_si128(reinterpret_cast<__m128i*>(d), _mm_packs_epi32(v1, v2));
    }
    for(; s < e; ++s, ++d) *d = utils::clipped_cast<icl32s,icl16s>(*s);
  }

  // --- icl32s → icl32f (4 pixels/iteration) ---
  template<> void convert<icl32s,icl32f>(const icl32s *s, const icl32s *e, icl32f *d) {
    for(; (reinterpret_cast<uintptr_t>(d) & 15) && s < e; ++s, ++d)
      *d = static_cast<icl32f>(*s);
    for(; s < e-3; s += 4, d += 4)
      _mm_store_ps(d, _mm_cvtepi32_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(s))));
    for(; s < e; ++s, ++d) *d = static_cast<icl32f>(*s);
  }

  // --- icl32s → icl64f (4 pixels/iteration) ---
  template<> void convert<icl32s,icl64f>(const icl32s *s, const icl32s *e, icl64f *d) {
    for(; (reinterpret_cast<uintptr_t>(d) & 15) && s < e; ++s, ++d)
      *d = static_cast<icl64f>(*s);
    for(; s < e-3; s += 4, d += 4){
      __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s));
      _mm_store_pd(d,   _mm_cvtepi32_pd(v));
      _mm_store_pd(d+2, _mm_cvtepi32_pd(_mm_shuffle_epi32(v, _MM_SHUFFLE(1,0,3,2))));
    }
    for(; s < e; ++s, ++d) *d = static_cast<icl64f>(*s);
  }

  // --- icl32f → icl16s (8 pixels/iteration, saturating) ---
  template<> void convert<icl32f,icl16s>(const icl32f *s, const icl32f *e, icl16s *d) {
    for(; (reinterpret_cast<uintptr_t>(s) & 15) && s < e; ++s, ++d)
      *d = utils::clipped_cast<icl32f,icl16s>(*s);
    __m128 vMin = _mm_set1_ps(static_cast<float>(std::numeric_limits<icl16s>::min()));
    __m128 vMax = _mm_set1_ps(static_cast<float>(std::numeric_limits<icl16s>::max()));
    for(; s < e-7; s += 8, d += 8){
      __m128 v1 = _mm_min_ps(_mm_max_ps(_mm_load_ps(s),   vMin), vMax);
      __m128 v2 = _mm_min_ps(_mm_max_ps(_mm_load_ps(s+4), vMin), vMax);
      _mm_storeu_si128(reinterpret_cast<__m128i*>(d),
        _mm_packs_epi32(_mm_cvttps_epi32(v1), _mm_cvttps_epi32(v2)));
    }
    for(; s < e; ++s, ++d) *d = utils::clipped_cast<icl32f,icl16s>(*s);
  }

  // --- icl32f → icl32s (4 pixels/iteration, saturating) ---
  template<> void convert<icl32f,icl32s>(const icl32f *s, const icl32f *e, icl32s *d) {
    for(; (reinterpret_cast<uintptr_t>(d) & 15) && s < e; ++s, ++d)
      *d = utils::clipped_cast<icl32f,icl32s>(*s);
    __m128 vMin = _mm_set1_ps(static_cast<float>(std::numeric_limits<icl32s>::min()));
    __m128 vMax = _mm_set1_ps(static_cast<float>(std::numeric_limits<icl32s>::max()));
    for(; s < e-3; s += 4, d += 4){
      __m128 v = _mm_min_ps(_mm_max_ps(_mm_loadu_ps(s), vMin), vMax);
      _mm_store_si128(reinterpret_cast<__m128i*>(d), _mm_cvttps_epi32(v));
    }
    for(; s < e; ++s, ++d) *d = utils::clipped_cast<icl32f,icl32s>(*s);
  }

  // --- icl32f → icl64f (4 pixels/iteration) ---
  template<> void convert<icl32f,icl64f>(const icl32f *s, const icl32f *e, icl64f *d) {
    for(; (reinterpret_cast<uintptr_t>(d) & 15) && s < e; ++s, ++d)
      *d = static_cast<icl64f>(*s);
    for(; s < e-3; s += 4, d += 4){
      __m128 v = _mm_loadu_ps(s);
      _mm_store_pd(d,   _mm_cvtps_pd(v));
      _mm_store_pd(d+2, _mm_cvtps_pd(_mm_movehl_ps(v, v)));
    }
    for(; s < e; ++s, ++d) *d = static_cast<icl64f>(*s);
  }

  // --- icl64f → icl32f (4 pixels/iteration) ---
  template<> void convert<icl64f,icl32f>(const icl64f *s, const icl64f *e, icl32f *d) {
    for(; (reinterpret_cast<uintptr_t>(s) & 15) && s < e; ++s, ++d)
      *d = static_cast<icl32f>(*s);
    for(; s < e-3; s += 4, d += 4){
      __m128 lo = _mm_cvtpd_ps(_mm_load_pd(s));
      __m128 hi = _mm_cvtpd_ps(_mm_load_pd(s+2));
      _mm_storeu_ps(d, _mm_shuffle_ps(lo, hi, _MM_SHUFFLE(1,0,1,0)));
    }
    for(; s < e; ++s, ++d) *d = static_cast<icl32f>(*s);
  }

  // --- icl64f → icl32s (4 pixels/iteration, saturating) ---
  template<> void convert<icl64f,icl32s>(const icl64f *s, const icl64f *e, icl32s *d) {
    for(; (reinterpret_cast<uintptr_t>(s) & 15) && s < e; ++s, ++d)
      *d = utils::clipped_cast<icl64f,icl32s>(*s);
    __m128d vMin = _mm_set1_pd(static_cast<double>(std::numeric_limits<icl32s>::min()));
    __m128d vMax = _mm_set1_pd(static_cast<double>(std::numeric_limits<icl32s>::max()));
    for(; s < e-3; s += 4, d += 4){
      __m128d v1 = _mm_min_pd(_mm_max_pd(_mm_load_pd(s),   vMin), vMax);
      __m128d v2 = _mm_min_pd(_mm_max_pd(_mm_load_pd(s+2), vMin), vMax);
      __m128i lo = _mm_cvttpd_epi32(v1);
      __m128i hi = _mm_cvttpd_epi32(v2);
      _mm_storeu_si128(reinterpret_cast<__m128i*>(d),
        _mm_add_epi32(lo, _mm_shuffle_epi32(hi, _MM_SHUFFLE(1,0,3,2))));
    }
    for(; s < e; ++s, ++d) *d = utils::clipped_cast<icl64f,icl32s>(*s);
  }

#endif // ICL_HAVE_SSE2

  // ================================================================
  // Explicit instantiation for all type pairs.
  // Pairs with SSE2 specializations above are skipped when SSE2 is
  // enabled (explicit instantiation after specialization is a no-op
  // that triggers a warning).
  // ================================================================

#ifdef ICL_HAVE_SSE2
  // icl8u → others (icl8u→icl32f is SSE2-specialized)
  template void convert(const icl8u*,  const icl8u*,  icl8u*);
  template void convert(const icl8u*,  const icl8u*,  icl16s*);
  template void convert(const icl8u*,  const icl8u*,  icl32s*);
  template void convert(const icl8u*,  const icl8u*,  icl64f*);
  // icl8s → all
  template void convert(const icl8s*,  const icl8s*,  icl8u*);
  template void convert(const icl8s*,  const icl8s*,  icl8s*);
  template void convert(const icl8s*,  const icl8s*,  icl16s*);
  template void convert(const icl8s*,  const icl8s*,  icl32s*);
  template void convert(const icl8s*,  const icl8s*,  icl32f*);
  template void convert(const icl8s*,  const icl8s*,  icl64f*);
  // icl16s → others (icl16s→icl32s/icl32f/icl64f are SSE2-specialized)
  template void convert(const icl16s*, const icl16s*, icl8u*);
  template void convert(const icl16s*, const icl16s*, icl16s*);
  // icl32s → others (icl32s→icl16s/icl32f/icl64f are SSE2-specialized)
  template void convert(const icl32s*, const icl32s*, icl8u*);
  template void convert(const icl32s*, const icl32s*, icl32s*);
  // icl32f → others (icl32f→icl8u/icl16s/icl32s/icl64f are SSE2-specialized)
  template void convert(const icl32f*, const icl32f*, icl32f*);
  // icl64f → others (icl64f→icl32f/icl32s are SSE2-specialized)
  template void convert(const icl64f*, const icl64f*, icl8u*);
  template void convert(const icl64f*, const icl64f*, icl16s*);
  template void convert(const icl64f*, const icl64f*, icl64f*);
#else
#define ICL_INSTANTIATE_DEPTH(D)                                        \
  template void convert(const icl8u*,  const icl8u*,  icl##D*);      \
  template void convert(const icl8s*,  const icl8s*,  icl##D*);      \
  template void convert(const icl16s*, const icl16s*, icl##D*);      \
  template void convert(const icl32s*, const icl32s*, icl##D*);      \
  template void convert(const icl32f*, const icl32f*, icl##D*);      \
  template void convert(const icl64f*, const icl64f*, icl##D*);
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
#endif
  // icl*→icl8s (not in the macro's depth set, no SSE2 specializations)
  template void convert(const icl8u*,  const icl8u*,  icl8s*);
  template void convert(const icl16s*, const icl16s*, icl8s*);
  template void convert(const icl32s*, const icl32s*, icl8s*);
  template void convert(const icl32f*, const icl32f*, icl8s*);
  template void convert(const icl64f*, const icl64f*, icl8s*);

  } // namespace icl::core
