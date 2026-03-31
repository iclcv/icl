#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLUtils/EnumDispatch.h>

#ifdef ICL_HAVE_SSE2
#include <ICLUtils/SSETypes.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using BOp = filter::BinaryArithmeticalOp;
  using Op = BOp::Op;

  template<BOp::optype OT>
  void simd_apply_typed(const Image &src1, const Image &src2, Image &dst) {
    src1.visitWith(dst, [&](const auto &s1, auto &d) {
      using T = typename std::remove_reference_t<decltype(s1)>::type;
      if constexpr (std::is_same_v<T, icl32f>) {
        const auto &s2 = src2.as<T>();
        __m128 absMask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
        visitROILinesPerChannel2With(s1, s2, d,
          [absMask](const icl32f *a, const icl32f *b, icl32f *dp, int, int w) {
            int i = 0;
            for(; i <= w - 4; i += 4) {
              __m128 va = _mm_loadu_ps(a+i);
              __m128 vb = _mm_loadu_ps(b+i);
              __m128 r;
              if constexpr (OT == BOp::addOp)    r = _mm_add_ps(va, vb);
              else if constexpr (OT == BOp::subOp)    r = _mm_sub_ps(va, vb);
              else if constexpr (OT == BOp::mulOp)    r = _mm_mul_ps(va, vb);
              else if constexpr (OT == BOp::divOp)    r = _mm_div_ps(va, vb);
              else if constexpr (OT == BOp::absSubOp) r = _mm_and_ps(_mm_sub_ps(va, vb), absMask);
              else r = va;
              _mm_storeu_ps(dp+i, r);
            }
            for(; i < w; ++i) {
              if constexpr (OT == BOp::addOp)    dp[i] = a[i] + b[i];
              else if constexpr (OT == BOp::subOp)    dp[i] = a[i] - b[i];
              else if constexpr (OT == BOp::mulOp)    dp[i] = a[i] * b[i];
              else if constexpr (OT == BOp::divOp)    dp[i] = a[i] / b[i];
              else if constexpr (OT == BOp::absSubOp) dp[i] = std::abs(a[i] - b[i]);
              else dp[i] = a[i];
            }
          });
      }
    });
  }

  void simd_apply(const Image &src1, const Image &src2, Image &dst, int optype) {
    dispatchEnum<BOp::addOp, BOp::subOp, BOp::mulOp, BOp::divOp, BOp::absSubOp>(optype, [&](auto tag) {
      simd_apply_typed<decltype(tag)::value>(src1, src2, dst);
    });
  }

  static int _reg = [] {
    auto simd = BOp::prototype().backends(Backend::Simd);
    simd.add<BOp::Sig>(Op::apply, simd_apply,
      applicableTo<icl32f>, "SSE2/NEON binary arithmetic (32f)");
    return 0;
  }();

} // anonymous namespace

#endif // ICL_HAVE_SSE2
