// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/detail/PlotFrame.h>
#include <icl/viz3d/GroupNode.h>
#include <icl/viz3d/MeshNode.h>
#include <icl/viz3d/TextNode.h>
#include <icl/cv3d/GeomDefs.h>
#include <icl/utils/StringUtils.h>
#include <algorithm>
#include <cmath>

namespace icl::viz3d {
  using utils::Range32f;
  using utils::str;
  using cv3d::Vec;
  using cv3d::GeomColor;

  namespace detail {

    static const GeomColor white(255, 255, 255, 255);

    static Range32f round_range(Range32f r) {
      if (r.minVal > r.maxVal) std::swap(r.minVal, r.maxVal);
      float m = std::fabs(r.maxVal - r.minVal), f = 1;
      if (m > 1) { while (m / f > 100) f *= 10; }
      else       { while (m / f < 10)  f *= 0.1f; }
      r.minVal = std::floor(r.minVal / f) * f;
      r.maxVal = std::ceil(r.maxVal / f) * f;
      return r;
    }

    static std::string create_label(float r) {
      return str(std::fabs(r) < 0.0000001f ? 0 : r);
    }

    std::shared_ptr<MeshNode> makePlotBox() {
      auto box = std::make_shared<MeshNode>();
      const float c[8][3] = {{ 1,-1, 1},{ 1, 1, 1},{-1, 1, 1},{-1,-1, 1},
                             { 1,-1,-1},{ 1, 1,-1},{-1, 1,-1},{-1,-1,-1}};
      for (auto &p : c) box->addVertex(Vec(p[0], p[1], p[2], 1), white);
      for (int i = 0; i < 4; ++i) {
        box->addLine(i, (i+1)%4, white);
        box->addLine(4+i, 4+(i+1)%4, white);
        box->addLine(i, i+4, white);
      }
      box->setPrimitiveVisible(PrimVertex, false);
      return box;
    }

    std::shared_ptr<GroupNode> makePlotAxis(const Range32f &range, bool invertLabels,
                                            const std::string &name) {
      auto g = std::make_shared<GroupNode>();
      Range32f in = range;
      // never emit inf/nan ticks if handed a degenerate/unbounded range
      if (!std::isfinite(in.minVal) || !std::isfinite(in.maxVal) || in.minVal >= in.maxVal)
        in = Range32f(-1, 1);
      const Range32f rr = round_range(in);
      const float mn = rr.minVal, mx = rr.maxVal;
      const int N = 10;
      const float step = (mx - mn) / N;
      const float lenBase = 0.1f, d = 0.1f;

      auto ticks = std::make_shared<MeshNode>();
      for (int i = -N/2, l = 0; i <= N/2; ++i, ++l) {
        const float r = float(i) / (N/2);
        const float len = i ? lenBase : 2*lenBase;
        const int base = (int)ticks->getVertices().size();
        ticks->addVertex(Vec(r, 0, 0, 1), white);
        ticks->addVertex(Vec(r, len, 0, 1), white);
        ticks->addVertex(Vec(r, 0, len, 1), white);
        ticks->addLine(base, base+1, white);
        ticks->addLine(base, base+2, white);
        // Label sits ON its tick (local x = r); for an inverted axis reverse the
        // VALUE (mx->mn) rather than the position, so numbers still line up with
        // their ticks but count the other way.
        auto t = TextNode::create(create_label(mn + (invertLabels ? (N - l) : l)*step), 0.08f, white);
        t->translate(r, -d, 0);
        g->addChild(t);
      }
      ticks->setPrimitiveVisible(PrimVertex, false);
      g->addChild(ticks);

      auto nameT = TextNode::create(name, 0.12f, white);
      nameT->translate((invertLabels ? -1 : 1) * (1 + 2*d), 0, 0);
      g->addChild(nameT);
      return g;
    }

    void placePlotAxes(const std::shared_ptr<GroupNode> (&axes)[3],
                       int sx, int sy, int sz) {
      // Each axis i sits on the box edge where the OTHER two coords equal the
      // chosen corner's signs, so all three tick-edges meet at (sx,sy,sz). With
      // the corner FURTHEST from the camera, the ticks frame the data from
      // behind. For axis i we take the cyclic partners (u,v) = (i+1, i+2) mod 3
      // (so p = u×v is right-handed) and build a rotation whose columns are
      // [p | a | b]:
      //   - p (image of local +x) is the axis's own + direction  -> values read
      //     right-handed;
      //   - a,b (images of the tick-bracket arms local +y,+z) are the two
      //     INTERIOR directions (-su·u, -sv·v), so the ticks embrace the two
      //     adjacent faces; their order is chosen to keep det(R)=+1;
      //   - the numeric labels (local -y -> -a) then fall just OUTSIDE the box.
      // viz3d::Node composes POST-multiply, so translate-to-edge first, then the
      // rotation (applied as rotate-then-translate to the local vertices).
      using cv3d::Mat; using cv3d::Vec;
      const int s[3] = { sx, sy, sz };
      auto e = [](int k, float val) { Vec r(0,0,0,0); r[k] = val; return r; };
      for (int i = 0; i < 3; ++i) {
        const int iu = (i+1)%3, iv = (i+2)%3;
        const int su = s[iu], sv = s[iv];
        const Vec p = e(i, 1);
        Vec a, b;
        if (su == sv) { a = e(iu, -su); b = e(iv, -sv); }
        else          { a = e(iv, -sv); b = e(iu, -su); }
        const Mat R(p[0], a[0], b[0], 0,
                    p[1], a[1], b[1], 0,
                    p[2], a[2], b[2], 0,
                    0,    0,    0,    1);
        const Vec t = e(iu, su) + e(iv, sv);
        axes[i]->translate(t[0], t[1], t[2]);
        axes[i]->transform(R);
      }
    }

  } // namespace detail
} // namespace icl::viz3d
