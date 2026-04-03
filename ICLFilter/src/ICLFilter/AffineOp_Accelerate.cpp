// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <Accelerate/Accelerate.h>

#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLFilter/AffineOp.h>
#include <ICLMath/FixedMatrix.h>

namespace {

  namespace u = icl::utils;
  using namespace icl::core;
  using icl::icl8u;
  using icl::icl32f;
  using AOp = icl::filter::AffineOp;

  // ================================================================
  // vImage affine warp — bilinear interpolation
  // ================================================================

  template<class T>
  void acc_affine_lin(const Image &src, Image &dst, const double *fwd) {
    // vImage_AffineTransform: [x',y',1] = [x,y,1] * [[a,b,0],[c,d,0],[tx,ty,1]]
    // ICL forward: x' = fwd[0]*x + fwd[1]*y + fwd[2],
    //              y' = fwd[3]*x + fwd[4]*y + fwd[5]
    vImage_AffineTransform transform = {
      static_cast<float>(fwd[0]), static_cast<float>(fwd[3]),  // a, b
      static_cast<float>(fwd[1]), static_cast<float>(fwd[4]),  // c, d
      static_cast<float>(fwd[2]), static_cast<float>(fwd[5])   // tx, ty
    };

    const Img<T> &s = src.as<T>();
    Img<T> &d = dst.as<T>();

    for(int c = 0; c < s.getChannels(); ++c) {
      vImage_Buffer srcBuf = {
        const_cast<T*>(s.getData(c)),
        static_cast<vImagePixelCount>(s.getHeight()),
        static_cast<vImagePixelCount>(s.getWidth()),
        static_cast<size_t>(s.getLineStep())
      };
      vImage_Buffer dstBuf = {
        d.getData(c),
        static_cast<vImagePixelCount>(d.getHeight()),
        static_cast<vImagePixelCount>(d.getWidth()),
        static_cast<size_t>(d.getLineStep())
      };

      if constexpr (std::is_same_v<T, icl8u>) {
        Pixel_8888 bg = {0, 0, 0, 0};
        vImageAffineWarp_Planar8(&srcBuf, &dstBuf, NULL, &transform,
                                  bg[0], kvImageBackgroundColorFill);
      } else {
        vImageAffineWarp_PlanarF(&srcBuf, &dstBuf, NULL, &transform,
                                  0.f, kvImageBackgroundColorFill);
      }
    }
  }

  // ================================================================
  // C++ nearest-neighbor fallback (same as AffineOp_Cpp.cpp)
  // ================================================================

  void nn_fallback(const Image &src, Image &dst, const double *fwd) {
    icl::math::FixedMatrix<double,3,3> M(fwd[0], fwd[1], fwd[2],
                                          fwd[3], fwd[4], fwd[5],
                                          0, 0, 1);
    M = M.inv();
    double inv[3][3];
    for(int i = 0; i < 3; ++i)
      for(int j = 0; j < 3; ++j)
        inv[i][j] = M(i,j);

    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      u::Rect dr = d.getROI();
      int sx = dr.x, sy = dr.y, ex = dr.right(), ey = dr.bottom();
      u::Rect r = s.getROI();
      for(int ch = 0; ch < s.getChannels(); ch++) {
        const Channel<T> srcCh = s[ch];
        Channel<T> dstCh = d[ch];
        for(int x = sx; x < ex; ++x) {
          for(int y = sy; y < ey; ++y) {
            float x2 = inv[0][0]*x + inv[1][0]*y + inv[2][0];
            float y2 = inv[0][1]*x + inv[1][1]*y + inv[2][1];
            int x3 = round(x2), y3 = round(y2);
            dstCh(x,y) = r.contains(x3,y3) ? srcCh(x3,y3) : T(0);
          }
        }
      }
    });
  }

  // ================================================================
  // Accelerate backend entry point
  // ================================================================

  void acc_affine(const Image &src, Image &dst, const double *fwd, scalemode interp) {
    if(interp == interpolateLIN) {
      if(src.getDepth() == depth8u)
        acc_affine_lin<icl8u>(src, dst, fwd);
      else
        acc_affine_lin<icl32f>(src, dst, fwd);
    } else {
      nn_fallback(src, dst, fwd);
    }
  }

  static int _reg = [] {
    auto acc = AOp::prototype().backends(Backend::Accelerate);
    acc.add<AOp::AffineSig>(AOp::Op::apply, acc_affine,
      applicableTo<icl8u, icl32f>,
      "Accelerate vImage affine warp (8u/32f bilinear)");
    return 0;
  }();

} // anonymous namespace
