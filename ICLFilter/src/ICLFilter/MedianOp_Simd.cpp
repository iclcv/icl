#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLUtils/SSEUtils.h>
#include <ICLUtils/Exception.h>
#include <ICLFilter/MedianOp.h>

#ifdef ICL_HAVE_SSE2

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // ================================================================
  // Sorting network core functions (same templates as in MedianOp.cpp,
  // instantiated here with SIMD types icl128, icl128i8u, icl128i16s)
  // ================================================================

  template<class V>
  inline void minmax(V &a, V &b) {
    using std::min; using std::max;
    V t = a; a = min(t, b); b = max(t, b);
  }

  template<class V>
  inline V median3x3_core(V a0, V a1, V a2, V b0, V b1, V b2, V c0, V c1, V c2) {
    using std::min; using std::max;

    V A1 = min(a1, a2); a2 = max(a1, a2);
    a1 = max(a0, A1); a0 = min(a0, A1);
    A1 = min(a1, a2); a2 = max(a1, a2);

    V B1 = min(b1, b2); b2 = max(b1, b2);
    b1 = max(b0, B1); b0 = min(b0, B1);
    B1 = min(b1, b2); b2 = max(b1, b2);

    V C1 = min(c1, c2); c2 = max(c1, c2);
    c1 = max(c0, C1); c0 = min(c0, C1);
    C1 = min(c1, c2); c2 = max(c1, c2);

    a0 = max(a0, max(b0, c0));
    a2 = min(a2, max(b2, c2));
    b1 = min(B1, C1); b2 = max(B1, C1);
    b1 = max(A1, b1);
    a1 = min(b1, b2);

    b1 = min(a1, a2); b2 = max(a1, a2);
    b1 = max(a0, b1);
    return min(b1, b2);
  }

  template<class V>
  inline V median5x5_core(V r00, V r01, V r02, V r03, V r04,
                          V r05, V r06, V r07, V r08, V r09,
                          V r10, V r11, V r12, V r13, V r14,
                          V r15, V r16, V r17, V r18, V r19,
                          V r20, V r21, V r22, V r23, V r24) {
    using std::min; using std::max;

    minmax(r00, r01);
    minmax(r03, r04); minmax(r02, r04); minmax(r02, r03);
    minmax(r06, r07); minmax(r05, r07); minmax(r05, r06);
    minmax(r02, r05);
    minmax(r03, r06); minmax(r00, r06); minmax(r00, r03);
    minmax(r04, r07); minmax(r01, r07); minmax(r01, r04);

    minmax(r09, r10); minmax(r08, r10); minmax(r08, r09);
    minmax(r12, r13); minmax(r11, r13); minmax(r11, r12);
    minmax(r15, r16); minmax(r14, r16); minmax(r14, r15);
    minmax(r11, r14); minmax(r08, r14); minmax(r08, r11);
    minmax(r13, r16); minmax(r10, r16); minmax(r10, r13);

    minmax(r18, r19); minmax(r17, r19); minmax(r17, r18);
    minmax(r21, r22); minmax(r20, r22); minmax(r20, r21);
    minmax(r23, r24);
    minmax(r20, r23); minmax(r17, r23); minmax(r17, r20);
    minmax(r21, r24); minmax(r18, r24); minmax(r18, r21);
    minmax(r19, r22);

    r17 = max(r08, r17);
    minmax(r09, r18); minmax(r00, r18); minmax(r00, r09);
    r09 = max(r00, r09);
    minmax(r10, r19); minmax(r01, r19); minmax(r01, r10);
    minmax(r11, r20); minmax(r02, r20); r11 = max(r02, r11);
    minmax(r12, r21); minmax(r03, r21); minmax(r03, r12);
    minmax(r13, r22); minmax(r04, r22); r04 = min(r04, r22);
    minmax(r04, r13);
    minmax(r14, r23); minmax(r05, r23); minmax(r05, r14);
    minmax(r15, r24); r06 = min(r06, r24); minmax(r06, r15);
    r07 = min(r07, r16); r07 = min(r07, r19);
    r13 = min(r13, r21); r15 = min(r15, r23);
    r07 = min(r07, r13); r07 = min(r07, r15);
    r09 = max(r01, r09); r11 = max(r03, r11);
    r17 = max(r05, r17); r17 = max(r11, r17); r17 = max(r09, r17);
    minmax(r04, r10); minmax(r06, r12); minmax(r07, r14);
    minmax(r04, r06); r07 = max(r04, r07);
    minmax(r12, r14); r10 = min(r10, r14);
    minmax(r06, r07); minmax(r10, r12); minmax(r06, r10);
    r17 = max(r06, r17);
    minmax(r12, r17); r07 = min(r07, r17);
    minmax(r07, r10); minmax(r12, r18);
    r12 = max(r07, r12); r10 = min(r10, r18);
    minmax(r12, r20); r10 = min(r10, r20);

    return max(r10, r12);
  }

  // ================================================================
  // Scalar wrappers (needed by sse_for as the scalar tail handler)
  // ================================================================

  template<class T>
  inline void subMedian3x3(const T *l0, const T *l1, const T *l2, T *med) {
    *med = median3x3_core<T>(l0[0], l0[1], l0[2],
                             l1[0], l1[1], l1[2],
                             l2[0], l2[1], l2[2]);
  }

  template<class T>
  inline void subMedian5x5(const T *l0, const T *l1, const T *l2, const T *l3, const T *l4, T *med) {
    *med = median5x5_core<T>(l0[0], l0[1], l0[2], l0[3], l0[4],
                             l1[0], l1[1], l1[2], l1[3], l1[4],
                             l2[0], l2[1], l2[2], l2[3], l2[4],
                             l3[0], l3[1], l3[2], l3[3], l3[4],
                             l4[0], l4[1], l4[2], l4[3], l4[4]);
  }

  // ================================================================
  // SIMD wrappers: load via SIMD type constructor, store via storeu
  // ================================================================

  template<class T0, class T1>
  inline void subSSEMedian3x3(const T0 *l0, const T0 *l1, const T0 *l2, T0 *med) {
    median3x3_core<T1>(T1(l0), T1(l0+1), T1(l0+2),
                       T1(l1), T1(l1+1), T1(l1+2),
                       T1(l2), T1(l2+1), T1(l2+2)).storeu(med);
  }

  template<class T0, class T1>
  inline void subSSEMedian5x5(const T0 *l0, const T0 *l1, const T0 *l2, const T0 *l3, const T0 *l4, T0 *med) {
    median5x5_core<T1>(T1(l0), T1(l0+1), T1(l0+2), T1(l0+3), T1(l0+4),
                       T1(l1), T1(l1+1), T1(l1+2), T1(l1+3), T1(l1+4),
                       T1(l2), T1(l2+1), T1(l2+2), T1(l2+3), T1(l2+4),
                       T1(l3), T1(l3+1), T1(l3+2), T1(l3+3), T1(l3+4),
                       T1(l4), T1(l4+1), T1(l4+2), T1(l4+3), T1(l4+4)).storeu(med);
  }

  // ================================================================
  // SSE helpers: channel loop + sse_for
  // ================================================================

  template<class T, class T1, long Step>
  void sseMedian3x3(const Img<T> &src, Img<T> &dst, const Point &roiOffset) {
    const int srcW = src.getWidth(), dstW = dst.getWidth();
    const int roiW = dst.getROIWidth(), roiH = dst.getROIHeight();
    for (int c = 0; c < src.getChannels(); c++) {
      const T *s = src.getData(c) + roiOffset.y * srcW + roiOffset.x;
      T *d = dst.getROIData(c);
      sse_for(s - srcW - 1, s - 1, s + srcW - 1, d, d + roiH * dstW,
              (long)srcW, (long)dstW, (long)roiW,
              subMedian3x3<T>, subSSEMedian3x3<T, T1>, Step);
    }
  }

  template<class T, class T1, long Step>
  void sseMedian5x5(const Img<T> &src, Img<T> &dst, const Point &roiOffset) {
    const int srcW = src.getWidth(), dstW = dst.getWidth();
    const int roiW = dst.getROIWidth(), roiH = dst.getROIHeight();
    for (int c = 0; c < src.getChannels(); c++) {
      const T *s = src.getData(c) + roiOffset.y * srcW + roiOffset.x;
      T *d = dst.getROIData(c);
      sse_for(s - 2*srcW - 2, s - srcW - 2, s - 2, s + srcW - 2, s + 2*srcW - 2,
              d, d + roiH * dstW,
              (long)srcW, (long)dstW, (long)roiW,
              subMedian5x5<T>, subSSEMedian5x5<T, T1>, Step);
    }
  }

  // ================================================================
  // Typed dispatch for SIMD fixed median
  // ================================================================

  template<class T> struct SimdMedianFixed {
    static void apply(const Img<T>&, Img<T>&, int, const Point&) {
      throw ICLException("SimdMedianFixed: unsupported depth");
    }
  };

  template<> struct SimdMedianFixed<icl8u> {
    static void apply(const Img8u &src, Img8u &dst, int maskDim, const Point &roiOffset) {
      if (maskDim == 3) sseMedian3x3<icl8u, icl128i8u, 16>(src, dst, roiOffset);
      else              sseMedian5x5<icl8u, icl128i8u, 16>(src, dst, roiOffset);
    }
  };
  template<> struct SimdMedianFixed<icl16s> {
    static void apply(const Img16s &src, Img16s &dst, int maskDim, const Point &roiOffset) {
      if (maskDim == 3) sseMedian3x3<icl16s, icl128i16s, 8>(src, dst, roiOffset);
      else              sseMedian5x5<icl16s, icl128i16s, 8>(src, dst, roiOffset);
    }
  };
  template<> struct SimdMedianFixed<icl32f> {
    static void apply(const Img32f &src, Img32f &dst, int maskDim, const Point &roiOffset) {
      if (maskDim == 3) sseMedian3x3<icl32f, icl128, 4>(src, dst, roiOffset);
      else              sseMedian5x5<icl32f, icl128, 4>(src, dst, roiOffset);
    }
  };

  void simd_median_fixed(const Image &src, Image &dst, int maskDim, const Point &roiOffset) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      SimdMedianFixed<T>::apply(s, d, maskDim, roiOffset);
    });
  }

  using MOp = icl::filter::MedianOp;
  using Op = MOp::Op;

  static int _reg = [] {
    auto simd = MOp::prototype().backends(Backend::Simd);
    simd.add<MOp::MedianFixedSig>(Op::fixed, simd_median_fixed,
      applicableTo<icl8u, icl16s, icl32f>, "SSE2/NEON median 3x3/5x5");
    return 0;
  }();

} // anonymous namespace

#endif // ICL_HAVE_SSE2
