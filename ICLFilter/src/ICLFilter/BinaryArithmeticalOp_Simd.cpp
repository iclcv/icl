#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLFilter/BinaryArithmeticalOp.h>

#ifdef ICL_HAVE_SSE2
#include <ICLUtils/SSETypes.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // SSE2 backend for icl32f: add, sub, mul, div, absSub all have direct instructions.
  void simd_apply(const Image &src1, const Image &src2, Image &dst, int optype) {
    src1.visitWith(dst, [&](const auto &s1, auto &d) {
      using T = typename std::remove_reference_t<decltype(s1)>::type;
      if constexpr (std::is_same_v<T, icl32f>) {
        const auto &s2 = src2.as<T>();
        __m128 absMask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
        visitROILinesPerChannel2With(s1, s2, d,
          [optype, absMask](const icl32f *a, const icl32f *b, icl32f *dp, int, int w) {
            int i = 0;
            for(; i <= w - 4; i += 4) {
              __m128 va = _mm_loadu_ps(a+i);
              __m128 vb = _mm_loadu_ps(b+i);
              __m128 r;
              switch(optype) {
                case filter::BinaryArithmeticalOp::addOp:    r = _mm_add_ps(va, vb); break;
                case filter::BinaryArithmeticalOp::subOp:    r = _mm_sub_ps(va, vb); break;
                case filter::BinaryArithmeticalOp::mulOp:    r = _mm_mul_ps(va, vb); break;
                case filter::BinaryArithmeticalOp::divOp:    r = _mm_div_ps(va, vb); break;
                case filter::BinaryArithmeticalOp::absSubOp: r = _mm_and_ps(_mm_sub_ps(va, vb), absMask); break;
                default: r = va; break;
              }
              _mm_storeu_ps(dp+i, r);
            }
            for(; i < w; ++i) {
              switch(optype) {
                case filter::BinaryArithmeticalOp::addOp:    dp[i] = a[i] + b[i]; break;
                case filter::BinaryArithmeticalOp::subOp:    dp[i] = a[i] - b[i]; break;
                case filter::BinaryArithmeticalOp::mulOp:    dp[i] = a[i] * b[i]; break;
                case filter::BinaryArithmeticalOp::divOp:    dp[i] = a[i] / b[i]; break;
                case filter::BinaryArithmeticalOp::absSubOp: dp[i] = std::abs(a[i] - b[i]); break;
                default: dp[i] = a[i]; break;
              }
            }
          });
      }
    });
  }

  using BAOp = icl::filter::BinaryArithmeticalOp;

  static const int _r1 = ImageBackendDispatching::registerBackend<BAOp::Sig>(
    "BinaryArithmeticalOp.apply", Backend::Simd, simd_apply,
    applicableTo<icl32f>, "SSE2/NEON binary arithmetic (32f)");

} // anonymous namespace

#endif // ICL_HAVE_SSE2
