/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/WarpOp_OpenCL.cpp              **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Sergius Gaulik                    **
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

  static const int _reg = ImageBackendDispatching::registerStatefulBackend<icl::filter::WarpOp::WarpSig>(
    "WarpOp.warp", Backend::OpenCL,
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
    "OpenCL warp (8u only)"
  );

} // anon namespace

#endif // ICL_HAVE_OPENCL
