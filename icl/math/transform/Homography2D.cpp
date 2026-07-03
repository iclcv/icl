// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/transform/Homography2D.h>
#include <icl/math/la/DynMatrix.h>

#include <icl/math/la/FixedVector.h>

#include <cmath>
#include <vector>
#include <algorithm>

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

  // Core normalized DLT. Returns H with apply(a[i]) ~= b[i], i.e. H*a = b.
  // (fit() calls this with (src,dst); the deprecated ctor with (pBs,pAs).)
  template<class T>
  static GenericHomography2D<T> dlt_fit(const Point32f *a, const Point32f *b, int n){
    std::vector<Point32f> an, bn;
    const FixedMatrix<T,3,3> Ta = hartley_normalize<T>(a, n, an);
    const FixedMatrix<T,3,3> Tb = hartley_normalize<T>(b, n, bn);

    DynMatrix<T> M = DynMatrix<T>::create(2*n, 8), r = DynMatrix<T>::create(2*n, 1);
    for(int i = 0; i < n; ++i){
      const T ax = an[i].x, ay = an[i].y, bx = bn[i].x, by = bn[i].y;
      T *m = &M(2*i, 0);
      m[0] = ax;  m[1] = ay;  m[2] = 1;
      m[3] = 0;   m[4] = 0;   m[5] = 0;
      m[6] = -ax*bx;  m[7] = -ay*bx;
      m += 8;
      m[0] = 0;   m[1] = 0;   m[2] = 0;
      m[3] = ax;  m[4] = ay;  m[5] = 1;
      m[6] = -ax*by;  m[7] = -ay*by;
      r[2*i    ] = bx;
      r[2*i + 1] = by;
    }

    const DynMatrix<T> h = M.solve(r);
    FixedMatrix<T,3,3> Hn;
    std::copy(h.begin(), h.end(), Hn.begin());
    Hn[8] = 1;

    // Un-normalize: Hn maps an -> bn (an = Ta*a, bn = Tb*b), so H = Tb^-1 * Hn * Ta
    // maps a -> b in the original coordinate system.
    FixedMatrix<T,3,3> H = inv_similarity<T>(Tb) * Hn * Ta;
    const T k = H(2, 2);
    if(std::abs(k) > T(1e-12)) for(int i = 0; i < 9; ++i) H[i] /= k;

    GenericHomography2D<T> out;
    std::copy(H.begin(), H.end(), out.begin());
    return out;
  }

  template<class T>
  GenericHomography2D<T> GenericHomography2D<T>::fit(const Point32f *src, const Point32f *dst, int n){
    return dlt_fit<T>(src, dst, n);                 // apply(src) = dst
  }

  template<class T>
  GenericHomography2D<T> GenericHomography2D<T>::refined(const Point32f *src, const Point32f *dst, int n){
    // Seed with the DLT estimate, then Levenberg-Marquardt on GEOMETRIC error
    // (the DLT only minimizes an algebraic proxy). We optimize in the Hartley-
    // normalized frame for conditioning and un-normalize the result.
    GenericHomography2D<T> H0 = dlt_fit<T>(src, dst, n);
    std::vector<Point32f> sn, dn;
    const FixedMatrix<T,3,3> Ts = hartley_normalize<T>(src, n, sn);
    const FixedMatrix<T,3,3> Td = hartley_normalize<T>(dst, n, dn);
    // seed in the normalized frame: Hn maps sn -> dn = Td * H0 * Ts^-1
    FixedMatrix<T,3,3> Hn = Td * (FixedMatrix<T,3,3>)H0 * inv_similarity<T>(Ts);
    if(std::abs(Hn(2,2)) > T(1e-12)) for(int i=0;i<9;++i) Hn[i] /= Hn(2,2);

    // 8 free parameters h0..h7 (Hn(2,2) fixed to 1)
    T h[8]; for(int i=0;i<8;++i) h[i] = Hn[i];
    auto cost = [&](const T *hh){
      T c = 0;
      for(int i=0;i<n;++i){ const T x=sn[i].x, y=sn[i].y, w=hh[6]*x+hh[7]*y+1;
        const T u=(hh[0]*x+hh[1]*y+hh[2])/w, v=(hh[3]*x+hh[4]*y+hh[5])/w;
        const T ex=u-dn[i].x, ey=v-dn[i].y; c += ex*ex+ey*ey; }
      return c;
    };
    T lambda = T(1e-3), prev = cost(h);
    for(int iter=0; iter<40; ++iter){
      // normal equations JtJ (8x8), Jtr (8) for the geometric residual
      T JtJ[8][8]={{0}}, Jtr[8]={0};
      for(int i=0;i<n;++i){
        const T x=sn[i].x, y=sn[i].y, w=h[6]*x+h[7]*y+1;
        const T u=(h[0]*x+h[1]*y+h[2])/w, v=(h[3]*x+h[4]*y+h[5])/w;
        const T ex=u-dn[i].x, ey=v-dn[i].y;
        const T Ju[8]={ x/w, y/w, T(1)/w, 0,0,0, -u*x/w, -u*y/w };
        const T Jv[8]={ 0,0,0, x/w, y/w, T(1)/w, -v*x/w, -v*y/w };
        for(int p=0;p<8;++p){ Jtr[p]+=Ju[p]*ex+Jv[p]*ey;
          for(int q=0;q<8;++q) JtJ[p][q]+=Ju[p]*Ju[q]+Jv[p]*Jv[q]; }
      }
      // LM damped step: (JtJ + lambda*diag(JtJ)) d = -Jtr
      DynMatrix<T> A=DynMatrix<T>::create(8,8), rhs=DynMatrix<T>::create(1,8);
      for(int p=0;p<8;++p){ for(int q=0;q<8;++q) A(p,q)=JtJ[p][q];
        A(p,p)+=lambda*JtJ[p][p]; rhs[p]=-Jtr[p]; }
      DynMatrix<T> d;
      try{ d = A.solve(rhs); }catch(...){ break; }
      T ht[8]; for(int p=0;p<8;++p) ht[p]=h[p]+d[p];
      const T tc = cost(ht);
      if(tc < prev){                                   // accept, decrease damping
        for(int p=0;p<8;++p) h[p]=ht[p];
        lambda = std::max(lambda*T(0.3), T(1e-9));
        if(prev - tc < T(1e-12)*(1+prev)){ prev=tc; break; }   // converged
        prev = tc;
      } else {                                         // reject, increase damping
        lambda = std::min(lambda*T(3), T(1e6));
      }
    }
    FixedMatrix<T,3,3> Hn2; for(int i=0;i<8;++i) Hn2[i]=h[i]; Hn2[8]=1;
    FixedMatrix<T,3,3> H = inv_similarity<T>(Td) * Hn2 * Ts;   // un-normalize
    const T k = H(2,2); if(std::abs(k) > T(1e-12)) for(int i=0;i<9;++i) H[i]/=k;
    GenericHomography2D<T> out; std::copy(H.begin(), H.end(), out.begin());
    return out;
  }

  template<class T>
  GenericHomography2D<T>::GenericHomography2D(const Point32f *pAs, const Point32f *pBs, int n){
    // legacy convention: maps pBs -> pAs (apply(pBs)=pAs)
    const GenericHomography2D<T> H = dlt_fit<T>(pBs, pAs, n);
    std::copy(H.begin(), H.end(), this->begin());
  }

  template struct ICLMath_API GenericHomography2D<float>;
  template struct ICLMath_API GenericHomography2D<double>;

  } // namespace icl::math
