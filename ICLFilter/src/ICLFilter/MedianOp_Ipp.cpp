#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLFilter/MedianOp.h>

#ifdef ICL_HAVE_IPP

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // ================================================================
  // IPP median helpers
  // ================================================================

  template<typename T, IppStatus (IPP_DECL *ippiFunc)(const T*, int, T*, int, IppiSize, IppiMaskSize)>
  void ippMedianFixed(const Img<T> &src, Img<T> &dst, const Point &roiOffset, int maskDim) {
    for (int c = 0; c < src.getChannels(); c++) {
      ippiFunc(src.getROIData(c, roiOffset), src.getLineStep(),
               dst.getROIData(c), dst.getLineStep(),
               dst.getROISize(), maskDim == 3 ? ippMskSize3x3 : ippMskSize5x5);
    }
  }

  template<typename T, IppStatus (IPP_DECL *ippiFunc)(const T*, int, T*, int, IppiSize, IppiSize, IppiPoint)>
  void ippMedianGeneric(const Img<T> &src, Img<T> &dst, const Size &maskSize,
                        const Point &roiOffset, const Point &anchor) {
    for (int c = 0; c < src.getChannels(); c++) {
      ippiFunc(src.getROIData(c, roiOffset), src.getLineStep(),
               dst.getROIData(c), dst.getLineStep(),
               dst.getROISize(), maskSize, anchor);
    }
  }

  // ================================================================
  // Typed dispatch for IPP fixed median
  // ================================================================

  template<class T> struct IppMedianFixed;

  template<> struct IppMedianFixed<icl8u> {
    static void apply(const Img8u &src, Img8u &dst, int maskDim, const Point &roiOffset) {
      ippMedianFixed<icl8u, ippiFilterMedianCross_8u_C1R>(src, dst, roiOffset, maskDim);
    }
  };
  template<> struct IppMedianFixed<icl16s> {
    static void apply(const Img16s &src, Img16s &dst, int maskDim, const Point &roiOffset) {
      ippMedianFixed<icl16s, ippiFilterMedianCross_16s_C1R>(src, dst, roiOffset, maskDim);
    }
  };

  void ipp_median_fixed(const Image &src, Image &dst, int maskDim, const Point &roiOffset) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      IppMedianFixed<T>::apply(s, d, maskDim, roiOffset);
    });
  }

  // ================================================================
  // Typed dispatch for IPP generic median
  // ================================================================

  template<class T> struct IppMedianGeneric;

  template<> struct IppMedianGeneric<icl8u> {
    static void apply(const Img8u &src, Img8u &dst, const Size &maskSize,
                      const Point &roiOffset, const Point &anchor) {
      ippMedianGeneric<icl8u, ippiFilterMedian_8u_C1R>(src, dst, maskSize, roiOffset, anchor);
    }
  };
  template<> struct IppMedianGeneric<icl16s> {
    static void apply(const Img16s &src, Img16s &dst, const Size &maskSize,
                      const Point &roiOffset, const Point &anchor) {
      ippMedianGeneric<icl16s, ippiFilterMedian_16s_C1R>(src, dst, maskSize, roiOffset, anchor);
    }
  };

  void ipp_median_generic(const Image &src, Image &dst, const Size &maskSize,
                           const Point &roiOffset, const Point &anchor) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      IppMedianGeneric<T>::apply(s, d, maskSize, roiOffset, anchor);
    });
  }

  // ================================================================
  // Registration
  // ================================================================

  using MOp = icl::filter::MedianOp;

  static const int _r1 = ImageBackendDispatching::registerBackend<MOp::MedianFixedSig>(
    "MedianOp.fixed", Backend::Ipp, ipp_median_fixed,
    applicableTo<icl8u, icl16s>, "IPP median fixed 3x3/5x5 (8u/16s)");

  static const int _r2 = ImageBackendDispatching::registerBackend<MOp::MedianGenericSig>(
    "MedianOp.generic", Backend::Ipp, ipp_median_generic,
    applicableTo<icl8u, icl16s>, "IPP median generic (8u/16s)");

} // anonymous namespace

#endif // ICL_HAVE_IPP
