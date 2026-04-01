// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Sergius Gaulik

#ifdef ICL_HAVE_OPENCL

#include <ICLFilter/WarpOp.h>
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/CLIncludes.h>
#include <ICLCore/ImageBackendDispatching.h>

using namespace icl::utils;
using namespace icl::core;

namespace {

  static const char* warpKernelSrc =
    "__kernel void warp(const unsigned int mode,                                \n"
    "                   __read_only image2d_t warpX,                            \n"
    "                   __read_only image2d_t warpY,                            \n"
    "                   __read_only image2d_t in,                               \n"
    "                   __write_only image2d_t out) {                           \n"
    "    const int x = get_global_id(0);                                        \n"
    "    const int y = get_global_id(1);                                        \n"
    "    const int w = get_global_size(0);                                      \n"
    "    const int h = get_global_size(1);                                      \n"
    "    if(x && y && x<w-1 && y<h-1) {                                         \n"
    "      const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |              \n"
    "                                CLK_ADDRESS_CLAMP |                        \n"
    "                                CLK_FILTER_LINEAR;                         \n"
    "      const sampler_t samplerN = CLK_NORMALIZED_COORDS_FALSE |             \n"
    "                                 CLK_ADDRESS_CLAMP |                       \n"
    "                                 CLK_FILTER_NEAREST;                       \n"
    "      float4 fX = read_imagef(warpX, sampler, (int2)(x,y));                \n"
    "      float4 fY = read_imagef(warpY, sampler, (int2)(x,y));                \n"
    "      uint4 inPixel = read_imageui(in, samplerN, (float2)(fX.s0, fY.s0));  \n"
    "      write_imageui(out, (int2)(x,y), inPixel.s0);                         \n"
    "  }                                                                        \n"
    "}                                                                          \n";

  struct CLWarpState {
    CLProgram program;
    CLImage2D input;
    CLImage2D output;
    CLImage2D warpX, warpY;
    CLKernel kernel;
    Size mapSize;

    CLWarpState() {
      program = CLProgram("gpu", warpKernelSrc);
      kernel = program.createKernel("warp");
    }

    void updateWarpMap(const Channel32f cwm[2]) {
      Size newSize = cwm[0].getSize();
      if(newSize != mapSize) {
        mapSize = newSize;
        int w = newSize.width;
        int h = newSize.height;
        warpX = program.createImage2D("r", w, h, 3, cwm[0].begin());
        warpY = program.createImage2D("r", w, h, 3, cwm[1].begin());
      }
    }

    void apply(const Image& src, Image& dst, const Channel32f* cwm,
               Point /*warpOffset*/, scalemode mode) {
      int w = src.getWidth();
      int h = src.getHeight();

      cl_filter_mode filterMode;
      if(mode == interpolateNN)
        filterMode = CL_FILTER_NEAREST;
      else if(mode == interpolateLIN)
        filterMode = CL_FILTER_LINEAR;
      else {
        ERROR_LOG("region average interpolation mode does not work with OpenCL");
        return;
      }

      updateWarpMap(cwm);

      input = program.createImage2D("r", w, h, src.getDepth());
      output = program.createImage2D("w", w, h, src.getDepth());

      const ImgBase* srcPtr = src.ptr();
      ImgBase* dstPtr = dst.ptr();
      for(int i = 0; i < srcPtr->getChannels(); ++i) {
        input.write(srcPtr->getDataPtr(i));
        kernel.setArgs(filterMode, warpX, warpY, input, output);
        kernel.apply(w, h, 0);
        output.read(dstPtr->getDataPtr(i));
      }
    }
  };

  using WOp = icl::filter::WarpOp;
  using Op = WOp::Op;

  static int _reg = [] {
    auto ocl = WOp::prototype().backends(Backend::OpenCL);
    ocl.addStateful<WOp::WarpSig>(
      Op::warp,
      []() {
        auto state = std::make_shared<CLWarpState>();
        return [state](const Image& src, Image& dst, const Channel32f* cwm,
                       Point warpOffset, scalemode mode) {
          state->apply(src, dst, cwm, warpOffset, mode);
        };
      },
      [](const Image& src) {
        return src.getDepth() == depth8u && src.hasFullROI();
      },
      "OpenCL warp (8u only)");
    return 0;
  }();

} // anon namespace

#endif // ICL_HAVE_OPENCL
