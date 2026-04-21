// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/Homography2D.h>
#include <icl/math/DynMatrix.h>

#include <icl/math/FixedVector.h>

#include <cmath>
#include <vector>

using namespace icl::utils;

namespace icl::math {

  // Hartley normalization (Hartley & Zisserman, "Multiple View Geometry",
  // Algorithm 4.2): translate the centroid to the origin and scale so the
  // mean distance from the origin is sqrt(2). Returns the 3x3 similarity
  // T such that T * p gives the normalized point, and writes the
  // normalized points into `out`. Without this, the DLT matrix for points
  // in pixel coordinates is poorly conditioned (entries span many orders
  // of magnitude), and the solve produces errors of tens of pixels.
  template<class T>
  static FixedMatrix<T,3,3> hartley_normalize(const Point32f *p, int n,
                                              std::vector<Point32f> &out){
    T cx = 0, cy = 0;
    for(int i = 0; i < n; ++i){ cx += p[i].x; cy += p[i].y; }
    cx /= n; cy /= n;
    T s = 0;
    for(int i = 0; i < n; ++i){
      const T dx = p[i].x - cx, dy = p[i].y - cy;
      s += std::sqrt(dx*dx + dy*dy);
    }
    s /= n;
    if(s < T(1e-9)) s = T(1);  // guard against coincident points
    const T k = std::sqrt(T(2)) / s;

    FixedMatrix<T,3,3> Tm;
    Tm(0, 0) = k;  Tm(0, 1) = 0;  Tm(0, 2) = -k*cx;
    Tm(1, 0) = 0;  Tm(1, 1) = k;  Tm(1, 2) = -k*cy;
    Tm(2, 0) = 0;  Tm(2, 1) = 0;  Tm(2, 2) = 1;

    out.resize(n);
    for(int i = 0; i < n; ++i){
      out[i] = Point32f((p[i].x - cx) * k, (p[i].y - cy) * k);
    }
    return Tm;
  }

  // Inverse of the similarity produced by hartley_normalize:
  //   T = [[k, 0, -k*cx], [0, k, -k*cy], [0, 0, 1]]
  //   T^-1 = [[1/k, 0, cx], [0, 1/k, cy], [0, 0, 1]]
  template<class T>
  static FixedMatrix<T,3,3> inv_similarity(const FixedMatrix<T,3,3> &Tm){
    const T k = Tm(0, 0);
    const T cx = -Tm(0, 2) / k;
    const T cy = -Tm(1, 2) / k;
    FixedMatrix<T,3,3> Ti;
    Ti(0, 0) = 1/k; Ti(0, 1) = 0;   Ti(0, 2) = cx;
    Ti(1, 0) = 0;   Ti(1, 1) = 1/k; Ti(1, 2) = cy;
    Ti(2, 0) = 0;   Ti(2, 1) = 0;   Ti(2, 2) = 1;
    return Ti;
  }

  template<class T>
  GenericHomography2D<T>::GenericHomography2D(const Point32f *x, const Point32f *y, int n){
    // Convention: the constructed homography maps y → x, i.e.
    // HOM.apply(y[i]) ≈ x[i]. The DLT derivation below assumes the
    // opposite direction (H*a = b), so we swap the pointers once.
    std::swap(x, y);

    // Hartley-normalize both point sets so the DLT matrix is
    // well-conditioned; fit in normalized space and un-normalize at the
    // end (H = Ty^-1 * Hn * Tx).
    std::vector<Point32f> xn, yn;
    const FixedMatrix<T,3,3> Tx = hartley_normalize<T>(x, n, xn);
    const FixedMatrix<T,3,3> Ty = hartley_normalize<T>(y, n, yn);

    DynMatrix<T> M(8, 2*n), r(1, 2*n);
    for(int i = 0; i < n; ++i){
      const T xx = xn[i].x, xy = xn[i].y, yx = yn[i].x, yy = yn[i].y;
      T *m = &M(2*i, 0);
      m[0] = xx;  m[1] = xy;  m[2] = 1;
      m[3] = 0;   m[4] = 0;   m[5] = 0;
      m[6] = -xx*yx;  m[7] = -xy*yx;
      m += 8;
      m[0] = 0;   m[1] = 0;   m[2] = 0;
      m[3] = xx;  m[4] = xy;  m[5] = 1;
      m[6] = -xx*yy;  m[7] = -xy*yy;

      r[2*i    ] = yx;
      r[2*i + 1] = yy;
    }

    const DynMatrix<T> h = M.solve(r);
    FixedMatrix<T,3,3> Hn;
    std::copy(h.begin(), h.end(), Hn.begin());
    Hn[8] = 1;

    // Un-normalize. Hn maps xn → yn, where xn = Tx*x and yn = Ty*y.
    // So Ty*y = Hn * Tx*x  →  y = Ty^-1 * Hn * Tx * x, hence in the
    // original coordinate system H = Ty^-1 * Hn * Tx maps x → y.
    const FixedMatrix<T,3,3> Ty_inv = inv_similarity<T>(Ty);
    FixedMatrix<T,3,3> H = Ty_inv * Hn * Tx;

    // Normalize so H(2,2) = 1, matching the previous convention.
    const T k = H(2, 2);
    if(std::abs(k) > T(1e-12)){
      for(int i = 0; i < 9; ++i) H[i] /= k;
    }
    std::copy(H.begin(), H.end(), Super::begin());
  }

  template struct ICLMath_API GenericHomography2D<float>;
  template struct ICLMath_API GenericHomography2D<double>;

  } // namespace icl::math
