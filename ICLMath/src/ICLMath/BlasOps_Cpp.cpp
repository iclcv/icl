/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/BlasOps_Cpp.cpp                    **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

// C++ fallback backends for BLAS/LAPACK operations.
// Contains naive GEMM and Golub-Kahan bidiagonalization SVD.

#include <ICLMath/BlasOps.h>
#include <ICLMath/DynMatrix.h>

#include <vector>
#include <algorithm>
#include <cmath>

using namespace icl::utils;

namespace icl {
  namespace math {

    namespace {

      // ================================================================
      // GEMM: naive O(M*N*K) — correct but slow
      // ================================================================

      template<class T>
      void cpp_gemm(bool transA, bool transB,
                    int M, int N, int K, T alpha,
                    const T* A, int lda, const T* B, int ldb,
                    T beta, T* C, int ldc) {
        for(int i = 0; i < M; ++i) {
          for(int j = 0; j < N; ++j) {
            T sum = 0;
            for(int k = 0; k < K; ++k) {
              T a = transA ? A[k * lda + i] : A[i * lda + k];
              T b = transB ? B[j * ldb + k] : B[k * ldb + j];
              sum += a * b;
            }
            C[i * ldc + j] = alpha * sum + beta * C[i * ldc + j];
          }
        }
      }

      // ================================================================
      // SVD: Golub-Kahan bidiagonalization
      // Translated from the original Algol code in "Handbook for
      // Automatic Computation, vol. II, Linear Algebra", Springer-Verlag.
      // (C) 2000, C. Bond. Modified for ICL DynMatrix types.
      //
      // Returns 0 on success, or k if convergence fails at the kth
      // singular value.
      // ================================================================

      static int svd_bidiag(int m, int n, int withu, int withv,
                            icl64f eps, icl64f tol,
                            const DynMatrix<icl64f>& a, icl64f* q,
                            DynMatrix<icl64f>& u, DynMatrix<icl64f>& v) {
        int i,j,k,l(0),l1,iter,retval;
        icl64f c,f,g,h,s,x,y,z;
        std::vector<icl64f> e(n);
        retval = 0;

        for (i=0;i<m;i++)
          for (j=0;j<n;j++)
            u(j,i) = a(j,i);

        // Householder's reduction to bidiagonal form
        g = x = 0.0;
        for (i=0;i<n;i++) {
          e[i] = g;
          s = 0.0;
          l = i+1;
          for (j=i;j<m;j++) s += u(i,j)*u(i,j);
          if (s < tol) { g = 0.0; }
          else {
            f = u(i,i);
            g = (f < 0) ? sqrt(s) : -sqrt(s);
            h = f * g - s;
            u(i,i) = f - g;
            for (j=l;j<n;j++) {
              s = 0.0;
              for (k=i;k<m;k++) s += u(i,k) * u(j,k);
              f = s / h;
              for (k=i;k<m;k++) u(j,k) += f * u(i,k);
            }
          }
          q[i] = g;
          s = 0.0;
          for (j=l;j<n;j++) s += u(j,i) * u(j,i);
          if (s < tol) { g = 0.0; }
          else {
            f = u(i+1,i);
            g = (f < 0) ? sqrt(s) : -sqrt(s);
            h = f * g - s;
            u(i+1,i) = f - g;
            for (j=l;j<n;j++) e[j] = u(j,i)/h;
            for (j=l;j<m;j++) {
              s = 0.0;
              for (k=l;k<n;k++) s += u(k,j) * u(k,i);
              for (k=l;k<n;k++) u(k,j) += s * e[k];
            }
          }
          y = fabs(q[i]) + fabs(e[i]);
          if (y > x) x = y;
        }

        // Accumulation of right-hand transformations
        if (withv) {
          for (i=n-1;i>=0;i--) {
            if (g != 0.0) {
              h = u(i+1,i) * g;
              for (j=l;j<n;j++) v(i,j) = u(j,i)/h;
              for (j=l;j<n;j++) {
                s = 0.0;
                for (k=l;k<n;k++) s += u(k,i) * v(j,k);
                for (k=l;k<n;k++) v(j,k) += s * v(i,k);
              }
            }
            for (j=l;j<n;j++) v(j,i) = v(i,j) = 0.0;
            v(i,i) = 1.0;
            g = e[i];
            l = i;
          }
        }

        // Accumulation of left-hand transformations
        if (withu) {
          for (i=n;i<m;i++) {
            for (j=n;j<m;j++) u(j,i) = 0.0;
            u(i,i) = 1.0;
          }
          for (i=n-1;i>=0;i--) {
            l = i + 1;
            g = q[i];
            for (j=l;j<m;j++) u(j,i) = 0.0;
            if (g != 0.0) {
              h = u(i,i) * g;
              for (j=l;j<m;j++) {
                s = 0.0;
                for (k=l;k<m;k++) s += u(i,k) * u(j,k);
                f = s / h;
                for (k=i;k<m;k++) u(j,k) += f * u(i,k);
              }
              for (j=i;j<m;j++) u(i,j) /= g;
            } else {
              for (j=i;j<m;j++) u(i,j) = 0.0;
            }
            u(i,i) += 1.0;
          }
        }

        // Diagonalization of the bidiagonal form
        eps *= x;
        for (k=n-1;k>=0;k--) {
          iter = 0;
          test_f_splitting:
          for (l=k;l>=0;l--) {
            if (fabs(e[l]) <= eps) goto test_f_convergence;
            if (fabs(q[l-1]) <= eps) goto cancellation;
          }

          cancellation:
          c = 0.0; s = 1.0; l1 = l - 1;
          for (i=l;i<=k;i++) {
            f = s * e[i]; e[i] *= c;
            if (fabs(f) <= eps) goto test_f_convergence;
            g = q[i];
            h = q[i] = sqrt(f*f + g*g);
            c = g / h; s = -f / h;
            if (withu) {
              for (j=0;j<m;j++) {
                y = u(l1,j); z = u(i,j);
                u(l1,j) = y * c + z * s;
                u(i,j) = -y * s + z * c;
              }
            }
          }
          test_f_convergence:
          z = q[k];
          if (l == k) goto convergence;

          iter++;
          if (iter > 30) { retval = k; break; }
          x = q[l]; y = q[k-1]; g = e[k-1]; h = e[k];
          f = ((y-z)*(y+z) + (g-h)*(g+h)) / (2*h*y);
          g = sqrt(f*f + 1.0);
          f = ((x-z)*(x+z) + h*(y/((f<0)?(f-g):(f+g))-h))/x;

          // Next QR transformation
          c = s = 1.0;
          for (i=l+1;i<=k;i++) {
            g = e[i]; y = q[i]; h = s * g; g *= c;
            e[i-1] = z = sqrt(f*f+h*h);
            c = f / z; s = h / z;
            f = x * c + g * s; g = -x * s + g * c;
            h = y * s; y *= c;
            if (withv) {
              for (j=0;j<n;j++) {
                x = v(i-1,j); z = v(i,j);
                v(i-1,j) = x * c + z * s;
                v(i,j) = -x * s + z * c;
              }
            }
            q[i-1] = z = sqrt(f*f + h*h);
            c = f/z; s = h/z;
            f = c * g + s * y; x = -s * g + c * y;
            if (withu) {
              for (j=0;j<m;j++) {
                y = u(i-1,j); z = u(i,j);
                u(i-1,j) = y * c + z * s;
                u(i,j) = -y * s + z * c;
              }
            }
          }
          e[l] = 0.0; e[k] = f; q[k] = x;
          goto test_f_splitting;

          convergence:
          if (z < 0.0) {
            q[k] = -z;
            if (withv)
              for (j=0;j<n;j++) v(k,j) = -v(k,j);
          }
        }
        return retval;
      }

