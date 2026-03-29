#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLFilter/BinaryCompareOp.h>

#ifdef ICL_HAVE_SSE2
#include <ICLUtils/SSETypes.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // --- Unsigned 8u compare helpers (SSE2 only has signed cmp) ---
  static inline __m128i unsigned_cmplt_epu8(__m128i a, __m128i b) {
    __m128i bias = _mm_set1_epi8(static_cast<char>(0x80));
    return _mm_cmplt_epi8(_mm_xor_si128(a, bias), _mm_xor_si128(b, bias));
  }
  static inline __m128i unsigned_cmpgt_epu8(__m128i a, __m128i b) {
    __m128i bias = _mm_set1_epi8(static_cast<char>(0x80));
    return _mm_cmpgt_epi8(_mm_xor_si128(a, bias), _mm_xor_si128(b, bias));
  }

  // SSE2 for icl32f: compare and produce 0/255 8u output
  void simd_compare(const Image &src1, const Image &src2, Image &dst, int optype) {
    using Op = filter::BinaryCompareOp;
    src1.visit([&](const auto &s1) {
      using T = typename std::remove_reference_t<decltype(s1)>::type;
      const auto &s2 = src2.as<T>();
      auto &d = dst.as8u();

      if constexpr (std::is_same_v<T, icl32f>) {
        visitROILinesPerChannel2With(s1, s2, d,
          [optype](const icl32f *a, const icl32f *b, icl8u *dp, int, int w) {
            // Process 4 floats → 4 bytes per iteration
            int i = 0;
            for(; i <= w - 4; i += 4) {
              __m128 va = _mm_loadu_ps(a+i);
              __m128 vb = _mm_loadu_ps(b+i);
              __m128 mask;
              switch(optype) {
                case Op::lt:   mask = _mm_cmplt_ps(va, vb); break;
                case Op::lteq: mask = _mm_cmple_ps(va, vb); break;
                case Op::eq:   mask = _mm_cmpeq_ps(va, vb); break;
                case Op::gteq: mask = _mm_cmpge_ps(va, vb); break;
                case Op::gt:   mask = _mm_cmpgt_ps(va, vb); break;
                default: mask = _mm_setzero_ps(); break;
              }
              // Convert 32-bit mask to 8-bit: movmskps gives 4 bits
              int bits = _mm_movemask_ps(mask);
              dp[i+0] = (bits & 1) ? 255 : 0;
              dp[i+1] = (bits & 2) ? 255 : 0;
              dp[i+2] = (bits & 4) ? 255 : 0;
              dp[i+3] = (bits & 8) ? 255 : 0;
            }
            for(; i < w; ++i) {
              switch(optype) {
                case Op::lt:   dp[i] = a[i] < b[i] ? 255 : 0; break;
                case Op::lteq: dp[i] = a[i] <= b[i] ? 255 : 0; break;
                case Op::eq:   dp[i] = a[i] == b[i] ? 255 : 0; break;
                case Op::gteq: dp[i] = a[i] >= b[i] ? 255 : 0; break;
                case Op::gt:   dp[i] = a[i] > b[i] ? 255 : 0; break;
                default: dp[i] = 0; break;
              }
            }
          });
      } else if constexpr (std::is_same_v<T, icl8u>) {
        visitROILinesPerChannel2With(s1, s2, d,
          [optype](const icl8u *a, const icl8u *b, icl8u *dp, int, int w) {
            int i = 0;
            for(; i <= w - 16; i += 16) {
              __m128i va = _mm_loadu_si128(reinterpret_cast<const __m128i*>(a+i));
              __m128i vb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b+i));
              __m128i mask;
              switch(optype) {
                case Op::lt:   mask = unsigned_cmplt_epu8(va, vb); break;
                case Op::lteq: mask = _mm_or_si128(unsigned_cmplt_epu8(va, vb), _mm_cmpeq_epi8(va, vb)); break;
                case Op::eq:   mask = _mm_cmpeq_epi8(va, vb); break;
                case Op::gteq: mask = _mm_or_si128(unsigned_cmpgt_epu8(va, vb), _mm_cmpeq_epi8(va, vb)); break;
                case Op::gt:   mask = unsigned_cmpgt_epu8(va, vb); break;
                default: mask = _mm_setzero_si128(); break;
              }
              // mask is 0xFF or 0x00 per byte — exactly what we need (255 or 0)
              _mm_storeu_si128(reinterpret_cast<__m128i*>(dp+i), mask);
            }
            for(; i < w; ++i) {
              switch(optype) {
                case Op::lt:   dp[i] = a[i] < b[i] ? 255 : 0; break;
                case Op::lteq: dp[i] = a[i] <= b[i] ? 255 : 0; break;
                case Op::eq:   dp[i] = a[i] == b[i] ? 255 : 0; break;
                case Op::gteq: dp[i] = a[i] >= b[i] ? 255 : 0; break;
                case Op::gt:   dp[i] = a[i] > b[i] ? 255 : 0; break;
                default: dp[i] = 0; break;
              }
            }
          });
      }
    });
  }

  using BCOp = icl::filter::BinaryCompareOp;

  static const int _r1 = ImageBackendDispatching::registerBackend<BCOp::CmpSig>(
    "BinaryCompareOp.compare", Backend::Simd, simd_compare,
    applicableTo<icl8u, icl32f>, "SSE2/NEON binary compare (8u/32f)");

} // anonymous namespace

#endif // ICL_HAVE_SSE2
