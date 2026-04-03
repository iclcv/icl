// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Apple Accelerate backend for image scaling (vImageScale).
// Excluded from build when ACCELERATE_FOUND is false (see CMakeLists.txt).
//
// Accelerate.h must be included BEFORE ICL headers to avoid naming conflicts
// (Accelerate defines Point, Size which collide with icl::utils::Point/Size).

#include <Accelerate/Accelerate.h>
#include <ICLCore/ImgOps.h>
#include <ICLCore/Img.h>
#include <ICLUtils/ClippedCast.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // Set up a vImage_Buffer for a single channel with ROI offset and stride.
  template<class T>
  vImage_Buffer make_buf(const Img<T> *img, int ch,
                         const icl::utils::Point &offs, const icl::utils::Size &size) {
    return {
      const_cast<T*>(img->getData(ch)) + offs.y * img->getWidth() + offs.x,
      static_cast<vImagePixelCount>(size.height),
      static_cast<vImagePixelCount>(size.width),
      static_cast<size_t>(img->getWidth()) * sizeof(T)
    };
  }

  // NN scaling fallback (trivial — just integer indexing, no interpolation).
  // Used when mode==interpolateNN since vImageScale only does Lanczos.
  template<class T>
  void nn_scale(const Img<T> *src, int srcC,
                const icl::utils::Point &srcOffs, const icl::utils::Size &srcSize,
                Img<T> *dst, int dstC,
                const icl::utils::Point &dstOffs, const icl::utils::Size &dstSize) {
    const T *sd = src->getData(srcC);
    T *dd = dst->getData(dstC);
    const int sw = src->getWidth(), dw = dst->getWidth();
    const float fSX = float(srcSize.width) / float(dstSize.width);
    const float fSY = float(srcSize.height) / float(dstSize.height);
    for(int yD = 0; yD < dstSize.height; ++yD) {
      int yS = int(srcOffs.y + fSY * yD);
      for(int xD = 0; xD < dstSize.width; ++xD) {
        int xS = int(srcOffs.x + fSX * xD);
        dd[(dstOffs.y + yD) * dw + dstOffs.x + xD] = sd[yS * sw + xS];
      }
    }
  }

  void acc_scaledCopy(const ImgBase& src, int srcC,
                      const icl::utils::Point& srcOffs, const icl::utils::Size& srcSize,
                      ImgBase& dst, int dstC,
                      const icl::utils::Point& dstOffs, const icl::utils::Size& dstSize,
                      scalemode mode) {

    // NN mode: vImageScale uses Lanczos resampling, no NN option.
    // Use inline NN (trivial integer indexing).
    if(mode == interpolateNN) {
      switch(src.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
        case depth##D: nn_scale(src.asImg<icl##D>(), srcC, srcOffs, srcSize, \
                                dst.asImg<icl##D>(), dstC, dstOffs, dstSize); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: break;
      }
      return;
    }

    // LIN and RA modes: use vImageScale (high-quality Lanczos resampling).
    // vImage handles both up/downscaling correctly with Lanczos which is
    // superior to bilinear and region-average.
    vImage_Error err = kvImageNoError;
    switch(src.getDepth()) {
      case depth8u: {
        auto sb = make_buf(src.asImg<icl8u>(), srcC, srcOffs, srcSize);
        auto db = make_buf(dst.asImg<icl8u>(), dstC, dstOffs, dstSize);
        err = vImageScale_Planar8(&sb, &db, nullptr, kvImageNoFlags);
        break;
      }
      case depth16s: {
        auto sb = make_buf(src.asImg<icl16s>(), srcC, srcOffs, srcSize);
        auto db = make_buf(dst.asImg<icl16s>(), dstC, dstOffs, dstSize);
        err = vImageScale_Planar16S(&sb, &db, nullptr, kvImageNoFlags);
        break;
      }
      case depth32f: {
        auto sb = make_buf(src.asImg<icl32f>(), srcC, srcOffs, srcSize);
        auto db = make_buf(dst.asImg<icl32f>(), dstC, dstOffs, dstSize);
        err = vImageScale_PlanarF(&sb, &db, nullptr, kvImageNoFlags);
        break;
      }
      default:
        // icl32s, icl64f: no vImage equivalent — use C++ bilinear inline
        switch(src.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
          case depth##D: { \
            const auto *s = src.asImg<icl##D>(); auto *d = dst.asImg<icl##D>(); \
            const icl##D *sd = s->getData(srcC); \
            const int sw = s->getWidth(); \
            float fSX = (float(srcSize.width)-1) / float(dstSize.width); \
            float fSY = (float(srcSize.height)-1) / float(dstSize.height); \
            for(int yD = 0; yD < dstSize.height; ++yD) { \
              float yS = srcOffs.y + fSY * yD; \
              for(int xD = 0; xD < dstSize.width; ++xD) { \
                float xS = srcOffs.x + fSX * xD; \
                float fX0 = xS - floor(xS), fX1 = 1.0f - fX0; \
                float fY0 = yS - floor(yS), fY1 = 1.0f - fY0; \
                int xl = int(xS), yl = int(yS); \
                const icl##D *p = sd + xl + yl * sw; \
                float a = *p, b = *(p+1); p += sw; \
                float dd2 = *p, c = *(p-1); \
                d->getData(dstC)[(dstOffs.y+yD)*d->getWidth()+dstOffs.x+xD] = \
                  clipped_cast<float,icl##D>(fX1*(fY1*a+fY0*c)+fX0*(fY1*b+fY0*dd2)); \
              } \
            } \
            break; \
          }
          ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
          default: break;
        }
        return;
    }
    if(err != kvImageNoError) {
      WARNING_LOG("vImageScale failed (error " << err << ")");
    }
  }

  // ---- Registration ----
  static int _reg = [] {
    auto acc = ImgOps::instance().backends(Backend::Accelerate);
    acc.add<ImgOps::ScaledCopySig>(ImgOps::Op::scaledCopy, acc_scaledCopy,
        "Accelerate vImageScale (Planar8/16S/32f, Lanczos)");
    return 0;
  }();

} // anonymous namespace
