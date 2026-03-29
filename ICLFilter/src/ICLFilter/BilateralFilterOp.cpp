/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BilateralFilterOp.cpp          **
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

#include <ICLFilter/BilateralFilterOp.h>
#include <ICLCore/CCFunctions.h>
#include <ICLCore/Img.h>

#include <vector>
#include <cmath>
#include <algorithm>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
namespace filter {

  namespace {

    // C++ fallback: brute-force O(r²) per pixel bilateral filter.
    //
    // Possible optimizations (not yet implemented):
    // - Range weight LUT: for 8u, only 256 possible |diff| values — precompute
    //   exp(-d²/(2σr²)) into a 256-entry table instead of calling exp() per neighbor.
    // - Separable approximation: bilateral filter is not truly separable, but the
    //   "bilateral grid" (Paris & Durand 2006) or "raised cosines" approach gives
    //   O(1) per pixel at the cost of accuracy. Suitable for large radii.
    // - SIMD inner loop: the per-neighbor multiply-accumulate is a good target for
    //   SSE2/NEON vectorization, especially the mono path.
    // - OpenMP parallelization: the outer pixel loop is embarrassingly parallel.
    //   Could use #pragma omp parallel for on the y-loop.
    // - Tiled/cache-friendly traversal: for large images and large radii, blocking
    //   the image into tiles that fit in L1/L2 improves memory access patterns.

    // Round-to-nearest for integer types, direct cast for float/double
    template<class T>
    inline T roundCast(float v) {
      if constexpr (std::is_floating_point_v<T>) return static_cast<T>(v);
      else return static_cast<T>(std::round(v));
    }

    // Precompute spatial Gaussian LUT for the given radius and sigma_s.
    // Layout: spatialLUT[(dy+r) * side + (dx+r)] = exp(-dist²/(2*σs²))
    std::vector<float> buildSpatialLUT(int radius, float sigma_s) {
      const int side = 2 * radius + 1;
      std::vector<float> lut(side * side);
      const float inv2ss = -1.f / (2.f * sigma_s * sigma_s);
      for(int dy = -radius; dy <= radius; ++dy) {
        for(int dx = -radius; dx <= radius; ++dx) {
          float dist2 = float(dx * dx + dy * dy);
          lut[(dy + radius) * side + (dx + radius)] = std::exp(dist2 * inv2ss);
        }
      }
      return lut;
    }

    // Bilateral filter for single-channel images (any depth T).
    // Range weight is based on scalar intensity difference.
    template<class T>
    void bilateral_mono(const Img<T>& src, Img<T>& dst,
                        int radius, float sigma_s, float sigma_r) {
      const int w = src.getWidth();
      const int h = src.getHeight();
      const int ch = src.getChannels();
      const float inv2rr = -1.f / (2.f * sigma_r * sigma_r);
      const auto spatialLUT = buildSpatialLUT(radius, sigma_s);
      const int side = 2 * radius + 1;

      for(int c = 0; c < ch; ++c) {
        const T* sdata = src.getData(c);
        T* ddata = dst.getData(c);

        for(int y = 0; y < h; ++y) {
          for(int x = 0; x < w; ++x) {
            float center = static_cast<float>(sdata[y * w + x]);
            float sum = 0.f, wsum = 0.f;

            const int y0 = std::max(0, y - radius);
            const int y1 = std::min(h - 1, y + radius);
            const int x0 = std::max(0, x - radius);
            const int x1 = std::min(w - 1, x + radius);

            for(int ny = y0; ny <= y1; ++ny) {
              for(int nx = x0; nx <= x1; ++nx) {
                float neighbor = static_cast<float>(sdata[ny * w + nx]);
                float diff = center - neighbor;
                float range_w = std::exp(diff * diff * inv2rr);
                float spatial_w = spatialLUT[(ny - y + radius) * side + (nx - x + radius)];
                float weight = spatial_w * range_w;
                sum += weight * neighbor;
                wsum += weight;
              }
            }
            ddata[y * w + x] = roundCast<T>(sum / wsum);
          }
        }
      }
    }

