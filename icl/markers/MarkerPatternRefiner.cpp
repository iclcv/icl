// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/markers/MarkerPatternRefiner.h>
#include <icl/math/transform/Homography2D.h>
#include <algorithm>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl::markers {

  using math::Homography2D;

  MarkerPatternRefiner::MarkerPatternRefiner(const Img8u &image, const Params &p)
    : m_w(image.getWidth()), m_h(image.getHeight()), m_p(p) {
    m_gray.resize((size_t)m_w * m_h);
    if (image.getChannels() >= 3) {
      const icl8u *r = image.begin(0), *g = image.begin(1), *b = image.begin(2);
      for (size_t i = 0; i < m_gray.size(); ++i) m_gray[i] = 0.299f*r[i] + 0.587f*g[i] + 0.114f*b[i];
    } else {
      const icl8u *d = image.begin(0);
      for (size_t i = 0; i < m_gray.size(); ++i) m_gray[i] = d[i];
    }
  }

  float MarkerPatternRefiner::sample(float x, float y) const {
    if (x < 0) x = 0; if (y < 0) y = 0;
    if (x > m_w-1) x = m_w-1; if (y > m_h-1) y = m_h-1;
    const int x0 = (int)x, y0 = (int)y;
    const int x1 = std::min(x0+1, m_w-1), y1 = std::min(y0+1, m_h-1);
    const float fx = x-x0, fy = y-y0;
    const float *G = m_gray.data();
    return (1-fx)*(1-fy)*G[(size_t)y0*m_w+x0] + fx*(1-fy)*G[(size_t)y0*m_w+x1]
         + (1-fx)*fy*G[(size_t)y1*m_w+x0] + fx*fy*G[(size_t)y1*m_w+x1];
  }

  namespace {
    struct TEdge { Point32f pos; Point32f nrm; };   // template px + black→white unit normal
    inline float vlen(const Point32f &v) { return std::sqrt(v.x*v.x + v.y*v.y); }

    // Interior edge points of the ideal template: strong-gradient pixels, normal =
    // gradient direction (points black→white). The template's OUTER boundary is
    // deliberately excluded (its single polarity would re-introduce the exposure
    // bias) — central differences at interior pixels only, and the gradient is 0
    // across runs of equal cells, so only true colour transitions are kept.
    std::vector<TEdge> templateEdges(const Img8u &t, float gradThresh, int maxEdges) {
      const int W = t.getWidth(), H = t.getHeight();
      const Channel8u c = t[0];
      std::vector<TEdge> e;
      for (int y = 1; y < H-1; ++y)
        for (int x = 1; x < W-1; ++x) {
          const float gx = (float)c(x+1,y) - c(x-1,y);
          const float gy = (float)c(x,y+1) - c(x,y-1);
          const float g = std::sqrt(gx*gx + gy*gy);
          if (g < gradThresh) continue;
          e.push_back({ Point32f(x + 0.5f, y + 0.5f), Point32f(gx/g, gy/g) });
        }
      if ((int)e.size() > maxEdges) {                 // uniform subsample
        std::vector<TEdge> s; s.reserve(maxEdges);
        const double stride = (double)e.size() / maxEdges;
        for (int i = 0; i < maxEdges; ++i) s.push_back(e[(size_t)(i*stride)]);
        e.swap(s);
      }
      // Outer-boundary edges (black border → white background). These are
      // single-polarity (all point outward) so they carry the exposure bias — but
      // the interior edges above are clustered in the marker centre, leaving the
      // corners poorly constrained by extrapolation. The outer edges span the full
      // marker and pin the corner localisation; the interior edges still dominate
      // the exposure-robust scale. Sample a few along each side.
      const int nSide = 8;
      for (int s = 1; s < nSide; ++s) {
        const float f = (float)s/nSide;
        e.push_back({ Point32f(f*W,    0.f), Point32f(0,-1) });  // top    → up
        e.push_back({ Point32f(f*W,    (float)H), Point32f(0, 1) });  // bottom → down
        e.push_back({ Point32f(0.f,    f*H), Point32f(-1,0) });  // left   → left
        e.push_back({ Point32f((float)W, f*H), Point32f( 1,0) });  // right  → right
      }
      return e;
    }
  }

  bool MarkerPatternRefiner::refine(const Img8u &tmpl, const Point32f templateCorners[4],
                                    Point32f corners[4]) const {
    const std::vector<TEdge> edges = templateEdges(tmpl, m_p.templateGrad, m_p.maxEdges);
    if ((int)edges.size() < m_p.minEdges) return false;

    // template "cell" size ~ one template pixel maps to ~ marker / N image px;
    // estimate the image cell size from the current corner span and the template span.
    const float tDiag = vlen(templateCorners[2] - templateCorners[0]);
    if (!(tDiag > 0)) return false;

    Point32f cur[4]; for (int i = 0; i < 4; ++i) cur[i] = corners[i];

    for (int it = 0; it < m_p.iterations; ++it) {
      const Homography2D H(cur, templateCorners, 4);   // apply(template) → image
      const float iDiag = 0.5f*(vlen(cur[2]-cur[0]) + vlen(cur[3]-cur[1]));
      const float imgPerTpl = iDiag / tDiag;           // image px per template px
      const float R = std::max(1.5f, m_p.searchFrac * imgPerTpl);   // ~ a fraction of a cell

      std::vector<Point32f> imgPts, tplPts;
      imgPts.reserve(edges.size()); tplPts.reserve(edges.size());
      for (const TEdge &e : edges) {
        const Point32f q  = H.apply(e.pos);
        const Point32f qn = H.apply(e.pos + e.nrm);    // map the normal direction
        Point32f n(qn.x - q.x, qn.y - q.y);
        const float nl = vlen(n); if (!(nl > 0)) continue;
        n = n * (1.0f/nl);
        // scan along +/- n for the strongest POSITIVE gradient (black→white in +n,
        // matching the template polarity); parabola-refine the peak.
        const float ds = m_p.step; const int M = (int)(2*R/ds) + 1;
        std::vector<float> gv; gv.reserve(M);
        for (float s = -R; s <= R + 1e-3f; s += ds) gv.push_back(sample(q.x + n.x*s, q.y + n.y*s));
        const int ns = (int)gv.size(); if (ns < 5) continue;
        int bi = 1; float best = 0.f;
        for (int i = 1; i < ns-1; ++i) { const float gr = gv[i+1]-gv[i-1]; if (gr > best) { best = gr; bi = i; } }
        if (bi <= 1 || bi >= ns-2 || best < 4.f) continue;   // no clear matching edge
        const float gL = gv[bi]-gv[bi-2], gC = gv[bi+1]-gv[bi-1], gRr = gv[bi+2]-gv[bi];
        const float denom = gL - 2*gC + gRr; float off = 0;
        if (std::fabs(denom) > 1e-6f) off = 0.5f*(gL-gRr)/denom;
        off = std::max(-1.f, std::min(1.f, off));
        const float s = -R + ds*(bi+off);
        imgPts.push_back(Point32f(q.x + n.x*s, q.y + n.y*s));
        tplPts.push_back(e.pos);
      }
      if ((int)imgPts.size() < m_p.minEdges) return false;
      const Homography2D Hf(imgPts.data(), tplPts.data(), (int)imgPts.size());  // template → image
      for (int i = 0; i < 4; ++i) {
        const Point32f c = Hf.apply(templateCorners[i]);
        if (!std::isfinite(c.x) || !std::isfinite(c.y)) return false;
        cur[i] = c;
      }
    }

    // accept only a sane refinement (corners shouldn't jump more than ~1 cell)
    const float iDiag = 0.5f*(vlen(cur[2]-cur[0]) + vlen(cur[3]-cur[1]));
    const float maxJump = 1.5f * (iDiag / tDiag);
    for (int i = 0; i < 4; ++i) if (vlen(cur[i]-corners[i]) > maxJump + 4.f) return false;
    for (int i = 0; i < 4; ++i) corners[i] = cur[i];
    return true;
  }

} // namespace icl::markers
