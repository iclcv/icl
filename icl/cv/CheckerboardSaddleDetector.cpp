// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv/CheckerboardSaddleDetector.h>
#include <cmath>
#include <vector>

using namespace icl::utils;
using namespace icl::core;

namespace icl::cv {

  CheckerboardSaddleDetector::CheckerboardSaddleDetector() : m_p(Params()) {}
  CheckerboardSaddleDetector::CheckerboardSaddleDetector(const Params &p) : m_p(p) {}

  namespace {
    // luminance buffer (float) for sub-pixel bilinear sampling
    std::vector<float> toGray(const Img8u &img, int &W, int &H) {
      W = img.getWidth(); H = img.getHeight();
      std::vector<float> g((size_t)W*H);
      const int c = img.getChannels();
      if (c >= 3) {
        const icl8u *r = img.begin(0), *gr = img.begin(1), *b = img.begin(2);
        for (size_t i = 0; i < g.size(); ++i)
          g[i] = 0.299f*r[i] + 0.587f*gr[i] + 0.114f*b[i];
      } else {
        const icl8u *d = img.begin(0);
        for (size_t i = 0; i < g.size(); ++i) g[i] = d[i];
      }
      return g;
    }

    inline float bilinear(const float *g, int W, float fx, float fy) {
      const int x0 = (int)std::floor(fx), y0 = (int)std::floor(fy);
      const float ax = fx - x0, ay = fy - y0;
      const float *p = g + (size_t)y0*W + x0;
      return (p[0]*(1-ax) + p[1]*ax)*(1-ay) + (p[W]*(1-ax) + p[W+1]*ax)*ay;
    }

    struct Ring {
      std::vector<float> dx, dy, c1, s1, c2, s2;
      explicit Ring(int n, float r) : dx(n), dy(n), c1(n), s1(n), c2(n), s2(n) {
        for (int k = 0; k < n; ++k) {
          const float th = 2.f*float(M_PI)*k/n;
          dx[k] = r*std::cos(th); dy[k] = r*std::sin(th);
          c1[k] = std::cos(th);      s1[k] = std::sin(th);
          c2[k] = std::cos(2*th);    s2[k] = std::sin(2*th);
        }
      }
    };

    // ring harmonics at (x,y); returns normalised response + sets orientation
    inline float responseAt(const float *g, int W, const Ring &R, int n,
                            float x, float y, float edgePenalty, float minAmp,
                            float *orientation = nullptr) {
      float a1c=0, a1s=0, a2c=0, a2s=0;
      for (int k = 0; k < n; ++k) {
        const float v = bilinear(g, W, x + R.dx[k], y + R.dy[k]);
        a1c += v*R.c1[k]; a1s += v*R.s1[k];
        a2c += v*R.c2[k]; a2s += v*R.s2[k];
      }
      const float A1 = 2.f*std::sqrt(a1c*a1c + a1s*a1s)/n;
      const float A2 = 2.f*std::sqrt(a2c*a2c + a2s*a2s)/n;
      if (orientation) *orientation = 0.5f*std::atan2(a2s, a2c);
      if (A2 < minAmp) return 0.f;
      const float resp = (A2 - edgePenalty*A1) / (A1 + A2 + 1e-3f);
      return resp > 0.f ? resp : 0.f;
    }
  }

  Img32f CheckerboardSaddleDetector::responseImage(const Img8u &image) const {
    int W, H; const std::vector<float> g = toGray(image, W, H);
    Img32f out(Size(W, H), 1);
    float *o = out.begin(0);
    std::fill(o, o + (size_t)W*H, 0.f);
    const int n = m_p.nSamples;
    const Ring R(n, m_p.radius);
    const int m = (int)std::ceil(m_p.radius) + 1;
    if (W <= 2*m || H <= 2*m) return out;
    for (int y = m; y < H-m; ++y)
      for (int x = m; x < W-m; ++x)
        o[(size_t)y*W + x] = responseAt(g.data(), W, R, n, (float)x, (float)y,
                                        m_p.edgePenalty, m_p.minAmplitude);
    return out;
  }

  std::vector<CornerSeed> CheckerboardSaddleDetector::detect(const Img8u &image) const {
    int W, H; const std::vector<float> g = toGray(image, W, H);
    std::vector<CornerSeed> seeds;
    const int n = m_p.nSamples;
    const Ring R(n, m_p.radius);
    const int m = (int)std::ceil(m_p.radius) + 1;
    if (W <= 2*m || H <= 2*m) return seeds;

    // response image over the valid interior
    std::vector<float> resp((size_t)W*H, 0.f);
    for (int y = m; y < H-m; ++y)
      for (int x = m; x < W-m; ++x)
        resp[(size_t)y*W + x] = responseAt(g.data(), W, R, n, (float)x, (float)y,
                                           m_p.edgePenalty, m_p.minAmplitude);

    // non-maximum suppression + sub-pixel (quadratic) refinement
    const int nms = m_p.nmsRadius;
    for (int y = m; y < H-m; ++y) {
      for (int x = m; x < W-m; ++x) {
        const float v = resp[(size_t)y*W + x];
        if (v < m_p.minScore) continue;
        bool isMax = true;
        for (int dy = -nms; dy <= nms && isMax; ++dy)
          for (int dx = -nms; dx <= nms; ++dx)
            if ((dx||dy) && resp[(size_t)(y+dy)*W + (x+dx)] > v) { isMax = false; break; }
        if (!isMax) continue;

        // parabolic sub-pixel peak from the 3x3 response neighbourhood
        const float l = resp[(size_t)y*W + x-1], r = resp[(size_t)y*W + x+1];
        const float u = resp[(size_t)(y-1)*W + x], d = resp[(size_t)(y+1)*W + x];
        float sx = 0.f, sy = 0.f;
        const float denx = (2*v - l - r), deny = (2*v - u - d);
        if (std::fabs(denx) > 1e-6f) sx = 0.5f*(l - r)/denx;
        if (std::fabs(deny) > 1e-6f) sy = 0.5f*(u - d)/deny;
        if (sx >  1.f) sx =  1.f; if (sx < -1.f) sx = -1.f;
        if (sy >  1.f) sy =  1.f; if (sy < -1.f) sy = -1.f;

        float ori = 0.f;
        responseAt(g.data(), W, R, n, x+sx, y+sy, m_p.edgePenalty, m_p.minAmplitude, &ori);
        // fold orientation into [0, pi/2) (board axes are 90°-symmetric)
        const float HALF_PI = float(M_PI)/2;
        while (ori < 0)        ori += HALF_PI;
        while (ori >= HALF_PI) ori -= HALF_PI;

        CornerSeed s; s.pos = Point32f(x+sx, y+sy); s.score = v; s.orientation = ori;
        seeds.push_back(s);
      }
    }
    return seeds;
  }

} // namespace icl::cv
