// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

// Compile-time SIMD availability for FixedMatrix inline specializations.
//
// Unlike BlasOps (runtime dispatch for DynMatrix), this header provides
// direct compile-time access to platform SIMD routines for small fixed-size
// matrices where any dispatch overhead is unacceptable.
//
// Strategy per platform:
//   macOS: Apple SIMD (<simd/simd.h>) for 4x4/2x2 mult, inv, matvec (3-5x faster)
//   Linux: Existing SSE2 specializations for 4x4 float mult/matvec in FixedMatrix.h;
//          clang -O3 auto-vectorizes C++ loops for everything else
//
// Rejected alternatives:
//   cblas/MKL: ~100ns call overhead makes it 25x SLOWER than inline C++ for 4x4
//   3x3 Apple SIMD: simd_float3 padding (48 vs 36 bytes) makes it 10x SLOWER
//
// Benchmark results (Apple M-series, -O3, batch=128 independent ops):
//
//   Operation          float          double
//                   SIMD   C++     SIMD   C++
//   4x4 multiply   1.8    8.6ns   2.8    8.9ns   4.8x / 3.2x
//   4x4 * vec4     0.8    1.4ns   1.4    1.4ns   1.6x / 1.0x
//   4x4 inverse    5.4   20.1ns   9.7   20.8ns   3.7x / 2.1x
//   4x4 det        2.0    2.2ns   2.4    2.3ns   ~1x  (compiler auto-vectorizes)
//   4x4 add        1.0    1.0ns   1.7    1.9ns   ~1x  (compiler auto-vectorizes)
//   4x4 smul       0.8    0.8ns   1.9    1.7ns   ~1x  (compiler auto-vectorizes)
//   full pipeline  2.6    6.5ns   3.9    6.7ns   2.4x / 1.7x
//
// See benchmarks/bench-fixedmatrix.cpp for the measurement code.

#include <cstring>      // memcpy
#include <type_traits>  // is_same_v

