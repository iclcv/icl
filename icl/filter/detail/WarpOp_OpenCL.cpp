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
    "__kernel void warp(__read_only image2d_t warpX,                            \n"
    "                   __read_only image2d_t warpY,                            \n"
    "                   __read_only image2d_t in,                               \n"
    "                   __write_only image2d_t out) {                           \n"
    "    const int x = get_global_id(0);                                        \n"
    "    const int y = get_global_id(1);                                        \n"
    // Warp-map fetch: integer coords + NEAREST (the map is a per-output lookup,
    // not a resampled signal). The input is an integer image, so it can only be
    // read NEAREST (read_imageui forbids LINEAR filtering).
    "    const sampler_t mapS = CLK_NORMALIZED_COORDS_FALSE |                   \n"
    "                           CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST; \n"
    "    const sampler_t inS  = CLK_NORMALIZED_COORDS_FALSE |                   \n"
    "                           CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST; \n"
    // Warp EVERY pixel — including the 1-px outer frame (skipping it left an
    // unwritten black border that reads as a false edge to a corner detector).
    "    const float fX = read_imagef(warpX, mapS, (int2)(x,y)).s0;             \n"
    "    const float fY = read_imagef(warpY, mapS, (int2)(x,y)).s0;             \n"
    // Out-of-bounds back-mapping → BLACK. This is the only sensible border:
    // letting the input sampler clamp instead smears the source EDGE pixel
    // (e.g. Cycles' sky) into the whole OOB region. Zero (default) border mode
    // tags OOB outputs with a negative sentinel in the warp map; we also guard
    // the upper bound so the rule holds regardless of the baked map.
    "    const int w = get_image_width(in), h = get_image_height(in);          \n"
    "    uint4 px = (uint4)(0,0,0,0);                                           \n"
    "    if (fX >= 0.0f && fY >= 0.0f && fX <= (float)(w-1) && fY <= (float)(h-1)) \n"
    "        px = read_imageui(in, inS, (float2)(fX, fY));                      \n"
    "    write_imageui(out, (int2)(x,y), px.s0);                                \n"
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

      // The input is an integer image (read_imageui), which only supports
      // NEAREST sampling — so NN and LIN both resolve to NEAREST source
      // fetches here. We just reject the region-average mode.
      if(mode != interpolateNN && mode != interpolateLIN) {
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
        kernel.setArgs(warpX, warpY, input, output);
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
