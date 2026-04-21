// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Include Accelerate BEFORE ICL headers to avoid Point/Size name collisions
// between macOS MacTypes.h and icl::utils.
#include <Accelerate/Accelerate.h>

#include <icl/core/ImageBackendDispatching.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>
#include <icl/filter/ConvolutionOp.h>
#include <vector>

namespace {

  namespace u = icl::utils;
  using namespace icl::core;
  using icl::icl32f;

  void acc_convolution(const Image &src, Image &dst, icl::filter::ConvolutionOp &op) {
    const float *kData = op.getKernel().getFloatData();
    const u::Size kSize = op.getMaskSize();
    const u::Point roiOff = op.getROIOffset();

    // vImage requires odd kernel dimensions. Pad even → odd with zeros at
    // bottom/right. The anchor (size/2) is unchanged: (even)/2 == (even+1)/2
    // in integer division, and the zero-padded row/column contributes nothing.
    int kw = kSize.width, kh = kSize.height;
    const bool padW = (kw % 2 == 0), padH = (kh % 2 == 0);
    std::vector<float> padded;
    if(padW || padH) {
      int pw = padW ? kw + 1 : kw;
      int ph = padH ? kh + 1 : kh;
      padded.resize(pw * ph, 0.f);
      for(int r = 0; r < kh; ++r)
        std::copy(kData + r * kw, kData + r * kw + kw, padded.data() + r * pw);
      kData = padded.data();
      kw = pw;
      kh = ph;
    }

    const Img32f &s = src.as32f();
    Img32f &d = dst.as32f();

    for(int c = 0; c < s.getChannels(); ++c) {
      vImage_Buffer srcBuf = {
        const_cast<icl32f*>(s.getData(c)),
        static_cast<vImagePixelCount>(s.getHeight()),
        static_cast<vImagePixelCount>(s.getWidth()),
        static_cast<size_t>(s.getLineStep())
      };
      vImage_Buffer dstBuf = {
        d.getROIData(c),
        static_cast<vImagePixelCount>(d.getROIHeight()),
        static_cast<vImagePixelCount>(d.getROIWidth()),
        static_cast<size_t>(d.getLineStep())
      };

      vImageConvolve_PlanarF(
          &srcBuf, &dstBuf, NULL,
          roiOff.x, roiOff.y,
          kData, kh, kw,
          0.f, kvImageEdgeExtend);
    }
  }

  using COp = icl::filter::ConvolutionOp;

  static int _reg = [] {
    auto acc = COp::prototype().backends(Backend::Accelerate);
    acc.add<COp::ConvSig>(COp::Op::apply, acc_convolution,
      applicableTo<icl::icl32f>, "Accelerate vImage convolution (32f)");
    return 0;
  }();

} // anonymous namespace