// --- Apple SIMD (macOS) ---
#if defined(__APPLE__)
  #include <simd/simd.h>
  #define ICL_HAVE_APPLE_SIMD 1

  namespace icl { namespace math { namespace simd_compat {

    // 4x4 float load/store — direct memcpy (same byte size, 64 bytes)
    inline simd_float4x4 load_4x4(const float *src) {
      simd_float4x4 m;
      std::memcpy(&m, src, 64);
      return m;
    }
    inline void store_4x4(const simd_float4x4 &m, float *dst) {
      std::memcpy(dst, &m, 64);
    }

    // 4x4 double load/store — direct memcpy (same byte size, 128 bytes)
    inline simd_double4x4 load_4x4(const double *src) {
      simd_double4x4 m;
      std::memcpy(&m, src, 128);
      return m;
    }
    inline void store_4x4(const simd_double4x4 &m, double *dst) {
      std::memcpy(dst, &m, 128);
    }

    // 2x2 float load/store — direct memcpy (same byte size, 16 bytes)
    inline simd_float2x2 load_2x2(const float *src) {
      simd_float2x2 m;
      std::memcpy(&m, src, 16);
      return m;
    }
    inline void store_2x2(const simd_float2x2 &m, float *dst) {
      std::memcpy(dst, &m, 16);
    }

    // 2x2 double load/store — direct memcpy (same byte size, 32 bytes)
    inline simd_double2x2 load_2x2(const double *src) {
      simd_double2x2 m;
      std::memcpy(&m, src, 32);
      return m;
    }
    inline void store_2x2(const simd_double2x2 &m, double *dst) {
      std::memcpy(dst, &m, 32);
    }

    // 3x3 float load/store — simd_float3 has padding (16 bytes vs 12),
    // so simd_float3x3 is 48 bytes vs float[9]=36. Must construct from rows.
    inline simd_float3x3 load_3x3(const float *src) {
      return simd_matrix_from_rows(
        simd_make_float3(src[0], src[1], src[2]),
        simd_make_float3(src[3], src[4], src[5]),
        simd_make_float3(src[6], src[7], src[8]));
    }
    inline void store_3x3(const simd_float3x3 &m, float *dst) {
      for (int r = 0; r < 3; ++r) {
        dst[r*3+0] = m.columns[0][r];
        dst[r*3+1] = m.columns[1][r];
        dst[r*3+2] = m.columns[2][r];
      }
    }

    // 3x3 double load/store — same padding issue as float
    inline simd_double3x3 load_3x3(const double *src) {
      return simd_matrix_from_rows(
        simd_make_double3(src[0], src[1], src[2]),
        simd_make_double3(src[3], src[4], src[5]),
        simd_make_double3(src[6], src[7], src[8]));
    }
    inline void store_3x3(const simd_double3x3 &m, double *dst) {
      for (int r = 0; r < 3; ++r) {
        dst[r*3+0] = m.columns[0][r];
        dst[r*3+1] = m.columns[1][r];
        dst[r*3+2] = m.columns[2][r];
      }
    }

    // 4-element vector load/store — direct memcpy (16/32 bytes)
    inline simd_float4 load_vec4(const float *src) {
      simd_float4 v;
      std::memcpy(&v, src, 16);
      return v;
    }
    inline void store_vec4(const simd_float4 &v, float *dst) {
      std::memcpy(dst, &v, 16);
    }
    inline simd_double4 load_vec4(const double *src) {
      simd_double4 v;
      std::memcpy(&v, src, 32);
      return v;
    }
    inline void store_vec4(const simd_double4 &v, double *dst) {
      std::memcpy(dst, &v, 32);
    }

    // --- Element-wise arithmetic helpers ---
    // These use if constexpr to select SIMD or scalar fallback at compile time.
    // For 4x4 and 2x2: memcpy load/store (zero overhead, same byte size).
    // For 3x3: from_rows/extraction (padding prevents memcpy). Included because
    // simd_add/sub/mul are still faster than scalar loops for 9 elements.

    // Note: 3x3 (DIM==9) intentionally excluded — the simd_float3 padding
    // overhead (48 vs 36 bytes) makes Apple SIMD ~10x slower than clang's
    // auto-vectorized C++ loop at -O3. Only 4x4 (DIM==16) and 2x2 (DIM==4)
    // benefit from Apple SIMD for element-wise ops.

    template<class T, unsigned int DIM>
    inline void add(const T* a, const T* b, T* dst) {
      if constexpr (DIM == 16 && std::is_same_v<T,float>) {
        store_4x4(simd_add(load_4x4(a), load_4x4(b)), dst);
      } else if constexpr (DIM == 16 && std::is_same_v<T,double>) {
        store_4x4(simd_add(load_4x4(a), load_4x4(b)), dst);
      } else if constexpr (DIM == 4 && std::is_same_v<T,float>) {
        store_2x2(simd_add(load_2x2(a), load_2x2(b)), dst);
      } else if constexpr (DIM == 4 && std::is_same_v<T,double>) {
        store_2x2(simd_add(load_2x2(a), load_2x2(b)), dst);
      } else {
        for (unsigned int i = 0; i < DIM; ++i) dst[i] = a[i] + b[i];
      }
    }

    template<class T, unsigned int DIM>
    inline void sub(const T* a, const T* b, T* dst) {
      if constexpr (DIM == 16 && std::is_same_v<T,float>) {
        store_4x4(simd_sub(load_4x4(a), load_4x4(b)), dst);
      } else if constexpr (DIM == 16 && std::is_same_v<T,double>) {
        store_4x4(simd_sub(load_4x4(a), load_4x4(b)), dst);
      } else if constexpr (DIM == 4 && std::is_same_v<T,float>) {
        store_2x2(simd_sub(load_2x2(a), load_2x2(b)), dst);
      } else if constexpr (DIM == 4 && std::is_same_v<T,double>) {
        store_2x2(simd_sub(load_2x2(a), load_2x2(b)), dst);
      } else {
        for (unsigned int i = 0; i < DIM; ++i) dst[i] = a[i] - b[i];
      }
    }

    template<class T, unsigned int DIM>
    inline void smul(T scalar, const T* src, T* dst) {
      if constexpr (DIM == 16 && std::is_same_v<T,float>) {
        store_4x4(simd_mul(scalar, load_4x4(src)), dst);
      } else if constexpr (DIM == 16 && std::is_same_v<T,double>) {
        store_4x4(simd_mul(scalar, load_4x4(src)), dst);
      } else if constexpr (DIM == 4 && std::is_same_v<T,float>) {
        store_2x2(simd_mul(scalar, load_2x2(src)), dst);
      } else if constexpr (DIM == 4 && std::is_same_v<T,double>) {
        store_2x2(simd_mul(scalar, load_2x2(src)), dst);
      } else {
        for (unsigned int i = 0; i < DIM; ++i) dst[i] = scalar * src[i];
      }
    }

  }}} // namespace icl::math::simd_compat

#else // non-Apple: scalar-only fallbacks (compiler auto-vectorizes at -O3)

namespace icl::math::simd_compat {

template<class T, unsigned int DIM>
inline void add(const T* a, const T* b, T* dst) {
  for (unsigned int i = 0; i < DIM; ++i) dst[i] = a[i] + b[i];
}

template<class T, unsigned int DIM>
inline void sub(const T* a, const T* b, T* dst) {
  for (unsigned int i = 0; i < DIM; ++i) dst[i] = a[i] - b[i];
}

template<class T, unsigned int DIM>
inline void smul(T scalar, const T* src, T* dst) {
  for (unsigned int i = 0; i < DIM; ++i) dst[i] = scalar * src[i];
}

} // namespace icl::math::simd_compat

#endif
