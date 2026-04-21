// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/QuickFilter.h>
#include <icl/qt/QuickContext.h>
#include <icl/core/Img.h>
#include <icl/core/ImgBase.h>
#include <icl/core/CCFunctions.h>
#include <icl/core/CoreFunctions.h>
#include <icl/filter/ConvolutionOp.h>
#include <icl/filter/MedianOp.h>
#include <icl/filter/MorphologicalOp.h>
#include <icl/filter/LUTOp.h>
#include <icl/filter/RotateOp.h>
#include <icl/filter/MirrorOp.h>
#include <icl/filter/FixedConvertOp.h>
#include <icl/filter/PseudoColorOp.h>

#include <map>
#include <cmath>
#include <string>

using namespace icl::core;
using namespace icl::utils;
using namespace icl::filter;

namespace icl::qt {

  namespace {
    /// Pool-backed deep copy: reuses a context buffer instead of heap-allocating
    inline Image poolCopy(const Image &src) {
      if(src.isNull()) return Image();
      auto &ctx = activeContext();
      Image dst = ctx.getBuffer(src.getDepth(),
          ImgParams(src.getSize(), src.getChannels(), src.getFormat()));
      ImgBase *dstPtr = dst.ptr();
      src.ptr()->deepCopy(&dstPtr);
      dst.setTime(src.getTime());
      return dst;
    }

    /// Pool-backed depth conversion (no-op shallow return if depth already matches)
    inline Image poolConvert(const Image &src, depth d) {
      if(src.isNull()) return Image();
      if(src.getDepth() == d) return src;
      Image dst = activeContext().getBuffer(d,
          ImgParams(src.getSize(), src.getChannels(), src.getFormat()));
      src.ptr()->convert(dst.ptr());
      dst.setTime(src.getTime());
      return dst;
    }
  }

  // ---- filter ----

