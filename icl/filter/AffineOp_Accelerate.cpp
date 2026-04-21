// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <Accelerate/Accelerate.h>

#include <icl/core/ImageBackendDispatching.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>
#include <icl/filter/AffineOp.h>
#include <icl/math/FixedMatrix.h>

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
    // vImage's matrix convention is opaque (Apple docs imply backward
    // mapping but passing the raw forward matrix gives a centered image
    // rotated CCW, while the C++ NN fallback — backward mapping with an
    // explicit inverse — rotates CW). Rather than guess the convention,
    // we exploit the fact that the *original* struct layout below is
    // known to produce a centered result: we decompose the forward
    // matrix as an isotropic similarity (scale + rotation + translation),
    // rebuild it with the rotation angle negated, re-run AffineOp's bbox
    // algorithm to get the matching translation, and feed that rebuilt
    // matrix through the same struct layout. This flips the visual
    // rotation direction (CCW → CW, matching NN) without disturbing
    // centering. Works exactly for isotropic similarity transforms, which
    // is all ICL callers use in practice (scale(s,s), rotate, translate).
    const double sc = std::sqrt(fwd[0]*fwd[0] + fwd[3]*fwd[3]);
    const double ang = std::atan2(fwd[3], fwd[0]);
    const double nc = std::cos(-ang) * sc;
    const double ns = std::sin(-ang) * sc;
    const u::Rect roi = src.getROI();

    // Recover the user's translation component: AffineOp may have added
    // the bbox adjustment of the ORIGINAL 2x2 to fwd's translation (when
    // m_adaptResultImage=true). Subtract it to isolate the user's own
    // translate() calls, then add back the bbox adjustment of the NEW
    // (negated-angle) 2x2 so centering still works.
    const double cx[4] = {double(roi.x),
                          double(roi.x + roi.width),
                          double(roi.x + roi.width),
                          double(roi.x)};
    const double cy[4] = {double(roi.y),
                          double(roi.y),
                          double(roi.y + roi.height),
                          double(roi.y + roi.height)};
    auto boundingMin = [&](double a00, double a01, double a10, double a11) {
      double xMin = std::numeric_limits<double>::infinity();
      double yMin = std::numeric_limits<double>::infinity();
      for(int i = 0; i < 4; ++i) {
        const double x = a00*cx[i] + a01*cy[i];
        const double y = a10*cx[i] + a11*cy[i];
        if(x < xMin) xMin = x;
        if(y < yMin) yMin = y;
      }
      return std::pair{xMin, yMin};
    };
    const auto [oldMinX, oldMinY] = boundingMin(fwd[0], fwd[1], fwd[3], fwd[4]);
    const auto [newMinX, newMinY] = boundingMin(nc, -ns, ns, nc);
    // user_tx = fwd[2] - (−oldMinX)  =>  user_tx = fwd[2] + oldMinX
    // new_tx  = user_tx + (−newMinX) =  fwd[2] + oldMinX − newMinX
    const double ntx = fwd[2] + oldMinX - newMinX;
    const double nty = fwd[5] + oldMinY - newMinY;
    // Sub-buffer translation adjustment. srcBuf below starts at the ROI
    // origin, so the transform must index into (roi.x + u, roi.y + v)
    // rather than (x, y) of the full image. Substituting into the
    // column-vector form M·[x, y, 1]ᵀ = A·[u, v]ᵀ + A·[roi.x, roi.y]ᵀ + t
    // gives: the translation for sub-buffer coords is
    //   t' = A·(roi.x, roi.y) + t
    // For identity A this reduces to (roi.x + tx, roi.y + ty), not
    // (tx − roi.x, ty − roi.y) — the previous code had the sign wrong
    // and only worked for identity in special cases.
    const double A00 = nc,  A01 = -ns;
    const double A10 = ns,  A11 = nc;
    const double txAdj = A00 * roi.x + A01 * roi.y + ntx;
    const double tyAdj = A10 * roi.x + A11 * roi.y + nty;
    vImage_AffineTransform transform = {
      static_cast<float>(A00),   static_cast<float>(A10),           // a, b
      static_cast<float>(A01),   static_cast<float>(A11),           // c, d
      static_cast<float>(txAdj), static_cast<float>(tyAdj)          // tx, ty
    };

    const Img<T> &s = src.as<T>();
    Img<T> &d = dst.as<T>();

    for(int c = 0; c < s.getChannels(); ++c) {
      // srcBuf describes only the ROI sub-image: data pointer shifted
      // to the ROI origin, dims = ROI dims, stride = full-image stride.
      // vImage's out-of-bounds detection then fills anything outside
      // the ROI with the background color, matching the C++ path.
      vImage_Buffer srcBuf = {
        const_cast<T*>(s.getData(c)) + roi.y * s.getWidth() + roi.x,
        static_cast<vImagePixelCount>(roi.height),
        static_cast<vImagePixelCount>(roi.width),
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
        inv[i][j] = M(j, i);

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
