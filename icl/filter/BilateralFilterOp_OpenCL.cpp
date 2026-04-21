// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Tobias Roehlig, Christof Elbrechter

#ifdef ICL_HAVE_OPENCL

#include <icl/filter/BilateralFilterOp.h>
#include <icl/utils/CLProgram.h>
#include <icl/utils/CLIncludes.h>
#include <icl/core/ImageBackendDispatching.h>
#include <icl/filter/OpenCL/BilateralFilterOpKernel.h>

using namespace icl::utils;
using namespace icl::core;

namespace {

  struct CLBilateralState {

    CLProgram program;
    CLKernel rgb_to_lab;
    CLKernel filter_mono;
    CLKernel filter_mono_float;
    CLKernel filter_color;

    // Cached buffer state for reuse
    CLImage2D image_buffer_in, image_buffer_out;
    CLImage2D in_r, in_g, in_b;
    CLImage2D lab_l, lab_a, lab_b;
    CLImage2D out_r, out_g, out_b;
    int cachedWidth = 0, cachedHeight = 0;
    int cachedDepth = -1, cachedChannels = 0;

    CLBilateralState() {
      std::string src = icl::filter::BilateralFilterOpKernelSource;
      program = CLProgram("gpu", src);
      rgb_to_lab = program.createKernel("rgbToLABCIE");
      filter_mono = program.createKernel("bilateral_filter_mono");
      filter_mono_float = program.createKernel("bilateral_filter_mono_float");
      filter_color = program.createKernel("bilateral_filter_color");
    }

    bool needsBufferReset(int w, int h, int d, int ch) const {
      return w != cachedWidth || h != cachedHeight
          || d != cachedDepth || ch != cachedChannels;
    }

    void apply(const Image& src, Image& dst,
               int radius, float sigma_s, float sigma_r, bool use_lab) {
      int w = src.getWidth();
      int h = src.getHeight();
      int d = static_cast<int>(src.getDepth());
      int ch = src.getChannels();
      bool resetBuf = needsBufferReset(w, h, d, ch);
      cachedWidth = w;
      cachedHeight = h;
      cachedDepth = d;
      cachedChannels = ch;

      const ImgBase* srcPtr = src.ptr();
      ImgBase* dstPtr = dst.ptr();

      if(src.getDepth() == depth8u && ch == 1) {
        applyMono8u(*srcPtr->as8u(), *dstPtr->as8u(), w, h,
                    radius, sigma_s, sigma_r, resetBuf);
      } else if(src.getDepth() == depth32f && ch == 1) {
        applyMonoFloat(*srcPtr->as32f(), *dstPtr->as32f(), w, h,
                       radius, sigma_s, sigma_r, resetBuf);
      } else if(src.getDepth() == depth8u && ch == 3) {
        applyColor8u(*srcPtr->as8u(), *dstPtr->as8u(), w, h,
                     radius, sigma_s, sigma_r, use_lab, resetBuf);
      }
    }

    void applyMono8u(const Img8u& src, Img8u& dst, int w, int h,
                     int radius, float sigma_s, float sigma_r, bool resetBuf) {
      const Channel8u ch = src[0];
      Channel8u ch_out = dst[0];
      if(resetBuf) {
        image_buffer_in = program.createImage2D("r", w, h, depth8u, 1, &ch(0,0));
        image_buffer_out = program.createImage2D("w", w, h, depth8u, 1, 0);
      } else {
        image_buffer_in.write(&ch(0,0));
      }
      filter_mono.setArgs(image_buffer_in, image_buffer_out, w, h,
                          radius, sigma_s, sigma_r);
      filter_mono.apply(w, h);
      filter_mono.finish();
      image_buffer_out.read(&ch_out(0,0));
    }

    void applyMonoFloat(const Img32f& src, Img32f& dst, int w, int h,
                        int radius, float sigma_s, float sigma_r, bool resetBuf) {
      const Channel32f ch = src[0];
      Channel32f ch_out = dst[0];
      if(resetBuf) {
        image_buffer_in = program.createImage2D("r", w, h, depth32f, 1, &ch(0,0));
        image_buffer_out = program.createImage2D("w", w, h, depth32f, 1, 0);
      } else {
        image_buffer_in.write(&ch(0,0));
      }
      filter_mono_float.setArgs(image_buffer_in, image_buffer_out, w, h,
                                radius, sigma_s, sigma_r);
      filter_mono_float.apply(w, h);
      filter_mono_float.finish();
      image_buffer_out.read(&ch_out(0,0));
    }

    void applyColor8u(const Img8u& src, Img8u& dst, int w, int h,
                      int radius, float sigma_s, float sigma_r,
                      bool use_lab, bool resetBuf) {
      const Channel8u r = src[0], g = src[1], b = src[2];
      if(resetBuf) {
        in_r = program.createImage2D("r", w, h, depth8u, 1, &r(0,0));
        in_g = program.createImage2D("r", w, h, depth8u, 1, &g(0,0));
        in_b = program.createImage2D("r", w, h, depth8u, 1, &b(0,0));
        out_r = program.createImage2D("w", w, h, depth8u, 1, 0);
        out_g = program.createImage2D("w", w, h, depth8u, 1, 0);
        out_b = program.createImage2D("w", w, h, depth8u, 1, 0);
        lab_l = program.createImage2D("rw", w, h, depth8u, 1, 0);
        lab_a = program.createImage2D("rw", w, h, depth8u, 1, 0);
        lab_b = program.createImage2D("rw", w, h, depth8u, 1, 0);
      } else {
        in_r.write(&r(0,0));
        in_g.write(&g(0,0));
        in_b.write(&b(0,0));
      }

      if(use_lab) {
        rgb_to_lab.setArgs(in_r, in_g, in_b, lab_l, lab_a, lab_b);
        rgb_to_lab.apply(w, h);
        rgb_to_lab.finish();
        filter_color.setArgs(lab_l, lab_a, lab_b, out_r, out_g, out_b,
                             w, h, radius, sigma_s, sigma_r,
                             static_cast<int>(use_lab));
      } else {
        filter_color.setArgs(in_r, in_g, in_b, out_r, out_g, out_b,
                             w, h, radius, sigma_s, sigma_r,
                             static_cast<int>(use_lab));
      }
      filter_color.apply(w, h);
      filter_color.finish();

      Channel8u _r = dst[0], _g = dst[1], _b = dst[2];
      out_r.read(&_r(0,0));
      out_g.read(&_g(0,0));
      out_b.read(&_b(0,0));
    }
  };

  using BOp = icl::filter::BilateralFilterOp;

  static int _reg = [] {
    using Op = BOp::Op;
    auto ocl = BOp::prototype().backends(Backend::OpenCL);
    ocl.addStateful<BOp::ApplySig>(
      Op::apply,
      []() {
        auto state = std::make_shared<CLBilateralState>();
        return [state](const Image& src, Image& dst,
                       int radius, float sigma_s, float sigma_r, bool use_lab) {
          state->apply(src, dst, radius, sigma_s, sigma_r, use_lab);
        };
      },
      [](const Image& src) {
        depth d = src.getDepth();
        int ch = src.getChannels();
        return src.hasFullROI()
            && ((d == depth8u && (ch == 1 || ch == 3))
                || (d == depth32f && ch == 1));
      },
      "OpenCL bilateral filter (8u mono/3ch, 32f mono)"
    );
    return 0;
  }();

} // anon namespace

#endif // ICL_HAVE_OPENCL
