// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "Upsampling.h"
#include <algorithm>
#include <cmath>

namespace icl::rt {

void upsampleBilinear(const core::Img8u &src, core::Img8u &dst,
                      int dstW, int dstH) {
  int srcW = src.getWidth(), srcH = src.getHeight();
  if (srcW == dstW && srcH == dstH) { dst = src; return; }

  dst = core::Img8u(utils::Size(dstW, dstH), core::formatRGB);
  float scaleX = (float)srcW / dstW;
  float scaleY = (float)srcH / dstH;

  for (int c = 0; c < 3; c++) {
    const icl8u *s = src.getData(c);
    icl8u *d = dst.getData(c);

    #pragma omp parallel for schedule(static)
    for (int dy = 0; dy < dstH; dy++) {
      for (int dx = 0; dx < dstW; dx++) {
        float sx = (dx + 0.5f) * scaleX - 0.5f;
        float sy = (dy + 0.5f) * scaleY - 0.5f;

        int x0 = std::max(0, (int)sx);
        int y0 = std::max(0, (int)sy);
        int x1 = std::min(srcW - 1, x0 + 1);
        int y1 = std::min(srcH - 1, y0 + 1);
        float fx = sx - x0;
        float fy = sy - y0;

        float v = s[x0 + y0*srcW] * (1-fx) * (1-fy)
                + s[x1 + y0*srcW] * fx     * (1-fy)
                + s[x0 + y1*srcW] * (1-fx) * fy
                + s[x1 + y1*srcW] * fx     * fy;
        d[dx + dy*dstW] = (icl8u)std::min(255.0f, std::max(0.0f, v));
      }
    }
  }
}

void upsampleEdgeAware(const core::Img8u &src, core::Img8u &dst,
                       int dstW, int dstH,
                       float sigma_s, float sigma_r) {
  int srcW = src.getWidth(), srcH = src.getHeight();
  if (srcW == dstW && srcH == dstH) { dst = src; return; }

  dst = core::Img8u(utils::Size(dstW, dstH), core::formatRGB);
  const icl8u *sR = src.getData(0);
  const icl8u *sG = src.getData(1);
  const icl8u *sB = src.getData(2);
  icl8u *dR = dst.getData(0);
  icl8u *dG = dst.getData(1);
  icl8u *dB = dst.getData(2);

  float scaleX = (float)srcW / dstW;
  float scaleY = (float)srcH / dstH;
  float invSS = -0.5f / (sigma_s * sigma_s);
  float invSR = -0.5f / (sigma_r * sigma_r);
  int radius = (int)std::ceil(2.0f * sigma_s);

  auto luma = [&](int i) -> float {
    return 0.299f * sR[i] + 0.587f * sG[i] + 0.114f * sB[i];
  };

  #pragma omp parallel for schedule(dynamic, 4)
  for (int dy = 0; dy < dstH; dy++) {
    for (int dx = 0; dx < dstW; dx++) {
      float sx = (dx + 0.5f) * scaleX - 0.5f;
      float sy = (dy + 0.5f) * scaleY - 0.5f;
      int cx = std::max(0, std::min(srcW - 1, (int)(sx + 0.5f)));
      int cy = std::max(0, std::min(srcH - 1, (int)(sy + 0.5f)));
      float cL = luma(cx + cy * srcW);

      float sumR = 0, sumG = 0, sumB = 0, sumW = 0;
      for (int ky = -radius; ky <= radius; ky++) {
        int ny = std::max(0, std::min(srcH - 1, cy + ky));
        for (int kx = -radius; kx <= radius; kx++) {
          int nx = std::max(0, std::min(srcW - 1, cx + kx));
          int ni = nx + ny * srcW;
          float sd = (nx - sx) * (nx - sx) + (ny - sy) * (ny - sy);
          float ld = luma(ni) - cL;
          float w = std::exp(sd * invSS + ld * ld * invSR);
          sumR += w * sR[ni];
          sumG += w * sG[ni];
          sumB += w * sB[ni];
          sumW += w;
        }
      }
      int di = dx + dy * dstW;
      if (sumW > 1e-6f) {
        dR[di] = (icl8u)std::min(255.0f, std::max(0.0f, sumR / sumW));
        dG[di] = (icl8u)std::min(255.0f, std::max(0.0f, sumG / sumW));
        dB[di] = (icl8u)std::min(255.0f, std::max(0.0f, sumB / sumW));
      } else {
        int si = cx + cy * srcW;
        dR[di] = sR[si]; dG[di] = sG[si]; dB[di] = sB[si];
      }
    }
  }
}

void upsampleNearestInt(const std::vector<int32_t> &src, int srcW, int srcH,
                        std::vector<int32_t> &dst, int dstW, int dstH) {
  dst.resize(dstW * dstH);
  float scaleX = (float)srcW / dstW;
  float scaleY = (float)srcH / dstH;

  for (int dy = 0; dy < dstH; dy++) {
    int sy = std::min(srcH - 1, (int)(dy * scaleY));
    for (int dx = 0; dx < dstW; dx++) {
      int sx = std::min(srcW - 1, (int)(dx * scaleX));
      dst[dx + dy * dstW] = src[sx + sy * srcW];
    }
  }
}

} // namespace icl::rt
