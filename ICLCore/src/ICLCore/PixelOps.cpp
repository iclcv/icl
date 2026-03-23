/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
**                                                                 **
** File   : ICLCore/src/ICLCore/PixelOps.cpp                       **
** Module : ICLCore                                                **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
********************************************************************/

#include <ICLCore/PixelOps.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLUtils/SSETypes.h>
#include <cstring>
#include <algorithm>
#include <type_traits>
#include <limits>

namespace icl{
  namespace core{

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

  } // namespace core
}
