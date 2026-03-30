#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLFilter/BinaryLogicalOp.h>
#include <ICLUtils/EnumDispatch.h>

#ifdef ICL_HAVE_SSE2
#include <ICLUtils/SSETypes.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using Op = filter::BinaryLogicalOp;

  template<Op::optype OT>
  void simd_apply_typed(const Image &src1, const Image &src2, Image &dst) {
    src1.visitWith(dst, [&](const auto &s1, auto &d) {
      using T = typename std::remove_reference_t<decltype(s1)>::type;
      if constexpr (std::is_integral_v<T>) {
        const auto &s2 = src2.as<T>();
        constexpr int step = 16 / sizeof(T);
        visitROILinesPerChannel2With(s1, s2, d, [](const T *a, const T *b, T *dp, int, int w) {
          int i = 0;
          for(; i <= w - step; i += step) {
            __m128i va = _mm_loadu_si128(reinterpret_cast<const __m128i*>(a+i));
            __m128i vb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b+i));
            __m128i r;
            if constexpr (OT == Op::andOp) r = _mm_and_si128(va, vb);
            else if constexpr (OT == Op::orOp)  r = _mm_or_si128(va, vb);
            else if constexpr (OT == Op::xorOp) r = _mm_xor_si128(va, vb);
            else r = va;
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dp+i), r);
          }
          for(; i < w; ++i) {
            if constexpr (OT == Op::andOp) dp[i] = a[i] & b[i];
            else if constexpr (OT == Op::orOp)  dp[i] = a[i] | b[i];
            else if constexpr (OT == Op::xorOp) dp[i] = a[i] ^ b[i];
            else dp[i] = a[i];
          }
        });
      }
    });
  }

  void simd_apply(const Image &src1, const Image &src2, Image &dst, int optype) {
    dispatchEnum<Op::andOp, Op::orOp, Op::xorOp>(optype, [&](auto tag) {
      simd_apply_typed<decltype(tag)::value>(src1, src2, dst);
    });
  }

  using BLOp = icl::filter::BinaryLogicalOp;

  static int _reg = [] {
    using Op = BLOp::Op;
    auto& proto = BLOp::prototype();
    proto.addBackend<BLOp::Sig>(Op::apply, Backend::Simd, simd_apply,
      applicableTo<icl8u, icl16s, icl32s>, "SSE2/NEON binary logical (8u/16s/32s)");
    return 0;
  }();

} // anonymous namespace

#endif // ICL_HAVE_SSE2
