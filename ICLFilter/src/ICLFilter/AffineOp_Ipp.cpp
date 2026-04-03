// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Img.h>
#include <ICLFilter/AffineOp.h>
#include <ipp.h>
#include <vector>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using AOp = filter::AffineOp;
  using Op = AOp::Op;

  // Helper: allocate spec, init, get work buffer, apply per channel
  template<class T, IppStatus (IPP_DECL *initFn)(IppiSize, IppiSize, IppDataType, const double[2][3],
                    IppiWarpDirection, int, IppiBorderType, const Ipp64f*, int, IppiWarpSpec*),
           IppStatus (IPP_DECL *applyFn)(const T*, int, T*, int, IppiPoint, IppiSize,
                    const IppiWarpSpec*, Ipp8u*)>
  void ippAffineImpl(const Img<T> &src, Img<T> &dst, const double coeffs[2][3],
                     IppDataType dt, IppiInterpolationType interpType) {
    IppiSize srcSize = {src.getWidth(), src.getHeight()};
    IppiSize dstSize = dst.getROISize();
    IppiPoint dstOffset = {dst.getROI().x, dst.getROI().y};
    Ipp64f borderVal = 0;

    int specSize = 0, initBufSize = 0;
    ippiWarpAffineGetSize(srcSize, dstSize, dt, coeffs, interpType,
                          ippWarpForward, ippBorderConst, &specSize, &initBufSize);
    std::vector<Ipp8u> specBuf(specSize);
    IppiWarpSpec *spec = (IppiWarpSpec*)specBuf.data();
    initFn(srcSize, dstSize, dt, coeffs, ippWarpForward, 1,
           ippBorderConst, &borderVal, 0, spec);

    int bufSize = 0;
    ippiWarpGetBufferSize(spec, dstSize, &bufSize);
    std::vector<Ipp8u> workBuf(bufSize);

    for(int c = src.getChannels()-1; c >= 0; --c) {
      applyFn(src.getData(c), src.getLineStep(),
              dst.getROIData(c), dst.getLineStep(),
              dstOffset, dstSize, spec, workBuf.data());
    }
  }

  void ipp_affine(const Image &src, Image &dst, const double *fwd, scalemode interp) {
    double coeffs[2][3] = {
      {fwd[0], fwd[1], fwd[2]},
      {fwd[3], fwd[4], fwd[5]}
    };

    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u>) {
        if(interp == interpolateLIN)
          ippAffineImpl<icl8u, ippiWarpAffineLinearInit, ippiWarpAffineLinear_8u_C1R>(
            s, d, coeffs, ipp8u, ippLinear);
        else
          ippAffineImpl<icl8u, ippiWarpAffineNearestInit, ippiWarpAffineNearest_8u_C1R>(
            s, d, coeffs, ipp8u, ippNearest);
      } else if constexpr (std::is_same_v<T, icl32f>) {
        if(interp == interpolateLIN)
          ippAffineImpl<icl32f, ippiWarpAffineLinearInit, ippiWarpAffineLinear_32f_C1R>(
            s, d, coeffs, ipp32f, ippLinear);
        else
          ippAffineImpl<icl32f, ippiWarpAffineNearestInit, ippiWarpAffineNearest_32f_C1R>(
            s, d, coeffs, ipp32f, ippNearest);
      }
    });
  }

  static int _reg = [] {
    auto ipp = AOp::prototype().backends(Backend::Ipp);
    ipp.add<AOp::AffineSig>(Op::apply, ipp_affine,
      applicableTo<icl8u, icl32f>, "IPP affine warp (8u/32f)");
    return 0;
  }();

} // anon namespace
