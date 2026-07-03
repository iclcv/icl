// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/transform/Homography2D.h>
#include <icl/math/la/DynMatrix.h>
#include <icl/math/fit/FitUtils.h>

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

    // Homogeneous DLT: h is the null space of the 2n x 9 constraint matrix A
    // (rows from b_i x (H a_i) = 0). This is the general form — unlike fixing
    // h9=1 it stays correct when the true H(2,2) -> 0. We solve it via the 9x9
    // normal matrix S = A^T A (accumulated per point, so A is never materialized)
    // and take the eigenvector of its SMALLEST eigenvalue — same solution as the
    // SVD's smallest right-singular vector, but O(n) + a 9x9 eigensolve instead of
    // an expensive 2n x 2n SVD. (Hartley normalization keeps S well-conditioned
    // despite the normal-equations squaring.)
    T AtA[9][9] = {{0}};
    for(int i = 0; i < n; ++i){
      const T ax = an[i].x, ay = an[i].y, bx = bn[i].x, by = bn[i].y;
      const T r0[9] = { 0, 0, 0, -ax, -ay, -1, by*ax, by*ay, by };
      const T r1[9] = { ax, ay, 1, 0, 0, 0, -bx*ax, -bx*ay, -bx };
      for(int p=0;p<9;++p) for(int q=0;q<9;++q) AtA[p][q] += r0[p]*r0[q] + r1[p]*r1[q];
    }
    DynMatrix<T> S = DynMatrix<T>::create(9,9);
    for(int p=0;p<9;++p) for(int q=0;q<9;++q) S(p,q) = AtA[p][q];
    // H is the homogeneous null-space of the correspondence matrix (smallest
    // eigenvector of the 9x9 scatter S = AᵀA) — shared with the algebraic fitters.
    const DynColVector<T> h = homogeneousNullSpace(S);
    FixedMatrix<T,3,3> Hn;
    std::copy(h.begin(), h.end(), Hn.begin());

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
  HomographyFit<T> GenericHomography2D<T>::robust(const Point32f *src, const Point32f *dst, int n,
                                                  T thr, int maxIters){
    HomographyFit<T> best;
    if(n < 4) return best;
    auto resid = [&](const GenericHomography2D<T> &H, int i)->T{
      const Point32f p = H.apply(src[i]);
      return std::sqrt((p.x-dst[i].x)*(p.x-dst[i].x) + (p.y-dst[i].y)*(p.y-dst[i].y));
    };
    // deterministic LCG so results are reproducible run-to-run
    unsigned rng = 0x9e3779b9u;
    auto pick = [&](int mod){ rng = rng*1664525u + 1013904223u; return (int)((rng>>8) % (unsigned)mod); };

    std::vector<int> bestIn;
    int iters = maxIters;
    for(int it=0; it<iters; ++it){
      int idx[4];
      for(int k=0;k<4;++k){ bool dup; do{ idx[k]=pick(n); dup=false;
        for(int j=0;j<k;++j) if(idx[j]==idx[k]) dup=true; }while(dup); }
      Point32f s4[4], d4[4]; for(int k=0;k<4;++k){ s4[k]=src[idx[k]]; d4[k]=dst[idx[k]]; }
      const GenericHomography2D<T> H = dlt_fit<T>(s4, d4, 4);
      bool okH=true; for(int e=0;e<9;++e) if(!std::isfinite((double)H[e])) okH=false;
      if(!okH) continue;
      std::vector<int> in;
      for(int i=0;i<n;++i) if(resid(H,i) < thr) in.push_back(i);
      if(in.size() > bestIn.size()){
        bestIn.swap(in);
        // adaptive termination: iterations for 99% confidence at this inlier ratio
        const double w = (double)bestIn.size()/n, p4 = w*w*w*w;
        if(p4 > 0 && p4 < 1){
          const int need = (int)std::ceil(std::log(1.0-0.99)/std::log(1.0-p4));
          iters = std::min(maxIters, std::max(it+1, need));
        }
      }
    }
    if((int)bestIn.size() < 4) return best;
    std::vector<Point32f> si, di;
    for(int i : bestIn){ si.push_back(src[i]); di.push_back(dst[i]); }
    best.H = dlt_fit<T>(si.data(), di.data(), (int)si.size());   // refit on inliers
    best.inliers = bestIn;
    T s=0; for(int i : bestIn){ const T r=resid(best.H,i); s+=r*r; }
    best.rms = std::sqrt(s/bestIn.size());
    best.ok = true;
    return best;
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
