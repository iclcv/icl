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
#include <cstring>
#include <algorithm>
#include <type_traits>

#ifdef ICL_HAVE_IPP
#include <ipp.h>
#endif

namespace icl{
  namespace core{

    // --- copy<T>: memcpy-based array copy ---
    template<class T>
    void copy(const T *src, const T *srcEnd, T *dst){
      memcpy(dst, src, static_cast<size_t>(srcEnd - src) * sizeof(T));
    }

    // Explicit instantiation for all 5 depth types
    template void copy(const icl8u*,  const icl8u*,  icl8u*);
    template void copy(const icl16s*, const icl16s*, icl16s*);
    template void copy(const icl32s*, const icl32s*, icl32s*);
    template void copy(const icl32f*, const icl32f*, icl32f*);
    template void copy(const icl64f*, const icl64f*, icl64f*);

#ifdef ICL_HAVE_IPP
    // IPP specializations override the generic memcpy versions
    template<> void copy(const icl8u *s, const icl8u *e, icl8u *d)  { ippsCopy_8u(s, d, static_cast<int>(e-s)); }
    template<> void copy(const icl16s *s, const icl16s *e, icl16s *d){ ippsCopy_16s(s, d, static_cast<int>(e-s)); }
    template<> void copy(const icl32s *s, const icl32s *e, icl32s *d){ ippsCopy_32s(s, d, static_cast<int>(e-s)); }
    template<> void copy(const icl32f *s, const icl32f *e, icl32f *d){ ippsCopy_32f(s, d, static_cast<int>(e-s)); }
    template<> void copy(const icl64f *s, const icl64f *e, icl64f *d){ ippsCopy_64f(s, d, static_cast<int>(e-s)); }
#endif

    // --- convert<S,D>: generic clipped_cast conversion ---
    template<class srcT, class dstT>
    void convert(const srcT *poSrcStart, const srcT *poSrcEnd, dstT *poDst){
      if constexpr (std::is_same_v<srcT, dstT>){
        copy(poSrcStart, poSrcEnd, const_cast<srcT*>(poDst));
      } else {
        std::transform(poSrcStart, poSrcEnd, poDst, utils::clipped_cast<srcT, dstT>);
      }
    }

    // Explicit instantiation for all (src, dst) type pairs
    // Includes all 5 standard depths plus icl8s (used in color conversion)
#define ICL_INSTANTIATE_DEPTH(D)                                        \
    template void convert(const icl8u*,  const icl8u*,  icl##D*);      \
    template void convert(const icl8s*,  const icl8s*,  icl##D*);      \
    template void convert(const icl16s*, const icl16s*, icl##D*);      \
    template void convert(const icl32s*, const icl32s*, icl##D*);      \
    template void convert(const icl32f*, const icl32f*, icl##D*);      \
    template void convert(const icl64f*, const icl64f*, icl##D*);
    ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    // icl8s cross-conversions (from standard depths TO icl8s)
    template void convert(const icl8u*,  const icl8u*,  icl8s*);
    template void convert(const icl16s*, const icl16s*, icl8s*);
    template void convert(const icl32s*, const icl32s*, icl8s*);
    template void convert(const icl32f*, const icl32f*, icl8s*);
    template void convert(const icl64f*, const icl64f*, icl8s*);

  } // namespace core
}

// IPP and SSE2 specializations for convert are in CoreFunctions.cpp
// (they override the generic versions above for specific type pairs)