  Image filter(const Image &image, const std::string &filterName) {
    // thread_local: each thread gets its own op instances (UnaryOp::apply is stateful)
    thread_local std::map<std::string, UnaryOp*, std::less<>> ops;
    if(ops.empty()) {
      ops["sobelx"] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelY3x3), true);
      ops["sobely"] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelX3x3), true);
      ops["gauss"]  = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::gauss3x3), true);
      ops["laplace"]= new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::laplace3x3), true);
      ops["median"] = new MedianOp(Size(3,3));
      ops["dilation"]= new MorphologicalOp(MorphologicalOp::dilate);
      ops["erosion"] = new MorphologicalOp(MorphologicalOp::erode);
      ops["opening"] = new MorphologicalOp(MorphologicalOp::openBorder);
      ops["closing"] = new MorphologicalOp(MorphologicalOp::closeBorder);
    }

    UnaryOp *u = ops[filterName];
    if(!u) {
      WARNING_LOG("unknown filter type: " << filterName);
      return Image();
    }
    u->setClipToROI(true);
    u->setCheckOnly(true);

    return activeContext().applyOp(*u, image);
  }

  // ---- blur ----

  Image blur(const Image &image, int maskRadius) {
    if(maskRadius < 1) return copy(image);
    if(maskRadius == 1) return filter(image, "gauss");

    std::vector<int> k(maskRadius * 2 + 1);
    const float sigma2 = 2.0f * (maskRadius / 2) * (maskRadius / 2);
    int sum = 0;
    for(unsigned int i = 0; i < k.size(); ++i) {
      float d = static_cast<int>(i) - maskRadius;
      k[i] = static_cast<int>(255.0f * std::exp(-d * d / sigma2));
      sum += k[i];
    }

    ConvolutionOp c_vert(ConvolutionKernel(k.data(), Size(1, k.size()), iclMax(1, sum), false));
    ConvolutionOp c_horz(ConvolutionKernel(k.data(), Size(k.size(), 1), iclMax(1, sum), false));

    auto &ctx = activeContext();
    Image tmp = ctx.applyOp(c_vert, image);
    return ctx.applyOp(c_horz, tmp);
  }

  // ---- color conversion ----

  Image cc(const Image &image, format fmt) {
    Image src = image;
    if(src.getFormat() == formatMatrix && src.getChannels() == 1)
      src.setFormat(formatGray);

    Image dst = activeContext().getBuffer(image.getDepth(),
        ImgParams(image.getSize(), getChannelsOfFormat(fmt), fmt));
    icl::core::cc(src.ptr(), dst.ptr());
    return dst;
  }

  Image rgb(const Image &image)  { return cc(image, formatRGB);  }
  Image hls(const Image &image)  { return cc(image, formatHLS);  }
  Image lab(const Image &image)  { return cc(image, formatLAB);  }
  Image gray(const Image &image) { return cc(image, formatGray); }

  // ---- scale ----

  Image scale(const Image &image, float factor) {
    return scale(image, static_cast<int>(factor * image.getWidth()),
                        static_cast<int>(factor * image.getHeight()));
  }

  Image scale(const Image &image, int width, int height) {
    Image dst = activeContext().getBuffer(image.getDepth(),
        ImgParams(Size(width, height), image.getChannels(), image.getFormat()));
    ImgBase *dstPtr = dst.ptr();
    image.ptr()->scaledCopy(&dstPtr);
    return dst;
  }

  // ---- channel ----

  Image channel(const Image &image, int ch) {
    Image sel = image.selectChannel(ch);
    Image dst = activeContext().getBuffer(image.getDepth(),
        ImgParams(image.getSize(), 1));
    ImgBase *dstPtr = dst.ptr();
    sel.ptr()->deepCopy(&dstPtr);
    return dst;
  }

  // ---- levels ----

  Image levels(const Image &image, icl8u lvls) {
    return activeContext().applyOp(LUTOp(lvls), poolConvert(image, depth8u));
  }

  // ---- thresh ----

  Image thresh(const Image &image, float threshold) {
    Image result = poolCopy(image);
    result.visit([&](auto &img) {
      using T = typename std::remove_reference_t<decltype(img)>::type;
      const T t = static_cast<T>(threshold);
      for(int c = 0; c < img.getChannels(); ++c) {
        T *data = img.getData(c);
        int dim = img.getDim();
        for(int i = 0; i < dim; ++i) {
          data[i] = static_cast<T>(255) * (data[i] > t);
        }
      }
    });
    return result;
  }

  // ---- copy / copyroi / norm ----

  Image copy(const Image &image) {
    return poolCopy(image);
  }

  Image copyroi(const Image &image) {
    if(image.isNull()) return Image();
    Image dst = activeContext().getBuffer(image.getDepth(),
        ImgParams(image.getROISize(), image.getChannels(), image.getFormat()));
    ImgBase *dstPtr = dst.ptr();
    image.ptr()->deepCopyROI(&dstPtr);
    dst.setTime(image.getTime());
    return dst;
  }

  Image copyroi(const Image &image, const utils::Rect &rect) {
    Image tmp = image;
    tmp.setROI(rect);
    return copyroi(tmp);
  }

  Image norm(const Image &image) {
    Image cpy = poolCopy(image);
    cpy.normalizeAllChannels(Range<icl64f>(0, 255));
    return cpy;
  }

  // ---- rotate ----

  Image rotate(const Image &image, float angleDeg) {
    return activeContext().applyOp(RotateOp(angleDeg), image);
  }

  // ---- flip ----

  Image flipx(const Image &image) { return activeContext().applyOp(MirrorOp(axisVert), image); }
  Image flipy(const Image &image) { return activeContext().applyOp(MirrorOp(axisHorz), image); }

  // ---- fixed_convert ----

  Image fixed_convert(const Image &src, const ImgParams &p, depth d) {
    return activeContext().applyOp(filter::FixedConvertOp(p, d), src);
  }

  // ---- pseudo ----

  Image pseudo(const Image &image, const std::vector<std::pair<float, Color>> &stops,
               int maxValue) {
    if(stops.empty()) {
      return activeContext().applyOp(PseudoColorOp(maxValue), image);
    } else {
      std::vector<PseudoColorOp::Stop> s;
      s.reserve(stops.size());
      for(auto &[pos, col] : stops) s.emplace_back(pos, col);
      return activeContext().applyOp(PseudoColorOp(s, maxValue), image);
    }
  }

} // namespace icl::qt
