// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv/CheckerboardSaddleDetector.h>
#include <cmath>
#include <vector>

#ifdef ICL_HAVE_OPENMP
#include <omp.h>
#endif

using namespace icl::utils;
using namespace icl::core;

namespace icl::cv {

  CheckerboardSaddleDetector::CheckerboardSaddleDetector() : m_p(Params()) {}
  CheckerboardSaddleDetector::CheckerboardSaddleDetector(const Params &p) : m_p(p) {}

  namespace {
    // luminance buffer (float) for sub-pixel bilinear sampling. We keep this
    // explicit (raw pointers + luminance weights) rather than core::cc: on mac
    // cc has no SIMD path (no IPP/Accelerate, SSE2 N/A on arm) so it falls back
    // to a slower per-pixel ImgIterator — and its RGB→Gray uses (R+G+B)/3, not
    // the 0.299/0.587/0.114 luminance weights, which would change the response.
    std::vector<float> toGray(const Img8u &img, int &W, int &H, bool mt) {
      W = img.getWidth(); H = img.getHeight();
      std::vector<float> g((size_t)W*H);
      const int N = (int)g.size();
      const int c = img.getChannels();
      if (c >= 3) {
        const icl8u *r = img.begin(0), *gr = img.begin(1), *b = img.begin(2);
#ifdef ICL_HAVE_OPENMP
#       pragma omp parallel for schedule(static) if(mt)
#endif
        for (int i = 0; i < N; ++i)
          g[i] = 0.299f*r[i] + 0.587f*gr[i] + 0.114f*b[i];
      } else {
        const icl8u *d = img.begin(0);
#ifdef ICL_HAVE_OPENMP
#       pragma omp parallel for schedule(static) if(mt)
#endif
        for (int i = 0; i < N; ++i) g[i] = d[i];
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
      // Precomputed bilinear sampling for the dense (integer-grid) response: at an
      // integer pixel (x,y) the sample (x+dx[k], y+dy[k]) has a fractional part
      // that depends only on k, so floor()/weights are constant across pixels.
      std::vector<int>   ox, oy;               // integer offsets floor(dx),floor(dy)
      std::vector<float> bx0, bx1, by0, by1;   // bilinear weights ax,1-ax,ay,1-ay
      explicit Ring(int n, float r) : dx(n), dy(n), c1(n), s1(n), c2(n), s2(n),
                                      ox(n), oy(n), bx0(n), bx1(n), by0(n), by1(n) {
        for (int k = 0; k < n; ++k) {
          const float th = 2.f*float(M_PI)*k/n;
          dx[k] = r*std::cos(th); dy[k] = r*std::sin(th);
          c1[k] = std::cos(th);      s1[k] = std::sin(th);
          c2[k] = std::cos(2*th);    s2[k] = std::sin(2*th);
          const int fx = (int)std::floor(dx[k]), fy = (int)std::floor(dy[k]);
          ox[k] = fx; oy[k] = fy;
          const float ax = dx[k]-fx, ay = dy[k]-fy;
          bx0[k] = ax; bx1[k] = 1.f-ax; by0[k] = ay; by1[k] = 1.f-ay;
        }
      }
    };

    // ring harmonics at (x,y); returns normalised response + sets orientation.
    // General (sub-pixel-capable) path: re-derives the bilinear footprint per
    // sample. Used only for the sub-pixel orientation refine at non-integer (x,y).
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

    // Fast dense path at an INTEGER pixel (x,y): precomputed offsets + weights,
    // and an early flat-area gate (A2 first → skip A1's sqrt + the divide on the
    // flat majority). Bit-identical to responseAt() at integer coordinates.
    inline float responseAtInt(const float *g, int W, const Ring &R, int n,
                               int x, int y, float edgePenalty, float minAmp) {
      float a1c=0, a1s=0, a2c=0, a2s=0;
      const float *base = g + (size_t)y*W + x;
      for (int k = 0; k < n; ++k) {
        const float *p = base + (size_t)R.oy[k]*W + R.ox[k];
        const float v = (p[0]*R.bx1[k] + p[1]*R.bx0[k])*R.by1[k]
                      + (p[W]*R.bx1[k] + p[W+1]*R.bx0[k])*R.by0[k];
        a1c += v*R.c1[k]; a1s += v*R.s1[k];
        a2c += v*R.c2[k]; a2s += v*R.s2[k];
      }
      const float A2 = 2.f*std::sqrt(a2c*a2c + a2s*a2s)/n;
      if (A2 < minAmp) return 0.f;
      const float A1 = 2.f*std::sqrt(a1c*a1c + a1s*a1s)/n;
      const float resp = (A2 - edgePenalty*A1) / (A1 + A2 + 1e-3f);
      return resp > 0.f ? resp : 0.f;
    }

    // Fill resp[] over the valid interior [m, W-m) x [m, H-m): precomputed
    // bilinear weights + early flat-area gate, optionally OpenMP-parallel (the
    // writes are independent, so results are identical either way).
    void fillResponse(std::vector<float> &resp, const float *g, int W, int H,
                      const Ring &R, int n, int m, const CheckerboardSaddleDetector::Params &p) {
#ifdef ICL_HAVE_OPENMP
#     pragma omp parallel for schedule(static) if(p.multithreaded)
#endif
      for (int y = m; y < H-m; ++y)
        for (int x = m; x < W-m; ++x)
          resp[(size_t)y*W + x] = responseAtInt(g, W, R, n, x, y, p.edgePenalty, p.minAmplitude);
    }
  }

