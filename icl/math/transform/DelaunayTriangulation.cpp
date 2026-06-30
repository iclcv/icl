// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/transform/DelaunayTriangulation.h>
#include <algorithm>
#include <array>

namespace icl::math {

  using utils::Point32f;

  std::vector<DelaunayTriangle>
  delaunayTriangulation(const std::vector<Point32f> &points) {
    const int n = (int)points.size();
    if (n < 3) return {};

    // Working coordinates in double precision, with 3 extra slots [n,n+1,n+2]
    // for a super-triangle that encloses every input point.
    std::vector<double> X(n + 3), Y(n + 3);
    double minx = points[0].x, maxx = minx, miny = points[0].y, maxy = miny;
    for (int i = 0; i < n; ++i) {
      X[i] = points[i].x; Y[i] = points[i].y;
      minx = std::min(minx, X[i]); maxx = std::max(maxx, X[i]);
      miny = std::min(miny, Y[i]); maxy = std::max(maxy, Y[i]);
    }
    const double dmax = std::max(std::max(maxx - minx, maxy - miny), 1.0);
    const double midx = 0.5 * (minx + maxx), midy = 0.5 * (miny + maxy);
    X[n]   = midx - 20 * dmax; Y[n]   = midy - dmax;
    X[n+1] = midx;             Y[n+1] = midy + 20 * dmax;
    X[n+2] = midx + 20 * dmax; Y[n+2] = midy - dmax;

    auto orient = [&](int a, int b, int c) {
      return (X[b]-X[a]) * (Y[c]-Y[a]) - (Y[b]-Y[a]) * (X[c]-X[a]);
    };
    // build a triangle with counter-clockwise winding
    auto ccw = [&](int a, int b, int c) -> DelaunayTriangle {
      if (orient(a, b, c) < 0) std::swap(b, c);
      return {a, b, c};
    };
    // is point d strictly inside the circumcircle of CCW triangle t?
    auto inCircle = [&](const DelaunayTriangle &t, int d) -> bool {
      const double ax = X[t.a]-X[d], ay = Y[t.a]-Y[d];
      const double bx = X[t.b]-X[d], by = Y[t.b]-Y[d];
      const double cx = X[t.c]-X[d], cy = Y[t.c]-Y[d];
      const double a2 = ax*ax + ay*ay, b2 = bx*bx + by*by, c2 = cx*cx + cy*cy;
      const double det = ax * (by*c2 - b2*cy)
                       - ay * (bx*c2 - b2*cx)
                       + a2 * (bx*cy - by*cx);
      return det > 0.0;
    };

    std::vector<DelaunayTriangle> tris;
    tris.push_back(ccw(n, n + 1, n + 2));

    std::vector<std::array<int, 2>> edges;   // edges of the removed (bad) triangles
    std::vector<DelaunayTriangle> good;
    for (int i = 0; i < n; ++i) {
      edges.clear();
      good.clear();
      for (const DelaunayTriangle &t : tris) {
        if (inCircle(t, i)) {
          edges.push_back({std::min(t.a, t.b), std::max(t.a, t.b)});
          edges.push_back({std::min(t.b, t.c), std::max(t.b, t.c)});
          edges.push_back({std::min(t.a, t.c), std::max(t.a, t.c)});
        } else {
          good.push_back(t);
        }
      }
      // re-triangulate the polygonal hole: each boundary edge (one that bordered
      // exactly one removed triangle) plus the new point forms a triangle.
      for (size_t e = 0; e < edges.size(); ++e) {
        int cnt = 0;
        for (size_t f = 0; f < edges.size(); ++f)
          if (edges[e] == edges[f]) ++cnt;
        if (cnt == 1) good.push_back(ccw(edges[e][0], edges[e][1], i));
      }
      tris.swap(good);
    }

    // drop every triangle still touching a super-triangle vertex
    std::vector<DelaunayTriangle> out;
    out.reserve(tris.size());
    for (const DelaunayTriangle &t : tris)
      if (t.a < n && t.b < n && t.c < n) out.push_back(t);
    return out;
  }

  std::vector<std::pair<int, int>>
  delaunayEdges(const std::vector<DelaunayTriangle> &triangles) {
    std::vector<std::pair<int, int>> e;
    e.reserve(triangles.size() * 3);
    for (const DelaunayTriangle &t : triangles) {
      e.emplace_back(std::min(t.a, t.b), std::max(t.a, t.b));
      e.emplace_back(std::min(t.b, t.c), std::max(t.b, t.c));
      e.emplace_back(std::min(t.a, t.c), std::max(t.a, t.c));
    }
    std::sort(e.begin(), e.end());
    e.erase(std::unique(e.begin(), e.end()), e.end());
    return e;
  }

} // namespace icl::math
