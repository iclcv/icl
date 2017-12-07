/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/CoreFunctions.cpp                  **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Michael Goetting, Robert Haschke, **
**          Sergius Gaulik                                         **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLCore/CoreFunctions.h>
#include <ICLMath/MathFunctions.h>
#include <ICLUtils/Exception.h>
#include <ICLCore/Img.h>
#include <ICLUtils/StringUtils.h>

#include <vector>
#include <numeric>

using namespace icl::utils;
using namespace icl::math;

namespace icl{
  namespace core{

    ImgBase *imgNew(depth d, const ImgParams &params){
      // {{{ open

      switch (d){
        case depth8u:  return new Img8u(params); break;
        case depth16s: return new Img16s(params); break;
        case depth32s: return new Img32s(params); break;
        case depth32f: return new Img32f(params); break;
        case depth64f: return new Img64f(params); break;
        default: ICL_INVALID_DEPTH; break;
      }
    }

    // }}}

    int getChannelsOfFormat(format eFormat){
      // {{{ open

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

    // }}}


    /// puts a string representation of format into the given stream
    std::ostream &operator<<(std::ostream &s,const format &f){
      if( ((int)f<0) || ((int)f)>=7) return s << "formatUnknown";
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
      if( ((int)d<0) || ((int)d)>=5) return s << "depthUnknown";
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
      // {{{ open
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

    // }}}

    ImgBase *ensureCompatible(ImgBase **ppoDst, depth d, const ImgParams &params){
      // {{{ open
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

    // }}}

    ImgBase* ensureCompatible(ImgBase **dst, depth d, const Size &size, int channels, format fmt, const Rect &roi){
      // {{{ open

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

    // }}}

    ImgBase *ensureCompatible(ImgBase **dst, const ImgBase *src){
      // {{{ open
      return ensureCompatible(dst,src->getDepth(),src->getParams());
    }
    // }}}

    unsigned int getSizeOf(depth eDepth){
      // {{{ open
      static unsigned int s_aiSizeTable[]= { sizeof(icl8u),
                                             sizeof(icl16s),
                                             sizeof(icl32s),
                                             sizeof(icl32f),
                                             sizeof(icl64f) };
      return s_aiSizeTable[eDepth];
    }

    // }}}


    // {{{ convert



  #ifdef ICL_HAVE_IPP
  #elif defined ICL_HAVE_SSE2
    // --- from icl8u to icl32f ---
    template<> void convert<icl8u,icl32f>(const icl8u *poSrcStart,const icl8u *poSrcEnd, icl32f *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poDst) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = static_cast<icl32f>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-15; poSrcStart += 16, poDst += 16) {
        // zero vector
        const __m128i vk0 = _mm_set1_epi8(0);

        // load 8u values
        __m128i v = _mm_loadu_si128((__m128i*)poSrcStart);

        // convert to 16u values
        __m128i vl = _mm_unpacklo_epi8(v, vk0);
        __m128i vh = _mm_unpackhi_epi8(v, vk0);
        // convert to 32u values
        __m128i vll = _mm_unpacklo_epi16(vl, vk0);
        __m128i vlh = _mm_unpackhi_epi16(vl, vk0);
        __m128i vhl = _mm_unpacklo_epi16(vh, vk0);
        __m128i vhh = _mm_unpackhi_epi16(vh, vk0);
        // convert to 32f values
        __m128 rvll = _mm_cvtepi32_ps(vll);
        __m128 rvlh = _mm_cvtepi32_ps(vlh);
        __m128 rvhl = _mm_cvtepi32_ps(vhl);
        __m128 rvhh = _mm_cvtepi32_ps(vhh);

        // store 32f values
        _mm_store_ps(poDst, rvll);
        _mm_store_ps(poDst+4, rvlh);
        _mm_store_ps(poDst+8, rvhl);
        _mm_store_ps(poDst+12, rvhh);
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = static_cast<icl32f>(*poSrcStart);
      }
    }


    // --- from icl16s to icl32s ---
    template<> void convert<icl16s,icl32s>(const icl16s *poSrcStart,const icl16s *poSrcEnd, icl32s *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poDst) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = static_cast<icl32f>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-7; poSrcStart += 8, poDst += 8) {
        // zero vector
        const __m128i vk0 = _mm_set1_epi16(0);

        // load 16s values
        __m128i v = _mm_loadu_si128((__m128i*)poSrcStart);

        // convert to 32s values
        __m128i vl = _mm_unpacklo_epi16(v, vk0);
        __m128i vh = _mm_unpackhi_epi16(v, vk0);

        // store 32s values
        _mm_store_si128((__m128i*)poDst, vl);
        _mm_store_si128((__m128i*)(poDst+4), vh);
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = static_cast<icl32f>(*poSrcStart);
      }
    }

