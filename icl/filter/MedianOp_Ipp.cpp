// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/ImageBackendDispatching.h>
#include <icl/core/Image.h>
#include <icl/filter/MedianOp.h>
#include <ipp.h>
#include <vector>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using MOp = filter::MedianOp;
  using Op = MOp::Op;

  // IPP median for a single typed image pair, all channels.
  // pSrc points to the source pixel corresponding to the first output pixel;
  // IPP reads mask-radius pixels outside this for border handling.
  template<class T>
  void ippMedianImpl(const Img<T> &src, Img<T> &dst,
                     IppiSize maskSz, const Point &roiOffset);

  template<>
  void ippMedianImpl(const Img<icl8u> &src, Img<icl8u> &dst,
                     IppiSize maskSz, const Point &roiOffset) {
    int bufSize = 0;
    ippiFilterMedianBorderGetBufferSize(dst.getROISize(), maskSz, ipp8u, 1, &bufSize);
    std::vector<Ipp8u> buf(bufSize);
    for(int c = src.getChannels()-1; c >= 0; --c) {
      const Ipp8u *pSrc = src.getData(c) + roiOffset.y * src.getWidth() + roiOffset.x;
      ippiFilterMedianBorder_8u_C1R(pSrc, src.getLineStep(),
                                    dst.getROIData(c), dst.getLineStep(),
                                    dst.getROISize(), maskSz,
                                    ippBorderRepl, 0, buf.data());
    }
  }

  template<>
  void ippMedianImpl(const Img<icl16s> &src, Img<icl16s> &dst,
                     IppiSize maskSz, const Point &roiOffset) {
    int bufSize = 0;
    ippiFilterMedianBorderGetBufferSize(dst.getROISize(), maskSz, ipp16s, 1, &bufSize);
    std::vector<Ipp8u> buf(bufSize);
    for(int c = src.getChannels()-1; c >= 0; --c) {
      const Ipp16s *pSrc = src.getData(c) + roiOffset.y * src.getWidth() + roiOffset.x;
      ippiFilterMedianBorder_16s_C1R(pSrc, src.getLineStep(),
                                     dst.getROIData(c), dst.getLineStep(),
                                     dst.getROISize(), maskSz,
                                     ippBorderRepl, 0, buf.data());
    }
  }

  // Dispatch: fixed (3x3 or 5x5)
  void ipp_median_fixed(const Image &src, Image &dst, int maskDim, const Point &roiOffset) {
    IppiSize maskSz = {maskDim, maskDim};
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u> || std::is_same_v<T, icl16s>)
        ippMedianImpl(s, d, maskSz, roiOffset);
    });
  }

  // Dispatch: generic (arbitrary mask size)
  void ipp_median_generic(const Image &src, Image &dst, const Size &maskSize,
                           const Point &roiOffset, const Point &anchor) {
    (void)anchor; // IPP uses centered anchor by default
    IppiSize maskSz = {maskSize.width, maskSize.height};
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u> || std::is_same_v<T, icl16s>)
        ippMedianImpl(s, d, maskSz, roiOffset);
    });
  }

  static int _reg = [] {
    auto ipp = MOp::prototype().backends(Backend::Ipp);
    ipp.add<MOp::MedianFixedSig>(Op::fixed, ipp_median_fixed,
      applicableTo<icl8u, icl16s>, "IPP median fixed (8u/16s)");
    ipp.add<MOp::MedianGenericSig>(Op::generic, ipp_median_generic,
      applicableTo<icl8u, icl16s>, "IPP median generic (8u/16s)");
    return 0;
  }();

} // anon namespace
