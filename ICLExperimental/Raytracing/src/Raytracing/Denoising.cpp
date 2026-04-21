// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "Denoising.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace icl::rt {

// ---- Bilateral filter ----

void denoiseBilateral(const core::Img8u &src, core::Img8u &dst, float strength) {
  int w = src.getWidth(), h = src.getHeight();
  dst = core::Img8u(utils::Size(w, h), core::formatRGB);

  const icl8u *sR = src.getData(0);
  const icl8u *sG = src.getData(1);
  const icl8u *sB = src.getData(2);
  icl8u *dR = dst.getData(0);
  icl8u *dG = dst.getData(1);
  icl8u *dB = dst.getData(2);

  // Range sigma scales with strength: 10 (weak) to 60 (strong)
  float sigma_r = 10.0f + strength * 50.0f;
  float invSigmaR2 = -0.5f / (sigma_r * sigma_r);

  // Spatial: fixed 5x5 kernel with Gaussian weights (sigma_s ~= 1.5)
  static const float spatialW[5][5] = {
    {0.0232f, 0.0338f, 0.0383f, 0.0338f, 0.0232f},
    {0.0338f, 0.0492f, 0.0558f, 0.0492f, 0.0338f},
    {0.0383f, 0.0558f, 0.0632f, 0.0558f, 0.0383f},
    {0.0338f, 0.0492f, 0.0558f, 0.0492f, 0.0338f},
    {0.0232f, 0.0338f, 0.0383f, 0.0338f, 0.0232f},
  };

  #pragma omp parallel for schedule(dynamic, 4)
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int ci = x + y * w;
      float cR = sR[ci], cG = sG[ci], cB = sB[ci];

      float sumR = 0, sumG = 0, sumB = 0, sumW = 0;
      for (int ky = -2; ky <= 2; ky++) {
        int ny = std::max(0, std::min(h - 1, y + ky));
        for (int kx = -2; kx <= 2; kx++) {
          int nx = std::max(0, std::min(w - 1, x + kx));
          int ni = nx + ny * w;
          float dr = sR[ni] - cR, dg = sG[ni] - cG, db = sB[ni] - cB;
          float colorDist2 = dr * dr + dg * dg + db * db;
          float w_range = std::exp(colorDist2 * invSigmaR2);
          float w_total = spatialW[ky + 2][kx + 2] * w_range;
          sumR += w_total * sR[ni];
          sumG += w_total * sG[ni];
          sumB += w_total * sB[ni];
          sumW += w_total;
        }
      }
      dR[ci] = (icl8u)std::min(255.0f, std::max(0.0f, sumR / sumW));
      dG[ci] = (icl8u)std::min(255.0f, std::max(0.0f, sumG / sumW));
      dB[ci] = (icl8u)std::min(255.0f, std::max(0.0f, sumB / sumW));
    }
  }
}

// ---- À-Trous wavelet filter ----
//
// Uses the B3-spline wavelet kernel [1/16, 1/4, 3/8, 1/4, 1/16] applied as
// a separable 2D filter. Each pass uses an increasing step size (1, 2, 4, 8, 16)
// giving exponentially growing spatial support without increasing kernel size.
// An edge-stopping function prevents blurring across color discontinuities.

void denoiseATrous(const core::Img8u &src, core::Img8u &dst, float strength) {
  int w = src.getWidth(), h = src.getHeight();
  int n = w * h;

  // Work in float [0,1] for precision
  std::vector<float> curR(n), curG(n), curB(n);
  std::vector<float> tmpR(n), tmpG(n), tmpB(n);

  const icl8u *sR = src.getData(0);
  const icl8u *sG = src.getData(1);
  const icl8u *sB = src.getData(2);
  for (int i = 0; i < n; i++) {
    curR[i] = sR[i] / 255.0f;
    curG[i] = sG[i] / 255.0f;
    curB[i] = sB[i] / 255.0f;
  }

  // B3-spline 1D kernel weights
  static const float kernel[5] = {1.0f/16, 4.0f/16, 6.0f/16, 4.0f/16, 1.0f/16};

  // Edge-stopping sigma: scales with strength
  // sigma_c in color space [0,1]: 0.02 (weak) to 0.15 (strong)
  float sigma_c = 0.02f + strength * 0.13f;
  float invSigmaC2 = -0.5f / (sigma_c * sigma_c);

  // 5 passes with step sizes 1, 2, 4, 8, 16
  for (int pass = 0; pass < 5; pass++) {
    int step = 1 << pass;

    #pragma omp parallel for schedule(static)
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        int ci = x + y * w;
        float cR = curR[ci], cG = curG[ci], cB = curB[ci];

        float sumR = 0, sumG = 0, sumB = 0, sumW = 0;
        for (int ky = -2; ky <= 2; ky++) {
          int ny = std::max(0, std::min(h - 1, y + ky * step));
          for (int kx = -2; kx <= 2; kx++) {
            int nx = std::max(0, std::min(w - 1, x + kx * step));
            int ni = nx + ny * w;

            float dr = curR[ni] - cR;
            float dg = curG[ni] - cG;
            float db = curB[ni] - cB;
            float colorDist2 = dr * dr + dg * dg + db * db;

            float w_spatial = kernel[ky + 2] * kernel[kx + 2];
            float w_edge = std::exp(colorDist2 * invSigmaC2);
            float wt = w_spatial * w_edge;

            sumR += wt * curR[ni];
            sumG += wt * curG[ni];
            sumB += wt * curB[ni];
            sumW += wt;
          }
        }
        tmpR[ci] = sumR / sumW;
        tmpG[ci] = sumG / sumW;
        tmpB[ci] = sumB / sumW;
      }
    }

    std::swap(curR, tmpR);
    std::swap(curG, tmpG);
    std::swap(curB, tmpB);
  }

  // Convert back to Img8u
  dst = core::Img8u(utils::Size(w, h), core::formatRGB);
  icl8u *dR = dst.getData(0);
  icl8u *dG = dst.getData(1);
  icl8u *dB = dst.getData(2);
  for (int i = 0; i < n; i++) {
    dR[i] = (icl8u)std::min(255.0f, std::max(0.0f, curR[i] * 255.0f));
    dG[i] = (icl8u)std::min(255.0f, std::max(0.0f, curG[i] * 255.0f));
    dB[i] = (icl8u)std::min(255.0f, std::max(0.0f, curB[i] * 255.0f));
  }
}

} // namespace icl::rt
