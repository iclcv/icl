// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLCore/ConvexHull.h>
#include <algorithm>
#include <vector>

using namespace icl::utils;

namespace icl{
  namespace utils{
    static inline bool operator <(const Point &a, const Point &p){
      return a.x < p.x || (a.x == p.x && a.y < p.y);
    }
    static inline int cross(const Point &O, const Point &A, const Point &B){
      return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
    }

    static inline bool operator <(const Point32f &a, const Point32f &p){
      return a.x < p.x || (a.x == p.x && a.y < p.y);
    }
    static inline float cross(const Point32f &O, const Point32f &A, const Point32f &B){
      return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
    }
  }

  namespace core{
    // Implementation was found here: http://www.algorithmist.com/index.php/Monotone_Chain_Convex_Hull.cpp
    // 2D cross product.
    // Return a positive value, if OAB makes a counter-clockwise turn,
    // negative for clockwise turn, and zero if the points are collinear.

    // Returns a list of points on the convex hull in counter-clockwise order.
    // Note: the last point in the returned list is the same as the first one.
    std::vector<Point> convexHull(std::vector<Point> P){
      int n = P.size(), k = 0;
      std::vector<Point> H(2*n);

      // Sort points lexicographically
      sort(P.begin(), P.end());

      // Build lower hull
      for (int i = 0; i < n; i++) {
        while (k >= 2 && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
        H[k++] = P[i];
      }

      // Build upper hull
      for (int i = n-2, t = k+1; i >= 0; i--) {
        while (k >= t && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
        H[k++] = P[i];
      }

      H.resize(k);
      return H;
    }


    std::vector<Point32f> convexHull(std::vector<Point32f> P){
      int n = P.size(), k = 0;
      std::vector<Point32f> H(2*n);

      // Sort points lexicographically
      std::sort(P.begin(), P.end());

      // Build lower hull
      for (int i = 0; i < n; i++) {
        while (k >= 2 && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
        H[k++] = P[i];
      }

      // Build upper hull
      for (int i = n-2, t = k+1; i >= 0; i--) {
        while (k >= t && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
        H[k++] = P[i];
      }

      H.resize(k);
      return H;
    }
  } // namespace geom
}
