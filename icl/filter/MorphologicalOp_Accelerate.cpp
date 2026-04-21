// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <Accelerate/Accelerate.h>

#include <icl/core/ImageBackendDispatching.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>
#include <icl/core/ImgBorder.h>
#include <icl/filter/MorphologicalOp.h>
#include <icl/filter/BinaryArithmeticalOp.h>
#include <limits>
#include <vector>

namespace {

  namespace u = icl::utils;
  using namespace icl::core;
  using icl::icl8u;
  using icl::icl32f;
  using MOp = icl::filter::MorphologicalOp;

  // ================================================================
  // vImage dilate/erode — dispatched on depth
  // ================================================================

  void acc_dilate_erode_8u(const Image &src, Image &dst, MOp &op, bool isDilate) {
    const u::Size kSize = op.getMaskSize();
    const u::Point roiOff = op.getROIOffset();
    const icl8u *mask = op.getMask();
    const Img8u &s = src.as8u();
    Img8u &d = dst.as8u();

    for(int c = 0; c < s.getChannels(); ++c) {
      vImage_Buffer srcBuf = {
        const_cast<icl8u*>(s.getData(c)),
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

      if(isDilate)
        vImageDilate_Planar8(&srcBuf, &dstBuf, roiOff.x, roiOff.y,
                              mask, kSize.height, kSize.width, kvImageEdgeExtend);
      else
        vImageErode_Planar8(&srcBuf, &dstBuf, roiOff.x, roiOff.y,
                             mask, kSize.height, kSize.width, kvImageEdgeExtend);
    }
  }

  void acc_dilate_erode_32f(const Image &src, Image &dst, MOp &op, bool isDilate) {
    const u::Size kSize = op.getMaskSize();
    const u::Point roiOff = op.getROIOffset();
    const icl8u *mask = op.getMask();

    // vImage PlanarF morphology uses additive float kernel:
    //   dilate: max(src + kernel), erode: min(src - kernel)
    // Convert binary mask → 0.0 (include) / -INF (exclude)
    int dim = kSize.getDim();
    std::vector<float> fkernel(dim);
    for(int i = 0; i < dim; ++i)
      fkernel[i] = mask[i] ? 0.f : -std::numeric_limits<float>::infinity();

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

      if(isDilate)
        vImageDilate_PlanarF(&srcBuf, &dstBuf, roiOff.x, roiOff.y,
                              fkernel.data(), kSize.height, kSize.width, kvImageEdgeExtend);
      else
        vImageErode_PlanarF(&srcBuf, &dstBuf, roiOff.x, roiOff.y,
                             fkernel.data(), kSize.height, kSize.width, kvImageEdgeExtend);
    }
  }

  void acc_dilate_erode(const Image &src, Image &dst, MOp &op, bool isDilate) {
    if(src.getDepth() == depth8u)
      acc_dilate_erode_8u(src, dst, op, isDilate);
    else
      acc_dilate_erode_32f(src, dst, op, isDilate);
  }

  // ================================================================
  // ROI shrink + border replication helpers
  // ================================================================

  static u::Rect shrink_roi(u::Rect roi, const u::Size &maskSize) {
    int dx = (maskSize.width-1)/2, dy = (maskSize.height-1)/2;
    roi.x+=dx; roi.y+=dy; roi.width-=2*dx; roi.height-=2*dy;
    return roi;
  }

  // ================================================================
  // Main Accelerate morphology backend
  // ================================================================

  void acc_morph(const Image &src, Image &dst, MOp &op) {
    auto ot = op.getOptype();
    u::Size sizeSave;
    std::vector<icl8u> maskSave;

    if(ot == MOp::dilate3x3 || ot == MOp::erode3x3) {
      sizeSave = op.getMaskSize();
      std::copy(op.getMask(), op.getMask()+sizeSave.getDim(), back_inserter(maskSave));
      op.setMask(u::Size(3,3));
    }

    switch(ot) {
      case MOp::dilate:
      case MOp::dilate3x3:
      case MOp::dilateBorderReplicate:
        acc_dilate_erode(src, dst, op, true);
        break;
      case MOp::erode:
      case MOp::erode3x3:
      case MOp::erodeBorderReplicate:
        acc_dilate_erode(src, dst, op, false);
        break;

      case MOp::openBorder:
      case MOp::closeBorder: {
        MOp sub_op(ot==MOp::openBorder ? MOp::erode : MOp::dilate,
                   op.getMaskSize(), op.getMask());
        sub_op.setClipToROI(op.getClipToROI());
        sub_op.setCheckOnly(op.getCheckOnly());
        sub_op.apply(src, op.openingBuffer());
        sub_op.setOptype(ot==MOp::openBorder ? MOp::dilate : MOp::erode);
        sub_op.apply(op.openingBuffer(), dst);
        break;
      }
      case MOp::tophatBorder:
      case MOp::blackhatBorder: {
        MOp sub_op(ot==MOp::tophatBorder ? MOp::openBorder : MOp::closeBorder,
                   op.getMaskSize(), op.getMask());
        sub_op.setClipToROI(op.getClipToROI());
        sub_op.setCheckOnly(op.getCheckOnly());
        sub_op.apply(src, op.openingBuffer());
        icl::filter::BinaryArithmeticalOp sub(icl::filter::BinaryArithmeticalOp::subOp);
        sub.setClipToROI(op.getClipToROI());
        sub.setCheckOnly(op.getCheckOnly());
        u::Rect roi = src.getROI();
        roi = shrink_roi(roi, op.getMaskSize());
        roi = shrink_roi(roi, op.getMaskSize());
        Image srcROIAdapted = src.deepCopy();
        srcROIAdapted.setROI(roi);
        if(ot == MOp::tophatBorder)
          sub.apply(srcROIAdapted, op.openingBuffer(), dst);
        else
          sub.apply(op.openingBuffer(), srcROIAdapted, dst);
        break;
      }
      case MOp::gradientBorder: {
        MOp sub_op(MOp::closeBorder, op.getMaskSize(), op.getMask());
        sub_op.setClipToROI(op.getClipToROI());
        sub_op.setCheckOnly(op.getCheckOnly());
        sub_op.apply(src, op.gradientBuffer1());
        sub_op.setOptype(MOp::openBorder);
        sub_op.apply(src, op.gradientBuffer2());
        icl::filter::BinaryArithmeticalOp sub(icl::filter::BinaryArithmeticalOp::subOp);
        sub.setClipToROI(op.getClipToROI());
        sub.setCheckOnly(op.getCheckOnly());
        sub.apply(op.gradientBuffer1(), op.gradientBuffer2(), dst);
        break;
      }
      default:
        ERROR_LOG("invalid optype: " << static_cast<int>(ot));
    }

    if(!op.getClipToROI() && (ot == MOp::erodeBorderReplicate || ot == MOp::dilateBorderReplicate)) {
      if(src.getDepth() == depth8u) {
        ImgBorder::copy(&dst.as8u());
      } else {
        ImgBorder::copy(&dst.as32f());
      }
    }
    if(ot == MOp::dilate3x3 || ot == MOp::erode3x3) {
      op.setMask(sizeSave, maskSave.data());
    }
  }

  static int _reg = [] {
    auto acc = MOp::prototype().backends(Backend::Accelerate);
    acc.add<MOp::MorphSig>(MOp::Op::apply, acc_morph,
      applicableTo<icl8u, icl32f>,
      "Accelerate vImage morphology (8u/32f)");
    return 0;
  }();

} // anonymous namespace
