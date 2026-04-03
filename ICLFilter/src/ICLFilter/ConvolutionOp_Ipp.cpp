// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Img.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ipp.h>
#include <vector>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using COp = filter::ConvolutionOp;
  using Op = COp::Op;

  // Build a normalized 32f kernel from the ConvolutionKernel
  std::vector<Ipp32f> buildKernel32f(const filter::ConvolutionKernel &k) {
    int n = k.getSize().getDim();
    std::vector<Ipp32f> kern(n);
    if(k.isFloat()) {
      std::copy(k.getFloatData(), k.getFloatData() + n, kern.begin());
    } else {
      float invFactor = 1.0f / k.getFactor();
      for(int i = 0; i < n; i++)
        kern[i] = k.getIntData()[i] * invFactor;
    }
    return kern;
  }

  // IPP convolution for same-depth, odd-kernel cases
  template<class T, IppDataType DT,
           IppStatus (IPP_DECL *filterFn)(const T*, int, T*, int, IppiSize,
                      IppiBorderType, const T*, const IppiFilterBorderSpec*, Ipp8u*)>
  void ippConvolveSame(const Img<T> &src, Img<T> &dst, COp &op) {
    auto kern = buildKernel32f(op.getKernel());
    IppiSize kernSize = {op.getKernel().getSize().width, op.getKernel().getSize().height};
    IppiSize roiSize = dst.getROISize();
    Point roiOff = op.getROIOffset();

    int specSize = 0, bufSize = 0;
    ippiFilterBorderGetSize(kernSize, roiSize, DT, ipp32f, 1, &specSize, &bufSize);
    std::vector<Ipp8u> specBuf(specSize), workBuf(bufSize);
    IppiFilterBorderSpec *spec = (IppiFilterBorderSpec*)specBuf.data();
    ippiFilterBorderInit_32f(kern.data(), kernSize, DT, 1, ippRndNear, spec);

    T bv[1] = {0};
    for(int c = src.getChannels()-1; c >= 0; --c) {
      const T *pSrc = src.getData(c) + roiOff.y * src.getWidth() + roiOff.x;
      filterFn(pSrc, src.getLineStep(),
               dst.getROIData(c), dst.getLineStep(),
               roiSize, ippBorderRepl, bv, spec, workBuf.data());
    }
  }

  void ipp_convolve(const Image &src, Image &dst, COp &op) {
    Size ks = op.getKernel().getSize();
    bool oddKernel = (ks.width % 2 == 1) && (ks.height % 2 == 1);
    bool sameDepth = src.getDepth() == dst.getDepth();

    // Only use IPP for odd-sized kernels with same src/dst depth.
    // Even kernels have anchor mismatch; mixed depths (8u→16s for Sobel) not implemented.
    if(oddKernel && sameDepth) {
      if(src.getDepth() == depth8u) {
        ippConvolveSame<icl8u, ipp8u, ippiFilterBorder_8u_C1R>(
          src.as8u(), dst.as8u(), op);
        return;
      } else if(src.getDepth() == depth32f) {
        ippConvolveSame<icl32f, ipp32f, ippiFilterBorder_32f_C1R>(
          src.as32f(), dst.as32f(), op);
        return;
      }
    }

    // Delegate unsupported cases to C++ backend
    auto* cpp = COp::prototype()
        .template getSelector<COp::ConvSig>(Op::apply)
        .get(Backend::Cpp);
    cpp->apply(src, dst, op);
  }

  static int _reg = [] {
    auto ipp = COp::prototype().backends(Backend::Ipp);
    ipp.add<COp::ConvSig>(Op::apply, ipp_convolve,
      applicableTo<icl8u, icl32f>, "IPP convolution (8u/32f, odd kernels)");
    return 0;
  }();

} // anon namespace
