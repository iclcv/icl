// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Sergius Gaulik

#ifdef ICL_HAVE_OPENCL

#include <icl/filter/affine/WarpOp.h>
#include <icl/utils/cl/CLProgram.h>
#include <icl/utils/cl/CLIncludes.h>
#include <icl/core/dispatch/ImageBackendDispatching.h>

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
    "    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |                \n"
    "                              CLK_ADDRESS_CLAMP |                          \n"
    "                              CLK_FILTER_LINEAR;                           \n"
    "    const sampler_t samplerN = CLK_NORMALIZED_COORDS_FALSE |               \n"
    "                               CLK_ADDRESS_CLAMP |                         \n"
    "                               CLK_FILTER_NEAREST;                         \n"
    // Warp EVERY pixel — including the 1-px outer frame. Skipping it left an
    // unwritten black border, which against a non-black background reads as a
    // false edge to a corner detector. The warp-map read uses integer coords
    // (no LINEAR border read) and the in-sampler CLAMPs, so the frame is safe.
    "    float4 fX = read_imagef(warpX, sampler, (int2)(x,y));                  \n"
    "    float4 fY = read_imagef(warpY, sampler, (int2)(x,y));                  \n"
    "    uint4 inPixel = read_imageui(in, samplerN, (float2)(fX.s0, fY.s0));    \n"
    "    write_imageui(out, (int2)(x,y), inPixel.s0);                           \n"
    "}                                                                          \n";

  struct CLWarpState {
    CLProgram program;
    CLImage2D input;
    CLImage2D output;
    CLImage2D warpX, warpY;
    CLKernel kernel;
    Size mapSize;
    unsigned mapVersion = 0;   // version last uploaded (WarpOp starts at 1)

    CLWarpState() {
      program = CLProgram("gpu", warpKernelSrc);
      kernel = program.createKernel("warp");
    }

    void updateWarpMap(const Channel32f cwm[2], unsigned version) {
      // (Re)upload the GPU warp map only when it actually changed: a different
      // size, or a new map of the SAME size pushed via WarpOp::setWarpMap()
      // (which bumps the version — e.g. a live distortion-coefficient slider).
      // Keying on size alone left the GPU using the first map forever.
      const Size newSize = cwm[0].getSize();
      if(version == mapVersion && newSize == mapSize) return;
      mapVersion = version;
      mapSize = newSize;
      warpX = program.createImage2D("r", newSize.width, newSize.height, 3, cwm[0].begin());
      warpY = program.createImage2D("r", newSize.width, newSize.height, 3, cwm[1].begin());
    }

    void apply(const Image& src, Image& dst, const Channel32f* cwm,
               Point /*warpOffset*/, scalemode mode, unsigned version) {
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

      updateWarpMap(cwm, version);

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
                       Point warpOffset, scalemode mode, unsigned version) {
          state->apply(src, dst, cwm, warpOffset, mode, version);
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