    // --- from icl16s to icl32f ---
    template<> void convert<icl16s,icl32f>(const icl16s *poSrcStart,const icl16s *poSrcEnd, icl32f *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poDst) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < -std::numeric_limits<icl32f>::max() ? -std::numeric_limits<icl32f>::max() :
        *poSrcStart > std::numeric_limits<icl32f>::max() ? std::numeric_limits<icl32f>::max() :
        static_cast<icl32f>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-7; poSrcStart += 8, poDst += 8) {
        // zero vector
        const __m128i vk0 = _mm_set1_epi16(0);
        // load 16s values
        __m128i v = _mm_loadu_si128((__m128i*)poSrcStart);

        // convert to 32s values
        __m128i vl = _mm_unpacklo_epi16(v, vk0);
        __m128i vh = _mm_unpackhi_epi16(v, vk0);

        // convert to 32f values
        __m128 rvl = _mm_cvtepi32_ps(vl);
        __m128 rvh = _mm_cvtepi32_ps(vh);

        // store 32f values
        _mm_store_ps(poDst, rvl);
        _mm_store_ps(poDst+4, rvh);
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < -std::numeric_limits<icl32f>::max() ? -std::numeric_limits<icl32f>::max() :
        *poSrcStart > std::numeric_limits<icl32f>::max() ? std::numeric_limits<icl32f>::max() :
        static_cast<icl32f>(*poSrcStart);
      }
    }

    // --- from icl16s to icl64f ---
    template<> void convert<icl16s,icl64f>(const icl16s *poSrcStart,const icl16s *poSrcEnd, icl64f *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poDst) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < -std::numeric_limits<icl64f>::max() ? -std::numeric_limits<icl64f>::max() :
        *poSrcStart > std::numeric_limits<icl64f>::max() ? std::numeric_limits<icl64f>::max() :
        static_cast<icl64f>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-7; poSrcStart += 8, poDst += 8) {
        // zero vector
        const __m128i vk0 = _mm_set1_epi16(0);

        // load 16s values
        __m128i v = _mm_loadu_si128((__m128i*)poSrcStart);

        // convert to 32s values
        __m128i vl = _mm_unpacklo_epi16(v, vk0);
        __m128i vh = _mm_unpackhi_epi16(v, vk0);

        // convert to 64f values
        __m128d vll = _mm_cvtepi32_pd(vl);
        __m128d vlh = _mm_cvtepi32_pd(_mm_shuffle_epi32(vl, _MM_SHUFFLE(1, 0, 3, 2)));
        __m128d vhl = _mm_cvtepi32_pd(vh);
        __m128d vhh = _mm_cvtepi32_pd(_mm_shuffle_epi32(vh, _MM_SHUFFLE(1, 0, 3, 2)));

        // store 64f values
        _mm_store_pd(poDst,   vll);
        _mm_store_pd(poDst+2, vlh);
        _mm_store_pd(poDst+4, vhl);
        _mm_store_pd(poDst+6, vhh);
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < -std::numeric_limits<icl64f>::max() ? -std::numeric_limits<icl64f>::max() :
        *poSrcStart > std::numeric_limits<icl64f>::max() ? std::numeric_limits<icl64f>::max() :
        static_cast<icl64f>(*poSrcStart);
      }
    }


    // --- from icl32s to icl16s ---
    template<> void convert<icl32s,icl16s>(const icl32s *poSrcStart,const icl32s *poSrcEnd, icl16s *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poSrcStart) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < std::numeric_limits<icl16s>::min() ? std::numeric_limits<icl16s>::min() :
        *poSrcStart > std::numeric_limits<icl16s>::max() ? std::numeric_limits<icl16s>::max() :
        static_cast<icl16s>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-7; poSrcStart += 8, poDst += 8) {
        // load 32s values
        __m128i v1 = _mm_load_si128((__m128i*)poSrcStart);
        __m128i v2 = _mm_load_si128((__m128i*)(poSrcStart+4));

        // convert to 16s values; store 16s values
        _mm_storeu_si128((__m128i*)poDst, _mm_packs_epi32(v1, v2));
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < std::numeric_limits<icl16s>::min() ? std::numeric_limits<icl16s>::min() :
        *poSrcStart > std::numeric_limits<icl16s>::max() ? std::numeric_limits<icl16s>::max() :
        static_cast<icl16s>(*poSrcStart);
      }
    }

    // --- from icl16s to icl32f ---
    template<> void convert<icl32s,icl32f>(const icl32s *poSrcStart,const icl32s *poSrcEnd, icl32f *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poDst) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < -std::numeric_limits<icl32f>::max() ? -std::numeric_limits<icl32f>::max() :
        *poSrcStart > std::numeric_limits<icl32f>::max() ? std::numeric_limits<icl32f>::max() :
        static_cast<icl32f>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-3; poSrcStart += 4, poDst += 4) {
        // load 32s values
        __m128i v = _mm_loadu_si128((__m128i*)poSrcStart);

        // convert to 32f values; store 32f values
        _mm_store_ps(poDst, _mm_cvtepi32_ps(v));
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < -std::numeric_limits<icl32f>::max() ? -std::numeric_limits<icl32f>::max() :
        *poSrcStart > std::numeric_limits<icl32f>::max() ? std::numeric_limits<icl32f>::max() :
        static_cast<icl32f>(*poSrcStart);
      }
    }

    // --- from icl16s to icl64f ---
    template<> void convert<icl32s,icl64f>(const icl32s *poSrcStart,const icl32s *poSrcEnd, icl64f *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poDst) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < -std::numeric_limits<icl64f>::max() ? -std::numeric_limits<icl64f>::max() :
        *poSrcStart > std::numeric_limits<icl64f>::max() ? std::numeric_limits<icl64f>::max() :
        static_cast<icl64f>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-3; poSrcStart += 4, poDst += 4) {
        // load 32s values
        __m128i v = _mm_loadu_si128((__m128i*)poSrcStart);

        // convert to 64f values
        __m128d vl = _mm_cvtepi32_pd(v);
        __m128d vh = _mm_cvtepi32_pd(_mm_shuffle_epi32(v, _MM_SHUFFLE(1, 0, 3, 2)));

        // store 64f values
        _mm_store_pd(poDst,   vl);
        _mm_store_pd(poDst+2, vh);
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < -std::numeric_limits<icl64f>::max() ? -std::numeric_limits<icl64f>::max() :
        *poSrcStart > std::numeric_limits<icl64f>::max() ? std::numeric_limits<icl64f>::max() :
        static_cast<icl64f>(*poSrcStart);
      }
    }


    // --- from icl32f to icl8u ---
    template <> void convert<icl32f,icl8u>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl8u *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poSrcStart) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < std::numeric_limits<icl8u>::min() ? std::numeric_limits<icl8u>::min() :
        *poSrcStart > std::numeric_limits<icl8u>::max() ? std::numeric_limits<icl8u>::max() :
        static_cast<icl8u>(*poSrcStart);
      }

      // save current rounding mode
      unsigned int initial_mode = _MM_GET_ROUNDING_MODE();
      // change rounding mode
      _MM_SET_ROUNDING_MODE(_MM_ROUND_DOWN);
      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-15; poSrcStart += 16, poDst += 16) {
        // load 32f values
        __m128 v0 = _mm_load_ps(poSrcStart);
        __m128 v1 = _mm_load_ps(poSrcStart+4);
        __m128 v2 = _mm_load_ps(poSrcStart+8);
        __m128 v3 = _mm_load_ps(poSrcStart+12);

        // convert to 32s values
        __m128i v0i = _mm_cvttps_epi32(v0);
        __m128i v1i = _mm_cvttps_epi32(v1);
        __m128i v2i = _mm_cvttps_epi32(v2);
        __m128i v3i = _mm_cvttps_epi32(v3);

        // convert to 16s values
        __m128i vl = _mm_packs_epi32(v0i, v1i);
        __m128i vh = _mm_packs_epi32(v2i, v3i);

        // convert to 8u values
        _mm_storeu_si128((__m128i*)poDst, _mm_packus_epi16(vl, vh));
      }
      // restore initial rounding mode
      _MM_SET_ROUNDING_MODE(initial_mode);

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < std::numeric_limits<icl8u>::min() ? std::numeric_limits<icl8u>::min() :
        *poSrcStart > std::numeric_limits<icl8u>::max() ? std::numeric_limits<icl8u>::max() :
        static_cast<icl8u>(*poSrcStart);
      }
    }

    // --- from icl32f to icl16s ---
    template <> void convert<icl32f,icl16s>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl16s *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poSrcStart) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < std::numeric_limits<icl16s>::min() ? std::numeric_limits<icl16s>::min() :
        *poSrcStart > std::numeric_limits<icl16s>::max() ? std::numeric_limits<icl16s>::max() :
        static_cast<icl16s>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-7; poSrcStart += 8, poDst += 8) {
        // min and max values of the destination data type
        __m128 vMin = _mm_set1_ps(std::numeric_limits<icl16s>::min());
        __m128 vMax = _mm_set1_ps(std::numeric_limits<icl16s>::max());

        // load 32f values
        __m128 v1  = _mm_load_ps(poSrcStart);
        __m128 v2  = _mm_load_ps(poSrcStart+4);

        // saturate
        v1 = _mm_max_ps(v1, vMin);
        v1 = _mm_min_ps(v1, vMax);
        v2 = _mm_max_ps(v2, vMin);
        v2 = _mm_min_ps(v2, vMax);

        // convert to 32s values; convert to 16s values; store 16s values
        _mm_storeu_si128((__m128i*)poDst, _mm_packs_epi32(_mm_cvttps_epi32(v1), _mm_cvttps_epi32(v2)));
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < std::numeric_limits<icl16s>::min() ? std::numeric_limits<icl16s>::min() :
        *poSrcStart > std::numeric_limits<icl16s>::max() ? std::numeric_limits<icl16s>::max() :
        static_cast<icl16s>(*poSrcStart);
      }
    }

    // --- from icl32f to icl32s ---
    template <> void convert<icl32f,icl32s>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl32s *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poDst) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < std::numeric_limits<icl32s>::min() ? std::numeric_limits<icl32s>::min() :
        *poSrcStart > std::numeric_limits<icl32s>::max() ? std::numeric_limits<icl32s>::max() :
        static_cast<icl32s>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-3; poSrcStart += 4, poDst += 4) {
        // min and max values of the destination data type
        __m128 vMin = _mm_set1_ps(std::numeric_limits<icl32s>::min());
        __m128 vMax = _mm_set1_ps(std::numeric_limits<icl32s>::max());

        // load 32f values
        __m128 v  = _mm_loadu_ps(poSrcStart);

        // saturate
        v = _mm_max_ps(v, vMin);
        v = _mm_min_ps(v, vMax);

        // convert to 32s values; store 32s values
        _mm_store_si128((__m128i*)poDst, _mm_cvttps_epi32(v));
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < std::numeric_limits<icl32s>::min() ? std::numeric_limits<icl32s>::min() :
        *poSrcStart > std::numeric_limits<icl32s>::max() ? std::numeric_limits<icl32s>::max() :
        static_cast<icl32s>(*poSrcStart);
      }
    }

    // --- from icl32f to icl64f ---
    template <> void convert<icl32f,icl64f>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl64f *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poDst) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = static_cast<icl64f>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-3; poSrcStart += 4, poDst += 4) {
        // load 32f values
        __m128 v  = _mm_loadu_ps(poSrcStart);

        // convert to 64f
        __m128d vl = _mm_cvtps_pd(v);
        __m128d vh = _mm_cvtps_pd(_mm_movehl_ps(v, v));

        // store 64f values
        _mm_store_pd(poDst, vl);
        _mm_store_pd(poDst+2, vh);
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = static_cast<icl64f>(*poSrcStart);
      }
    }


    // --- from icl64f to icl32f ---
    template<> void convert<icl64f,icl32f>(const icl64f *poSrcStart,const icl64f *poSrcEnd, icl32f *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poSrcStart) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = static_cast<icl32f>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-3; poSrcStart += 4, poDst += 4) {
        // load 64f values
        __m128d v1 = _mm_load_pd(poSrcStart);
        __m128d v2 = _mm_load_pd(poSrcStart+2);

        // convert to 32f values
        __m128 vl = _mm_cvtpd_ps(v1);
        __m128 vh = _mm_cvtpd_ps(v2);

        // store 32f values
        _mm_storeu_ps(poDst, _mm_shuffle_ps(vl, vh, _MM_SHUFFLE(1, 0, 1, 0)));
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = static_cast<icl32f>(*poSrcStart);
      }
    }

    // --- from icl64f to icl32s ---
    template <> void convert<icl64f,icl32s>(const icl64f *poSrcStart,const icl64f *poSrcEnd, icl32s *poDst) {
      // cast the first unaligned values
      for(; (((uintptr_t)poSrcStart) & 15) && (poSrcStart < poSrcEnd); ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < std::numeric_limits<icl32s>::min() ? std::numeric_limits<icl32s>::min() :
        *poSrcStart > std::numeric_limits<icl32s>::max() ? std::numeric_limits<icl32s>::max() :
        static_cast<icl32s>(*poSrcStart);
      }

      // cast four values at the same time
      for (; poSrcStart < poSrcEnd-3; poSrcStart += 4, poDst += 4) {
        // min and max values of the destination data type
        __m128d vMin = _mm_set1_pd(std::numeric_limits<icl32s>::min());
        __m128d vMax = _mm_set1_pd(std::numeric_limits<icl32s>::max());

        // load 64f values
        __m128d v1 = _mm_load_pd(poSrcStart);
        __m128d v2 = _mm_load_pd(poSrcStart+2);

        // saturate
        v1 = _mm_max_pd(v1, vMin);
        v1 = _mm_min_pd(v1, vMax);
        v2 = _mm_max_pd(v2, vMin);
        v2 = _mm_min_pd(v2, vMax);

        // convert to 32s values
        __m128i vl = _mm_cvttpd_epi32(v1);
        __m128i vh = _mm_cvttpd_epi32(v2);

        // store 32s values
        _mm_storeu_si128((__m128i*)poDst, _mm_add_epi32(vl, _mm_shuffle_epi32(vh, _MM_SHUFFLE(1, 0, 3, 2))));
      }

      // cast the last values
      for(; poSrcStart < poSrcEnd; ++poSrcStart, ++poDst) {
        *poDst = *poSrcStart < std::numeric_limits<icl32s>::min() ? std::numeric_limits<icl32s>::min() :
        *poSrcStart > std::numeric_limits<icl32s>::max() ? std::numeric_limits<icl32s>::max() :
        static_cast<icl32s>(*poSrcStart);
      }
    }
  #endif

    // }}}


  namespace{
      template<class T>
      double channel_mean(const Img<T> &image, int channel, bool roiOnly){
        if(roiOnly && !image.hasFullROI()){
          return math::mean(image.beginROI(channel),image.endROI(channel));
        }else{
          return math::mean(image.begin(channel),image.end(channel));
        }
      }
  #ifdef ICL_HAVE_IPP
      template<> double channel_mean<icl8u>(const Img8u &image, int channel, bool roiOnly){
        icl64f m=0;
        ippiMean_8u_C1R(roiOnly ? image.getROIData(channel) : image.getData(channel),image.getLineStep(),
                        roiOnly ? image.getROISize() : image.getROISize(),&m);
      return m;
      }
      template<> double channel_mean<icl16s>(const Img16s &image, int channel, bool roiOnly){
        icl64f m=0;
        ippiMean_16s_C1R(roiOnly ? image.getROIData(channel) : image.getData(channel),image.getLineStep(),
                         roiOnly ? image.getROISize() : image.getROISize(),&m);
        return m;
      }
      template<> double channel_mean<icl32f>(const Img32f &image, int channel, bool roiOnly){
        icl64f m=0;
      ippiMean_32f_C1R(roiOnly ? image.getROIData(channel) : image.getData(channel),image.getLineStep(),
                       roiOnly ? image.getROISize() : image.getROISize(),&m, ippAlgHintAccurate);
      return m;
      }
  #endif
    }

    std::vector<double> mean(const ImgBase *poImg, int iChannel, bool roiOnly){
      FUNCTION_LOG("");
      std::vector<double> vecMean;
      ICLASSERT_RETURN_VAL(poImg,vecMean);

      int firstChannel = iChannel<0 ? 0 : iChannel;
      int lastChannel = iChannel<0 ? poImg->getChannels()-1 : firstChannel;

      switch(poImg->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                                               \
        case depth##D:                                                           \
          for(int i=firstChannel;i<=lastChannel;++i){                            \
            vecMean.push_back(channel_mean(*poImg->asImg<icl##D>(),i,roiOnly));  \
          }                                                                      \
          break;
        ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
      }
      return vecMean;
    }



    // }}}

    // {{{ variance

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
            ICLASSERT_RETURN_VAL(j<(int)mean.size(),vecVar);                                                 \
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
    // }}}

    // {{{ std-deviation

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
    // }}}

    // {{{ mean-and-std-deviation
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
    // }}}

    // {{{ histogramm functions

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

  #ifdef ICL_HAVE_IPP

  #define COMPUTE_DEFAULT_HISTO_256_TEMPLATE(D)                                                                                         \
      template<> void compute_default_histo_256<icl##D>(const Img##D &image, int c, std::vector<int> &h, bool roiOnly){                  \
        ICLASSERT_RETURN(h.size() == 256);                                                                                               \
        IppStatus sts;                                                                                                                   \
                                                                                                                                         \
        const int nBins = 256;                                                                                                           \
        Ipp32f lowerLevel[] = {0};                                                                                                       \
        Ipp32f upperLevel[] = {255};                                                                                                     \
        int nLevels[] = { nBins+1 };                                                                                                     \
        Ipp32f pLevels[257], *ppLevels[1];                                                                                               \
        int sizeHistObj, sizeBuffer;                                                                                                     \
        IppiHistogramSpec* pHistObj;                                                                                                     \
        Ipp8u* pBuffer;                                                                                                                  \
        Ipp32u pHistVec[nBins];                                                                                                          \
                                                                                                                                         \
        if(roiOnly && !image.hasFullROI()){                                                                                              \
          sts = ippiHistogramGetBufferSize(ipp##D, image.getROISize(), nLevels, 1/*nChan*/, 1/*uniform*/, &sizeHistObj, &sizeBuffer);    \
        }else{                                                                                                                           \
          sts = ippiHistogramGetBufferSize(ipp##D, image.getSize(), nLevels, 1/*nChan*/, 1/*uniform*/, &sizeHistObj, &sizeBuffer);       \
        }                                                                                                                                \
        pHistObj = (IppiHistogramSpec*)ippsMalloc_8u( sizeHistObj );                                                                     \
        pBuffer = (Ipp8u*)ippsMalloc_8u( sizeBuffer );                                                                                   \
        sts = ippiHistogramUniformInit( ipp##D, lowerLevel, upperLevel, nLevels, 1, pHistObj );                                          \
                                                                                                                                         \
        ppLevels[0] = pLevels;                                                                                                           \
        sts = ippiHistogramGetLevels( pHistObj, ppLevels );                                                                              \
                                                                                                                                         \
        if(roiOnly && !image.hasFullROI()){                                                                                              \
          sts = ippiHistogram_##D##_C1R(image.getROIData(c), image.getLineStep(), image.getROISize(), pHistVec, pHistObj, pBuffer );     \
        }else{                                                                                                                           \
          sts = ippiHistogram_##D##_C1R(image.getData(c), image.getLineStep(), image.getSize(), pHistVec, pHistObj, pBuffer );           \
        }                                                                                                                                \
        if ( sts != ippStsNoErr )   WARNING_LOG("IPP Error");                                                                            \
        ippsFree( pHistObj );                                                                                                            \
        ippsFree( pBuffer );                                                                                                             \
                                                                                                                                         \
        int* data = h.data();                                                                                                            \
        data = (int*)pHistVec;                                                                                                           \
                                                                                                                                         \
      }
      COMPUTE_DEFAULT_HISTO_256_TEMPLATE(8u)
      COMPUTE_DEFAULT_HISTO_256_TEMPLATE(16s)


  #define COMPUTE_COMPLEX_HISTO_TEMPLATE(D)                                                                                              \
      template<> void compute_complex_histo(const Img##D &image, int c, std::vector<int> &h, bool roiOnly){                              \
        Range<icl##D> range = image.getMinMax(c);                                                                                        \
        IppStatus sts;                                                                                                                   \
                                                                                                                                         \
        const int nBins = h.size();                                                                                                      \
        Ipp32f lowerLevel[] = {(float)range.minVal};                                                                                     \
        Ipp32f upperLevel[] = {(float)range.maxVal};                                                                                     \
        int nLevels[] = { nBins+1 };                                                                                                     \
        Ipp32f pLevels[nBins+1], *ppLevels[1];                                                                                           \
        int sizeHistObj, sizeBuffer;                                                                                                     \
        IppiHistogramSpec* pHistObj;                                                                                                     \
        Ipp8u* pBuffer;                                                                                                                  \
        Ipp32u pHistVec[nBins];                                                                                                          \
                                                                                                                                         \
        if(roiOnly && !image.hasFullROI()){                                                                                              \
          sts = ippiHistogramGetBufferSize(ipp##D, image.getROISize(), nLevels, 1/*nChan*/, 1/*uniform*/, &sizeHistObj, &sizeBuffer);    \
        }else{                                                                                                                           \
          sts = ippiHistogramGetBufferSize(ipp##D, image.getSize(), nLevels, 1/*nChan*/, 1/*uniform*/, &sizeHistObj, &sizeBuffer);       \
        }                                                                                                                                \
        pHistObj = (IppiHistogramSpec*)ippsMalloc_8u( sizeHistObj );                                                                     \
        pBuffer = (Ipp8u*)ippsMalloc_8u( sizeBuffer );                                                                                   \
        sts = ippiHistogramUniformInit( ipp##D, lowerLevel, upperLevel, nLevels, 1, pHistObj );                                          \
                                                                                                                                         \
        ppLevels[0] = pLevels;                                                                                                           \
        sts = ippiHistogramGetLevels( pHistObj, ppLevels );                                                                              \
                                                                                                                                         \
        if(roiOnly && !image.hasFullROI()){                                                                                              \
          sts = ippiHistogram_##D##_C1R(image.getROIData(c), image.getLineStep(), image.getROISize(), pHistVec, pHistObj, pBuffer );     \
        }else{                                                                                                                           \
          sts = ippiHistogram_##D##_C1R(image.getData(c), image.getLineStep(), image.getSize(), pHistVec, pHistObj, pBuffer );           \
        }                                                                                                                                \
        if ( sts != ippStsNoErr )   WARNING_LOG("IPP Error");                                                                            \
        ippsFree( pHistObj );                                                                                                            \
        ippsFree( pBuffer );                                                                                                             \
                                                                                                                                         \
        int* data = h.data();                                                                                                            \
        data = (int*)pHistVec;                                                                                                           \
                                                                                                                                         \
      }

      COMPUTE_COMPLEX_HISTO_TEMPLATE(8u)
      COMPUTE_COMPLEX_HISTO_TEMPLATE(16s)

  #endif


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

    // }}}


  } // namespace core
} //namespace
