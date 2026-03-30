#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLFilter/BinaryCompareOp.h>
#include <ICLUtils/EnumDispatch.h>

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

  using Op = filter::BinaryCompareOp;

  template<Op::optype OT>
  void simd_compare_typed(const Image &src1, const Image &src2, Image &dst) {
    src1.visit([&](const auto &s1) {
      using T = typename std::remove_reference_t<decltype(s1)>::type;
      const auto &s2 = src2.as<T>();
      auto &d = dst.as8u();

      if constexpr (std::is_same_v<T, icl32f>) {
        visitROILinesPerChannel2With(s1, s2, d,
          [](const icl32f *a, const icl32f *b, icl8u *dp, int, int w) {
            int i = 0;
            for(; i <= w - 4; i += 4) {
              __m128 va = _mm_loadu_ps(a+i);
              __m128 vb = _mm_loadu_ps(b+i);
              __m128 mask;
              if constexpr (OT == Op::lt)   mask = _mm_cmplt_ps(va, vb);
              else if constexpr (OT == Op::lteq) mask = _mm_cmple_ps(va, vb);
              else if constexpr (OT == Op::eq)   mask = _mm_cmpeq_ps(va, vb);
              else if constexpr (OT == Op::gteq) mask = _mm_cmpge_ps(va, vb);
              else if constexpr (OT == Op::gt)   mask = _mm_cmpgt_ps(va, vb);
              else mask = _mm_setzero_ps();
              int bits = _mm_movemask_ps(mask);
              dp[i+0] = (bits & 1) ? 255 : 0;
              dp[i+1] = (bits & 2) ? 255 : 0;
              dp[i+2] = (bits & 4) ? 255 : 0;
              dp[i+3] = (bits & 8) ? 255 : 0;
            }
            for(; i < w; ++i) {
              if constexpr (OT == Op::lt)   dp[i] = a[i] < b[i] ? 255 : 0;
              else if constexpr (OT == Op::lteq) dp[i] = a[i] <= b[i] ? 255 : 0;
              else if constexpr (OT == Op::eq)   dp[i] = a[i] == b[i] ? 255 : 0;
              else if constexpr (OT == Op::gteq) dp[i] = a[i] >= b[i] ? 255 : 0;
              else if constexpr (OT == Op::gt)   dp[i] = a[i] > b[i] ? 255 : 0;
              else dp[i] = 0;
            }
          });
      } else if constexpr (std::is_same_v<T, icl8u>) {
        visitROILinesPerChannel2With(s1, s2, d,
          [](const icl8u *a, const icl8u *b, icl8u *dp, int, int w) {
            int i = 0;
            for(; i <= w - 16; i += 16) {
              __m128i va = _mm_loadu_si128(reinterpret_cast<const __m128i*>(a+i));
              __m128i vb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b+i));
              __m128i mask;
              if constexpr (OT == Op::lt)   mask = unsigned_cmplt_epu8(va, vb);
              else if constexpr (OT == Op::lteq) mask = _mm_or_si128(unsigned_cmplt_epu8(va, vb), _mm_cmpeq_epi8(va, vb));
              else if constexpr (OT == Op::eq)   mask = _mm_cmpeq_epi8(va, vb);
              else if constexpr (OT == Op::gteq) mask = _mm_or_si128(unsigned_cmpgt_epu8(va, vb), _mm_cmpeq_epi8(va, vb));
              else if constexpr (OT == Op::gt)   mask = unsigned_cmpgt_epu8(va, vb);
              else mask = _mm_setzero_si128();
              _mm_storeu_si128(reinterpret_cast<__m128i*>(dp+i), mask);
            }
            for(; i < w; ++i) {
              if constexpr (OT == Op::lt)   dp[i] = a[i] < b[i] ? 255 : 0;
              else if constexpr (OT == Op::lteq) dp[i] = a[i] <= b[i] ? 255 : 0;
              else if constexpr (OT == Op::eq)   dp[i] = a[i] == b[i] ? 255 : 0;
              else if constexpr (OT == Op::gteq) dp[i] = a[i] >= b[i] ? 255 : 0;
              else if constexpr (OT == Op::gt)   dp[i] = a[i] > b[i] ? 255 : 0;
              else dp[i] = 0;
            }
          });
      }
    });
  }

  void simd_compare(const Image &src1, const Image &src2, Image &dst, int optype) {
    dispatchEnum<Op::lt, Op::lteq, Op::eq, Op::gteq, Op::gt>(optype, [&](auto tag) {
      simd_compare_typed<decltype(tag)::value>(src1, src2, dst);
    });
  }

  using BCOp = icl::filter::BinaryCompareOp;

  static int _reg = [] {
    using Op = BCOp::Op;
    auto& proto = BCOp::prototype();
    proto.addBackend<BCOp::CmpSig>(Op::compare, Backend::Simd, simd_compare,
      applicableTo<icl8u, icl32f>, "SSE2/NEON binary compare (8u/32f)");
    return 0;
  }();

} // anonymous namespace

#endif // ICL_HAVE_SSE2
