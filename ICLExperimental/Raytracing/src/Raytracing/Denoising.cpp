// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "Denoising.h"
#include <algorithm>
#include <cmath>
#include <cstring>
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

// ---- SVGF: Spatiotemporal Variance-Guided Filtering ----
//
// Three stages per frame:
//   1. Temporal reprojection: blend with previous frame using motion vectors
//   2. Variance estimation: compute per-pixel luminance variance from moments
//   3. Guided À-Trous spatial filter: 5 passes using depth + normals + variance

static inline float luminance(float r, float g, float b) {
  return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

static inline float svgfDot3(float ax, float ay, float az,
                             float bx, float by, float bz) {
  return ax*bx + ay*by + az*bz;
}

void denoiseSVGF(const core::Img8u &src, core::Img8u &dst,
                 const float *depth,
                 const float *normalX, const float *normalY, const float *normalZ,
                 const float *reflectivity,
                 const RTRayGenParams &camera,
                 SVGFState &state,
                 float strength) {
  int w = src.getWidth(), h = src.getHeight();
  int n = w * h;

  // Convert input to float [0,1]
  const icl8u *sR = src.getData(0);
  const icl8u *sG = src.getData(1);
  const icl8u *sB = src.getData(2);

  std::vector<float> curR(n), curG(n), curB(n);
  for (int i = 0; i < n; i++) {
    curR[i] = sR[i] / 255.0f;
    curG[i] = sG[i] / 255.0f;
    curB[i] = sB[i] / 255.0f;
  }

  // Resize state if needed
  if (state.width != w || state.height != h) {
    state.width = w;
    state.height = h;
    state.prevR.resize(n, 0); state.prevG.resize(n, 0); state.prevB.resize(n, 0);
    state.prevDepth.resize(n, 0);
    state.prevNX.resize(n, 0); state.prevNY.resize(n, 0); state.prevNZ.resize(n, 0);
    state.prevMeshId.resize(n, -1);
    state.moment1.resize(n, 0);
    state.moment2.resize(n, 0);
    state.historyLen.resize(n, 0);
    state.hasPrevFrame = false;
  }

  // ---- Stage 1: Temporal reprojection ----
  std::vector<float> filtR(n), filtG(n), filtB(n);
  std::vector<float> variance(n, 0);
  std::vector<int> histLen(n, 1);

  if (state.hasPrevFrame) {
    const RTMat4 &Q = state.prevViewProj;

    #pragma omp parallel for schedule(static)
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        int idx = x + y * w;
        float d = depth[idx];

        // Reconstruct world position
        float px = x + 0.5f;
        float py = y + 0.5f;
        const RTMat4 &Qi = camera.invViewProj;
        float dx = Qi.cols[0][0]*px + Qi.cols[1][0]*py + Qi.cols[2][0];
        float dy = Qi.cols[0][1]*px + Qi.cols[1][1]*py + Qi.cols[2][1];
        float dz = Qi.cols[0][2]*px + Qi.cols[1][2]*py + Qi.cols[2][2];
        float dlen = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (dlen > 1e-6f) { dx /= dlen; dy /= dlen; dz /= dlen; }

        float wx = camera.cameraPos.x + d * dx;
        float wy = camera.cameraPos.y + d * dy;
        float wz = camera.cameraPos.z + d * dz;

        // Project into previous frame
        float qx = Q.cols[0][0]*wx + Q.cols[1][0]*wy + Q.cols[2][0]*wz + Q.cols[3][0];
        float qy = Q.cols[0][1]*wx + Q.cols[1][1]*wy + Q.cols[2][1]*wz + Q.cols[3][1];
        float qz = Q.cols[0][2]*wx + Q.cols[1][2]*wy + Q.cols[2][2]*wz + Q.cols[3][2];

        bool valid = false;
        if (std::abs(qz) > 1e-6f) {
          float prevPx = qx / qz;
          float prevPy = qy / qz;
          int ppx = (int)(prevPx);
          int ppy = (int)(prevPy);

          if (ppx >= 0 && ppx < w && ppy >= 0 && ppy < h) {
            int prevIdx = ppx + ppy * w;

            // Consistency checks
            float depthRatio = (state.prevDepth[prevIdx] > 1e-6f)
                ? std::abs(depth[idx] - state.prevDepth[prevIdx]) / state.prevDepth[prevIdx]
                : 1.0f;
            float normalDot = svgfDot3(normalX[idx], normalY[idx], normalZ[idx],
                                       state.prevNX[prevIdx], state.prevNY[prevIdx], state.prevNZ[prevIdx]);

            if (depthRatio < 0.1f && normalDot > 0.9f) {
              valid = true;
              int hist = std::min(255, state.historyLen[prevIdx] + 1);
              float alpha = std::max(1.0f / hist, 0.05f);

              filtR[idx] = state.prevR[prevIdx] * (1 - alpha) + curR[idx] * alpha;
              filtG[idx] = state.prevG[prevIdx] * (1 - alpha) + curG[idx] * alpha;
              filtB[idx] = state.prevB[prevIdx] * (1 - alpha) + curB[idx] * alpha;

              float lum = luminance(curR[idx], curG[idx], curB[idx]);
              float m1 = state.moment1[prevIdx] * (1 - alpha) + lum * alpha;
              float m2 = state.moment2[prevIdx] * (1 - alpha) + lum * lum * alpha;
              state.moment1[idx] = m1;
              state.moment2[idx] = m2;
              variance[idx] = std::max(0.0f, m2 - m1 * m1);
              histLen[idx] = hist;
            }
          }
        }

        if (!valid) {
          // Disoccluded: use current frame only
          filtR[idx] = curR[idx];
          filtG[idx] = curG[idx];
          filtB[idx] = curB[idx];
          float lum = luminance(curR[idx], curG[idx], curB[idx]);
          state.moment1[idx] = lum;
          state.moment2[idx] = lum * lum;
          histLen[idx] = 1;

          // Spatial variance estimate for short-history pixels (3x3)
          float sumLum = 0, sumLum2 = 0;
          int count = 0;
          for (int ky = -1; ky <= 1; ky++) {
            int ny = std::max(0, std::min(h-1, y+ky));
            for (int kx = -1; kx <= 1; kx++) {
              int nx = std::max(0, std::min(w-1, x+kx));
              int ni = nx + ny * w;
              float l = luminance(curR[ni], curG[ni], curB[ni]);
              sumLum += l; sumLum2 += l*l; count++;
            }
          }
          variance[idx] = std::max(0.0f, sumLum2/count - (sumLum/count)*(sumLum/count));
        }
      }
    }
  } else {
    // First frame: no temporal data
    for (int i = 0; i < n; i++) {
      filtR[i] = curR[i]; filtG[i] = curG[i]; filtB[i] = curB[i];
      float lum = luminance(curR[i], curG[i], curB[i]);
      state.moment1[i] = lum;
      state.moment2[i] = lum * lum;
    }
  }

  // ---- Stage 2: Guided À-Trous spatial filter (5 passes) ----
  static const float kernel[5] = {1.0f/16, 4.0f/16, 6.0f/16, 4.0f/16, 1.0f/16};

  float sigmaDepth = 1.0f;
  float sigmaNormal = 128.0f;
  float sigmaLum = 4.0f * (0.3f + strength * 0.7f);

  std::vector<float> tmpR(n), tmpG(n), tmpB(n);
  std::vector<float> tmpVar(n);

  for (int pass = 0; pass < 5; pass++) {
    int step = 1 << pass;

    #pragma omp parallel for schedule(static)
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        int ci = x + y * w;
        float cD = depth[ci];
        float cNx = normalX[ci], cNy = normalY[ci], cNz = normalZ[ci];
        float cLum = luminance(filtR[ci], filtG[ci], filtB[ci]);
        float cVar = variance[ci];
        float lumSigma = sigmaLum * std::sqrt(std::max(1e-6f, cVar)) + 1e-6f;

        // Reduce spatial filtering for reflective surfaces (quadratic falloff)
        float refl = reflectivity ? reflectivity[ci] : 0.0f;
        float reflScale = std::max(0.01f, (1.0f - refl) * (1.0f - refl));

        float sumR = 0, sumG = 0, sumB = 0, sumW = 0, sumVar = 0;

        for (int ky = -2; ky <= 2; ky++) {
          int ny = std::max(0, std::min(h-1, y + ky*step));
          for (int kx = -2; kx <= 2; kx++) {
            int nx = std::max(0, std::min(w-1, x + kx*step));
            int ni = nx + ny * w;

            // Spatial weight (B3-spline)
            float ws = kernel[ky+2] * kernel[kx+2];

            // Depth weight
            float dd = std::abs(depth[ni] - cD);
            float wd = std::exp(-dd / (sigmaDepth * std::abs(cD) + 1e-6f));

            // Normal weight
            float ndot = svgfDot3(normalX[ni], normalY[ni], normalZ[ni],
                                  cNx, cNy, cNz);
            float wn = std::pow(std::max(0.0f, ndot), sigmaNormal);

            // Luminance weight (variance-guided)
            float dl = std::abs(luminance(filtR[ni], filtG[ni], filtB[ni]) - cLum);
            float wl = std::exp(-dl / lumSigma);

            float wt = ws * wd * wn * wl;
            // Scale down neighbor contribution for reflective pixels
            if (kx != 0 || ky != 0) wt *= reflScale;
            sumR += wt * filtR[ni];
            sumG += wt * filtG[ni];
            sumB += wt * filtB[ni];
            sumVar += wt * variance[ni];
            sumW += wt;
          }
        }

        if (sumW > 1e-10f) {
          tmpR[ci] = sumR / sumW;
          tmpG[ci] = sumG / sumW;
          tmpB[ci] = sumB / sumW;
          tmpVar[ci] = sumVar / sumW;
        } else {
          tmpR[ci] = filtR[ci];
          tmpG[ci] = filtG[ci];
          tmpB[ci] = filtB[ci];
          tmpVar[ci] = variance[ci];
        }
      }
    }

    std::swap(filtR, tmpR);
    std::swap(filtG, tmpG);
    std::swap(filtB, tmpB);
    std::swap(variance, tmpVar);
  }

  // ---- Store state for next frame ----
  state.prevR = filtR;
  state.prevG = filtG;
  state.prevB = filtB;
  state.prevDepth.assign(depth, depth + n);
  state.prevNX.assign(normalX, normalX + n);
  state.prevNY.assign(normalY, normalY + n);
  state.prevNZ.assign(normalZ, normalZ + n);
  state.historyLen = histLen;
  state.prevViewProj = camera.viewProj;
  state.prevCamPos = camera.cameraPos;
  state.hasPrevFrame = true;

  // Convert back to Img8u
  dst = core::Img8u(utils::Size(w, h), core::formatRGB);
  icl8u *dR = dst.getData(0);
  icl8u *dG = dst.getData(1);
  icl8u *dB = dst.getData(2);
  for (int i = 0; i < n; i++) {
    dR[i] = (icl8u)std::min(255.0f, std::max(0.0f, filtR[i] * 255.0f));
    dG[i] = (icl8u)std::min(255.0f, std::max(0.0f, filtG[i] * 255.0f));
    dB[i] = (icl8u)std::min(255.0f, std::max(0.0f, filtB[i] * 255.0f));
  }
}

} // namespace icl::rt