    // Bilateral filter for 3-channel color images with LAB-based range weights.
    // Range distance is computed in LAB, accumulation on original channel values.
    template<class T>
    void bilateral_color_lab(const Img<T>& src, Img<T>& dst,
                             int radius, float sigma_s, float sigma_r) {
      const int w = src.getWidth();
      const int h = src.getHeight();
      const int n = w * h;
      const float inv2rr = -1.f / (2.f * sigma_r * sigma_r);
      const auto spatialLUT = buildSpatialLUT(radius, sigma_s);
      const int side = 2 * radius + 1;

      // Convert src to LAB (all in float)
      std::vector<float> labL(n), labA(n), labB(n);
      const T* r = src.getData(0);
      const T* g = src.getData(1);
      const T* b = src.getData(2);

      for(int i = 0; i < n; ++i) {
        icl32f rf = static_cast<icl32f>(r[i]);
        icl32f gf = static_cast<icl32f>(g[i]);
        icl32f bf = static_cast<icl32f>(b[i]);
        cc_util_rgb_to_lab(rf, gf, bf, labL[i], labA[i], labB[i]);
      }

      T* dr = dst.getData(0);
      T* dg = dst.getData(1);
      T* db = dst.getData(2);

      for(int y = 0; y < h; ++y) {
        for(int x = 0; x < w; ++x) {
          const int idx = y * w + x;
          float cL = labL[idx], cA = labA[idx], cB = labB[idx];
          float sumR = 0, sumG = 0, sumB = 0, wsum = 0;

          const int y0 = std::max(0, y - radius);
          const int y1 = std::min(h - 1, y + radius);
          const int x0 = std::max(0, x - radius);
          const int x1 = std::min(w - 1, x + radius);

          for(int ny = y0; ny <= y1; ++ny) {
            for(int nx = x0; nx <= x1; ++nx) {
              const int nidx = ny * w + nx;
              float dL = cL - labL[nidx];
              float dA = cA - labA[nidx];
              float dB = cB - labB[nidx];
              float range_w = std::exp((dL*dL + dA*dA + dB*dB) * inv2rr);
              float spatial_w = spatialLUT[(ny - y + radius) * side + (nx - x + radius)];
              float weight = spatial_w * range_w;
              sumR += weight * static_cast<float>(r[nidx]);
              sumG += weight * static_cast<float>(g[nidx]);
              sumB += weight * static_cast<float>(b[nidx]);
              wsum += weight;
            }
          }
          dr[idx] = roundCast<T>(sumR / wsum);
          dg[idx] = roundCast<T>(sumG / wsum);
          db[idx] = roundCast<T>(sumB / wsum);
        }
      }
    }

    // Bilateral filter for multi-channel images without LAB.
    // Range distance uses Euclidean distance across all channels.
    template<class T>
    void bilateral_color_direct(const Img<T>& src, Img<T>& dst,
                                int radius, float sigma_s, float sigma_r) {
      const int w = src.getWidth();
      const int h = src.getHeight();
      const int ch = src.getChannels();
      const float inv2rr = -1.f / (2.f * sigma_r * sigma_r);
      const auto spatialLUT = buildSpatialLUT(radius, sigma_s);
      const int side = 2 * radius + 1;

      // Gather channel data pointers
      std::vector<const T*> sdata(ch);
      std::vector<T*> ddata(ch);
      for(int c = 0; c < ch; ++c) {
        sdata[c] = src.getData(c);
        ddata[c] = dst.getData(c);
      }

      std::vector<float> sums(ch);
      for(int y = 0; y < h; ++y) {
        for(int x = 0; x < w; ++x) {
          const int idx = y * w + x;
          std::fill(sums.begin(), sums.end(), 0.f);
          float wsum = 0.f;

          // Center pixel values
          std::vector<float> center(ch);
          for(int c = 0; c < ch; ++c)
            center[c] = static_cast<float>(sdata[c][idx]);

          const int y0 = std::max(0, y - radius);
          const int y1 = std::min(h - 1, y + radius);
          const int x0 = std::max(0, x - radius);
          const int x1 = std::min(w - 1, x + radius);

          for(int ny = y0; ny <= y1; ++ny) {
            for(int nx = x0; nx <= x1; ++nx) {
              const int nidx = ny * w + nx;
              float dist2 = 0.f;
              for(int c = 0; c < ch; ++c) {
                float d = center[c] - static_cast<float>(sdata[c][nidx]);
                dist2 += d * d;
              }
              float range_w = std::exp(dist2 * inv2rr);
              float spatial_w = spatialLUT[(ny - y + radius) * side + (nx - x + radius)];
              float weight = spatial_w * range_w;
              for(int c = 0; c < ch; ++c)
                sums[c] += weight * static_cast<float>(sdata[c][nidx]);
              wsum += weight;
            }
          }
          for(int c = 0; c < ch; ++c)
            ddata[c][idx] = roundCast<T>(sums[c] / wsum);
        }
      }
    }

    void cpp_bilateral(const Image& src, Image& dst,
                       int radius, float sigma_s, float sigma_r, bool use_lab) {
      src.visitWith(dst, [&](const auto& s, auto& d) {
        using T = typename std::remove_reference_t<decltype(s)>::type;
        if(s.getChannels() >= 3 && use_lab) {
          bilateral_color_lab<T>(s, d, radius, sigma_s, sigma_r);
        } else if(s.getChannels() > 1) {
          bilateral_color_direct<T>(s, d, radius, sigma_s, sigma_r);
        } else {
          bilateral_mono<T>(s, d, radius, sigma_s, sigma_r);
        }
      });
    }

    static const int _reg_cpp = ImageBackendDispatching::registerBackend<BilateralFilterOp::ApplySig>(
      "BilateralFilterOp.apply", Backend::Cpp, cpp_bilateral,
      nullptr, "C++ bilateral filter (all depths)");

  } // anon namespace

  BilateralFilterOp::BilateralFilterOp(int radius, float sigma_s, float sigma_r, bool use_lab)
    : m_radius(radius), m_sigmaS(sigma_s), m_sigmaR(sigma_r), m_useLAB(use_lab)
  {
    initDispatching("BilateralFilterOp");
    addSelector<ApplySig>("apply");
  }

  void BilateralFilterOp::apply(const Image &src, Image &dst) {
    if(!prepare(dst, src)) return;

    auto* impl = getSelector<ApplySig>("apply").resolve(src);
    if(!impl) {
      ERROR_LOG("no applicable backend for BilateralFilterOp");
      return;
    }
    impl->apply(src, dst, m_radius, m_sigmaS, m_sigmaR, m_useLAB);
  }

} // namespace filter
} // namespace icl
