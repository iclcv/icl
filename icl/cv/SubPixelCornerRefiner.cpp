// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv/SubPixelCornerRefiner.h>
#include <algorithm>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl::cv {

  SubPixelCornerRefiner::SubPixelCornerRefiner(const Img8u &image, const Params &p)
    : m_w(image.getWidth()), m_h(image.getHeight()), m_p(p) {
    m_gray.resize((size_t)m_w * m_h);
    if (image.getChannels() >= 3) {
      const icl8u *r = image.begin(0), *g = image.begin(1), *b = image.begin(2);
      for (size_t i = 0; i < m_gray.size(); ++i)
        m_gray[i] = 0.299f*r[i] + 0.587f*g[i] + 0.114f*b[i];
    } else {
      const icl8u *d = image.begin(0);
      for (size_t i = 0; i < m_gray.size(); ++i) m_gray[i] = d[i];
    }
  }

  float SubPixelCornerRefiner::sample(float x, float y) const {
    if (x < 0) x = 0; if (y < 0) y = 0;
    if (x > m_w-1) x = m_w-1; if (y > m_h-1) y = m_h-1;
    const int x0 = (int)x, y0 = (int)y;
    const int x1 = std::min(x0+1, m_w-1), y1 = std::min(y0+1, m_h-1);
    const float fx = x-x0, fy = y-y0;
    const float *G = m_gray.data();
    return (1-fx)*(1-fy)*G[(size_t)y0*m_w+x0] + fx*(1-fy)*G[(size_t)y0*m_w+x1]
         + (1-fx)*fy*G[(size_t)y1*m_w+x0] + fx*fy*G[(size_t)y1*m_w+x1];
  }

  static inline float vlen(const Point32f &v) { return std::sqrt(v.x*v.x + v.y*v.y); }

  void SubPixelCornerRefiner::refineQuad(Point32f C[4]) const {
    // shortest edge → cap the perpendicular search so it can't reach inner pattern
    float minEdge = 1e30f;
    for (int e = 0; e < 4; ++e) minEdge = std::min(minEdge, vlen(C[(e+1)%4] - C[e]));
    const float R  = std::min(m_p.searchRadius, 0.2f * minEdge);
    const float ds = m_p.step;
    if (!(R >= 1.f)) return;   // quad too small to refine reliably

    // fit a straight line (point lp + unit dir lu) to each edge's border samples
    Point32f lp[4], lu[4];
    bool ok[4] = {false,false,false,false};
    for (int e = 0; e < 4; ++e) {
      const Point32f A = C[e], B = C[(e+1)%4];
      Point32f al = B - A; const float L = vlen(al);
      if (L < 4.f) continue;
      al = al * (1.0/L);
      const Point32f nrm(-al.y, al.x);
      std::vector<Point32f> pts;
      const int M = m_p.samplesPerEdge, skip = std::max(0, m_p.skipEnds);
      for (int m = skip; m <= M-skip; ++m) {
        const float t = (float)m/M;
        const Point32f base = A + al * (t*L);
        std::vector<float> gv;
        for (float s = -R; s <= R + 1e-3f; s += ds)
          gv.push_back(sample(base.x + nrm.x*s, base.y + nrm.y*s));
        const int n = (int)gv.size();
        if (n < 5) continue;
        // |central-difference gradient|, then its parabola-interpolated peak
        int bi = 1; float best = 0;
        std::vector<float> gr(n, 0.f);
        for (int i = 1; i < n-1; ++i) { gr[i] = std::fabs(gv[i+1]-gv[i-1]); }
        for (int i = 2; i < n-1; ++i) if (gr[i] > best) { best = gr[i]; bi = i; }
        if (bi <= 1 || bi >= n-2) continue;
        const float gL = gr[bi-1], gC = gr[bi], gRr = gr[bi+1];
        const float denom = gL - 2*gC + gRr;
        float off = 0; if (std::fabs(denom) > 1e-6f) off = 0.5f*(gL-gRr)/denom;
        off = std::max(-1.f, std::min(1.f, off));
        const float s = -R + ds*(bi+off);
        pts.push_back(Point32f(base.x + nrm.x*s, base.y + nrm.y*s));
      }
      if (pts.size() < 3) continue;
      // total-least-squares line: centroid + principal direction
      double mx=0, my=0; for (const auto &q : pts) { mx += q.x; my += q.y; }
      mx /= pts.size(); my /= pts.size();
      double sxx=0, syy=0, sxy=0;
      for (const auto &q : pts) { const double dx=q.x-mx, dy=q.y-my; sxx+=dx*dx; syy+=dy*dy; sxy+=dx*dy; }
      const double th = 0.5*std::atan2(2*sxy, sxx-syy);
      lp[e] = Point32f((float)mx, (float)my);
      lu[e] = Point32f((float)std::cos(th), (float)std::sin(th));
      ok[e] = true;
    }

    // corner e = intersection of edge (e-1) and edge e (only if both fitted)
    Point32f out[4];
    for (int e = 0; e < 4; ++e) out[e] = C[e];
    for (int e = 0; e < 4; ++e) {
      const int a = (e+3)%4;
      if (!ok[a] || !ok[e]) continue;
      const Point32f p = lp[a], r = lu[a], q = lp[e], s = lu[e];
      const float den = r.x*s.y - r.y*s.x;
      if (std::fabs(den) < 1e-6f) continue;           // near-parallel: keep coarse
      const float tt = ((q.x-p.x)*s.y - (q.y-p.y)*s.x) / den;
      const Point32f cand(p.x + r.x*tt, p.y + r.y*tt);
      if (vlen(cand - C[e]) < 3.f) out[e] = cand;     // reject implausible jumps
    }
    for (int e = 0; e < 4; ++e) C[e] = out[e];
  }

} // namespace icl::cv