  Img32f CheckerboardSaddleDetector::responseImage(const Img8u &image) const {
    int W, H; const std::vector<float> g = toGray(image, W, H, m_p.multithreaded);
    Img32f out(Size(W, H), 1);
    float *o = out.begin(0);
    std::fill(o, o + (size_t)W*H, 0.f);
    const int n = m_p.nSamples;
    const Ring R(n, m_p.radius);
    const int m = (int)std::ceil(m_p.radius) + 1;
    if (W <= 2*m || H <= 2*m) return out;
    std::vector<float> resp((size_t)W*H, 0.f);
    fillResponse(resp, g.data(), W, H, R, n, m, m_p);
    std::copy(resp.begin(), resp.end(), o);
    return out;
  }

  std::vector<CornerSeed> CheckerboardSaddleDetector::detect(const Img8u &image) const {
    int W, H; const std::vector<float> g = toGray(image, W, H, m_p.multithreaded);
    std::vector<CornerSeed> seeds;
    const int n = m_p.nSamples;
    const Ring R(n, m_p.radius);
    const int m = (int)std::ceil(m_p.radius) + 1;
    if (W <= 2*m || H <= 2*m) return seeds;

    // response image over the valid interior
    std::vector<float> resp((size_t)W*H, 0.f);
    fillResponse(resp, g.data(), W, H, R, n, m, m_p);

    // non-maximum suppression + sub-pixel (quadratic) refinement. Scan a row
    // band [y0,y1) and append its seeds (in row-major order) to `out`. The full
    // W*H scan dominates this stage, so it is parallelized over contiguous row
    // bands below; per-band buckets concatenated in order keep the output
    // identical to the serial scan.
    const int nms = m_p.nmsRadius;
    const float *rp = resp.data();
    auto scanBand = [&](int y0, int y1, std::vector<CornerSeed> &out) {
      for (int y = y0; y < y1; ++y) {
        for (int x = m; x < W-m; ++x) {
          const float v = rp[(size_t)y*W + x];
          if (v < m_p.minScore) continue;
          bool isMax = true;
          for (int dy = -nms; dy <= nms && isMax; ++dy)
            for (int dx = -nms; dx <= nms; ++dx)
              if ((dx||dy) && rp[(size_t)(y+dy)*W + (x+dx)] > v) { isMax = false; break; }
          if (!isMax) continue;

          // parabolic sub-pixel peak from the 3x3 response neighbourhood
          const float l = rp[(size_t)y*W + x-1], r = rp[(size_t)y*W + x+1];
          const float u = rp[(size_t)(y-1)*W + x], d = rp[(size_t)(y+1)*W + x];
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

          out.push_back(CornerSeed{Point32f(x+sx, y+sy), v, ori});
        }
      }
    };

#ifdef ICL_HAVE_OPENMP
    if (m_p.multithreaded) {
      const int nt = std::max(1, omp_get_max_threads());
      std::vector<std::vector<CornerSeed>> buckets(nt);
      const int y0 = m, span = H-m - y0;
#     pragma omp parallel num_threads(nt)
      {
        const int t = omp_get_thread_num();
        // contiguous, ordered row bands → concatenating buckets[0..nt) below
        // reproduces the serial row-major seed order exactly.
        scanBand(y0 + (long)span*t/nt, y0 + (long)span*(t+1)/nt, buckets[t]);
      }
      for (auto &b : buckets) seeds.insert(seeds.end(), b.begin(), b.end());
    } else
#endif
      scanBand(m, H-m, seeds);

    return seeds;
  }

} // namespace icl::cv