      // ================================================================
      // GESDD wrapper: raw pointer interface → DynMatrix SVD
      // ================================================================

      template<class T>
      int cpp_gesdd(char jobz, int M, int N, T* A, int lda,
                    T* S, T* U, int ldu, T* Vt, int ldvt) {
        int mn = std::min(M, N);

        // Convert to double (svd_bidiag operates on icl64f)
        DynMatrix<icl64f> Ad(N, M);
        for(int i = 0; i < M; ++i)
          for(int j = 0; j < N; ++j)
            Ad(j, i) = static_cast<icl64f>(A[i * lda + j]);

        DynMatrix<icl64f> Ud(M, M), Vd(N, N);
        std::vector<icl64f> sq(std::max(M, N));

        int r;
        if (N > M)
          r = svd_bidiag(N, M, 1, 1, 1e-18, 1e-18, Ad.transp(), sq.data(), Vd, Ud);
        else
          r = svd_bidiag(M, N, 1, 1, 1e-18, 1e-18, Ad, sq.data(), Ud, Vd);

        if(r) return r;

        // Sort singular values descending
        struct IdxVal { icl64f val; int idx; bool operator<(const IdxVal& o) const { return val > o.val; } };
        std::vector<IdxVal> sorted(std::max(M, N));
        for(int i = 0; i < std::max(M, N); ++i)
          sorted[i] = {(i < mn) ? sq[i] : 0.0, i};
        std::sort(sorted.begin(), sorted.end());

        // Copy sorted singular values
        for(int i = 0; i < mn; ++i)
          S[i] = static_cast<T>(sorted[i].val);

        // Copy sorted U and Vt
        if(jobz != 'N') {
          for(int j = 0; j < mn; ++j) {
            int jidx = sorted[j].idx;
            for(int i = 0; i < M; ++i)
              U[i * ldu + j] = static_cast<T>(Ud(jidx, i));
          }
          for(int j = 0; j < mn; ++j) {
            int jidx = sorted[j].idx;
            for(int i = 0; i < N; ++i)
              Vt[j * ldvt + i] = static_cast<T>(Vd(jidx, i));
          }
        }

        return 0;
      }

    } // anonymous namespace

    static const int _cpp_blas_reg = []() {
      auto cpp_f = BlasOps<float>::instance().backends(Backend::Cpp);
      cpp_f.add<BlasOps<float>::GemmSig>(BlasOp::gemm, cpp_gemm<float>, "C++ naive GEMM");
      cpp_f.add<BlasOps<float>::GesddSig>(BlasOp::gesdd, cpp_gesdd<float>, "C++ Golub-Kahan SVD");

      auto cpp_d = BlasOps<double>::instance().backends(Backend::Cpp);
      cpp_d.add<BlasOps<double>::GemmSig>(BlasOp::gemm, cpp_gemm<double>, "C++ naive GEMM");
      cpp_d.add<BlasOps<double>::GesddSig>(BlasOp::gesdd, cpp_gesdd<double>, "C++ Golub-Kahan SVD");

      return 0;
    }();

  } // namespace math
} // namespace icl
