#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLFilter/BinaryLogicalOp.h>

#ifdef ICL_HAVE_SSE2
#include <ICLUtils/SSETypes.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  void simd_apply(const Image &src1, const Image &src2, Image &dst, int optype) {
    src1.visitWith(dst, [&](const auto &s1, auto &d) {
      using T = typename std::remove_reference_t<decltype(s1)>::type;
      if constexpr (std::is_integral_v<T>) {
        const auto &s2 = src2.as<T>();
        constexpr int step = 16 / sizeof(T);
        visitROILinesPerChannel2With(s1, s2, d, [optype](const T *a, const T *b, T *dp, int, int w) {
          int i = 0;
          for(; i <= w - step; i += step) {
            __m128i va = _mm_loadu_si128(reinterpret_cast<const __m128i*>(a+i));
            __m128i vb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b+i));
            __m128i r;
            switch(optype) {
              case filter::BinaryLogicalOp::andOp: r = _mm_and_si128(va, vb); break;
              case filter::BinaryLogicalOp::orOp:  r = _mm_or_si128(va, vb); break;
              case filter::BinaryLogicalOp::xorOp: r = _mm_xor_si128(va, vb); break;
              default: r = va; break;
            }
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dp+i), r);
          }
          for(; i < w; ++i) {
            switch(optype) {
              case filter::BinaryLogicalOp::andOp: dp[i] = a[i] & b[i]; break;
              case filter::BinaryLogicalOp::orOp:  dp[i] = a[i] | b[i]; break;
              case filter::BinaryLogicalOp::xorOp: dp[i] = a[i] ^ b[i]; break;
              default: dp[i] = a[i]; break;
            }
          }
        });
      }
    });
  }

  using BLOp = icl::filter::BinaryLogicalOp;

  static const int _r1 = ImageBackendDispatching::registerBackend<BLOp::Sig>(
    "BinaryLogicalOp.apply", Backend::Simd, simd_apply,
    applicableTo<icl8u, icl16s, icl32s>, "SSE2/NEON binary logical (8u/16s/32s)");

} // anonymous namespace

#endif // ICL_HAVE_SSE2
