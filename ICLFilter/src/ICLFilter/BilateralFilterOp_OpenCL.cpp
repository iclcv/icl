/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BilateralFilterOp_OpenCL.cpp   **
** Module : ICLFilter                                              **
** Authors: Tobias Roehlig, Christof Elbrechter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#ifdef ICL_HAVE_OPENCL

#include <ICLFilter/BilateralFilterOp.h>
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/CLIncludes.h>
#include <ICLCore/BackendDispatch.h>
#include <ICLFilter/OpenCL/BilateralFilterOpKernel.h>

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

  static const int _reg = registerStatefulBackend<icl::filter::BilateralFilterOp::ApplySig>(
    "BilateralFilterOp.apply", Backend::OpenCL,
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

} // anon namespace

#endif // ICL_HAVE_OPENCL
