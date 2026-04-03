#include <ICLFilter/MedianOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <vector>
#include <algorithm>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using MOp = filter::MedianOp;

  // ================================================================
  // Sorting network core functions
  // Templated on value type V: works for both scalar types (T)
  // and SIMD types (icl128, icl128i8u, icl128i16s).
  // ADL finds icl::utils::min/max for SIMD, std::min/max for scalars.
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
  // Wrappers: load from pointers, call core, store result.
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
  // Huang histogram median — O(n) per pixel for integer types
  // ================================================================

  template<class T, int HistSize>
  void huangMedian(const Img<T> &src, Img<T> &dst,
                   const Size &maskSize, const Point &roiOffset, const Point &anchor) {
    const int maskW = maskSize.width, maskH = maskSize.height;
    const int half = maskSize.getDim() / 2;
    const int halfexact = (maskSize.getDim() + 1) / 2;
    const int srcW = src.getWidth(), dstW = dst.getWidth();
    const int roiW = dst.getROIWidth(), roiH = dst.getROIHeight();
    const int tlY = roiOffset.y - anchor.y;
    const int tlX0 = roiOffset.x - anchor.x;

    for (int c = 0; c < src.getChannels(); c++) {
      const T *srcData = src.getData(c);
      T *dstROI = dst.getROIData(c);

      for (int k = 0; k < roiW; k++) {
        T median = 0;
        icl16s hist[HistSize] = {0};
        icl16s left = 0;

        const T *s_tl = srcData + tlY * srcW + tlX0 + k;
        for (int my = 0; my < maskH; my++) {
          const T *row = s_tl + my * srcW;
          for (int mx = 0; mx < maskW; mx++) {
            hist[row[mx]]++;
          }
        }

        icl16s sum = 0;
        for (int i = 0; i < HistSize; ++i) {
          sum += hist[i];
          if (sum >= halfexact) {
            dstROI[k] = median = i;
            left = sum - hist[i];
            break;
          }
        }

        const T *sRtl = s_tl;
        const T *sRbl = s_tl + maskH * srcW;

        for (int y = 1; y < roiH; y++, sRtl += srcW, sRbl += srcW) {
          for (int mx = 0; mx < maskW; mx++) {
            hist[sRtl[mx]]--;
            if (sRtl[mx] < median) --left;
            hist[sRbl[mx]]++;
            if (sRbl[mx] < median) ++left;
          }

          if (left > half) {
            do {
              --median;
              left -= hist[median];
            } while(left > half);
          } else {
            while (left + hist[median] < halfexact) {
              left += hist[median];
              ++median;
            }
          }

          dstROI[y * dstW + k] = median;
        }
      }
    }
  }

  // ================================================================
  // Generic sort-based median for arbitrary mask sizes
  // ================================================================

  template<class T>
  void genericMedian(const Img<T> &src, Img<T> &dst, const Size &maskSize,
                     const Point &roiOffset, const Point &anchor) {
    const int maskW = maskSize.width, maskH = maskSize.height;
    const int maskDim = maskSize.getDim();
    std::vector<T> buf(maskDim);
    auto mid = buf.begin() + maskDim / 2;
    const int srcW = src.getWidth(), dstW = dst.getWidth();
    const int roiW = dst.getROIWidth(), roiH = dst.getROIHeight();
    const int baseY = roiOffset.y - anchor.y;
    const int baseX = roiOffset.x - anchor.x;

    for (int c = 0; c < src.getChannels(); c++) {
      const T *srcData = src.getData(c);
      T *dstROI = dst.getROIData(c);
      for (int y = 0; y < roiH; y++) {
        T *dstRow = dstROI + y * dstW;
        for (int x = 0; x < roiW; x++) {
          auto it = buf.begin();
          for (int my = 0; my < maskH; my++) {
            const T *row = srcData + (baseY + y + my) * srcW + (baseX + x);
            for (int mx = 0; mx < maskW; mx++) {
              *it++ = row[mx];
            }
          }
          std::nth_element(buf.begin(), mid, buf.end());
          dstRow[x] = *mid;
        }
      }
    }
  }

  // ================================================================
  // Per-pixel helper: channel/row/col loops with a kernel lambda.
  // ================================================================

  template<class T, class Kernel>
  void applyPerPixel(const Img<T> &src, Img<T> &dst, const Point &roiOffset, Kernel kernel) {
    const int srcW = src.getWidth(), dstW = dst.getWidth();
    const int roiW = dst.getROIWidth(), roiH = dst.getROIHeight();
    for (int c = 0; c < src.getChannels(); c++) {
      const T *srcData = src.getData(c);
      T *dstROI = dst.getROIData(c);
      for (int y = 0; y < roiH; y++) {
        const T *p = srcData + (roiOffset.y + y) * srcW + roiOffset.x;
        T *d = dstROI + y * dstW;
        for (int x = 0; x < roiW; x++) {
          kernel(p + x, d + x, srcW);
        }
      }
    }
  }

  // ================================================================
  // C++ fixed-size median (3x3 and 5x5): scalar sorting networks
  // ================================================================

  template<class T>
  void cppMedianFixed(const Img<T> &src, Img<T> &dst, int maskDim, const Point &roiOffset) {
    if (maskDim == 3) {
      applyPerPixel(src, dst, roiOffset, [](const T *p, T *d, int w) {
        subMedian3x3(p - w - 1, p - 1, p + w - 1, d);
      });
    } else {
      applyPerPixel(src, dst, roiOffset, [](const T *p, T *d, int w) {
        subMedian5x5(p - 2*w - 2, p - w - 2, p - 2, p + w - 2, p + 2*w - 2, d);
      });
    }
  }

  // ================================================================
  // C++ generic median: huangMedian for integer types, sort for others
  // ================================================================

  template<class T> struct GenericMedianImpl {
    static void apply(const Img<T> &src, Img<T> &dst, const Size &maskSize,
                      const Point &roiOffset, const Point &anchor) {
      genericMedian(src, dst, maskSize, roiOffset, anchor);
    }
  };
  template<> struct GenericMedianImpl<icl8u> {
    static void apply(const Img8u &src, Img8u &dst, const Size &maskSize,
                      const Point &roiOffset, const Point &anchor) {
      huangMedian<icl8u, 256>(src, dst, maskSize, roiOffset, anchor);
    }
  };
  template<> struct GenericMedianImpl<icl16s> {
    static void apply(const Img16s &src, Img16s &dst, const Size &maskSize,
                      const Point &roiOffset, const Point &anchor) {
      huangMedian<icl16s, 65536>(src, dst, maskSize, roiOffset, anchor);
    }
  };

  // ================================================================
  // Backend functions
  // ================================================================

  void cpp_median_fixed(const Image &src, Image &dst, int maskDim, const Point &roiOffset) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      cppMedianFixed<T>(s, d, maskDim, roiOffset);
    });
  }

  void cpp_median_generic(const Image &src, Image &dst, const Size &maskSize,
                           const Point &roiOffset, const Point &anchor) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      GenericMedianImpl<T>::apply(s, d, maskSize, roiOffset, anchor);
    });
  }

  // ================================================================
  // Registration into the class prototype
  // ================================================================

  static int _reg = [] {
    using Op = MOp::Op;
    auto cpp = MOp::prototype().backends(Backend::Cpp);
    cpp.add<MOp::MedianFixedSig>(Op::fixed, cpp_median_fixed, "C++ median fixed 3x3/5x5");
    cpp.add<MOp::MedianGenericSig>(Op::generic, cpp_median_generic, "C++ median generic");
    return 0;
  }();

} // anonymous namespace
