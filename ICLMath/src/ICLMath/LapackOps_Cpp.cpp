/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/LapackOps_Cpp.cpp                  **
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

// C++ fallback backends for LAPACK operations.
// Contains Golub-Kahan bidiagonalization SVD (gesdd).

#include <ICLMath/LapackOps.h>
#include <ICLMath/DynMatrix.h>
#include <ICLUtils/BasicTypes.h>

#include <vector>
#include <algorithm>
#include <cmath>

using namespace icl::utils;

namespace icl {
  namespace math {

    namespace {

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


      // ================================================================
      // SYEV: Jacobi iteration for symmetric eigenvalue decomposition
      // Based on VTK's vtkJacobiN. Operates on T** arrays internally.
      // ================================================================

      template<class T>
      static int jacobi_iterate(T **a, int n, T *w, T **v) {
        T bspace[4], zspace[4];
        T *b = (n > 4) ? new T[n] : bspace;
        T *z = (n > 4) ? new T[n] : zspace;

        for(int ip = 0; ip < n; ip++) {
          for(int iq = 0; iq < n; iq++) v[ip][iq] = 0;
          v[ip][ip] = 1;
        }
        for(int ip = 0; ip < n; ip++) { b[ip] = w[ip] = a[ip][ip]; z[ip] = 0; }

        for(int i = 0; i < 30; i++) {
          T sm = 0;
          for(int ip = 0; ip < n-1; ip++)
            for(int iq = ip+1; iq < n; iq++)
              sm += std::abs(a[ip][iq]);
          if(sm == 0) break;

          T tresh = (i < 3) ? T(0.2) * sm / (n * n) : T(0);
          for(int ip = 0; ip < n-1; ip++) {
            for(int iq = ip+1; iq < n; iq++) {
              T g = T(100) * std::abs(a[ip][iq]);
              if(i > 3 && (std::abs(w[ip]) + g) == std::abs(w[ip])
                       && (std::abs(w[iq]) + g) == std::abs(w[iq])) {
                a[ip][iq] = 0;
              } else if(std::abs(a[ip][iq]) > tresh) {
                T h = w[iq] - w[ip];
                T t;
                if((std::abs(h) + g) == std::abs(h)) {
                  t = a[ip][iq] / h;
                } else {
                  T theta = T(0.5) * h / a[ip][iq];
                  t = T(1) / (std::abs(theta) + std::sqrt(T(1) + theta * theta));
                  if(theta < 0) t = -t;
                }
                T c = T(1) / std::sqrt(T(1) + t * t);
                T s = t * c;
                T tau = s / (T(1) + c);
                h = t * a[ip][iq];
                z[ip] -= h; z[iq] += h; w[ip] -= h; w[iq] += h;
                a[ip][iq] = 0;
#define JROT(a,i,j,k,l) { T g_=a[i][j]; T h_=a[k][l]; a[i][j]=g_-s*(h_+g_*tau); a[k][l]=h_+s*(g_-h_*tau); }
                for(int j = 0; j <= ip-1; j++) JROT(a,j,ip,j,iq);
                for(int j = ip+1; j <= iq-1; j++) JROT(a,ip,j,j,iq);
                for(int j = iq+1; j < n; j++) JROT(a,ip,j,iq,j);
                for(int j = 0; j < n; j++) JROT(v,j,ip,j,iq);
#undef JROT
              }
            }
          }
          for(int ip = 0; ip < n; ip++) { b[ip] += z[ip]; w[ip] = b[ip]; z[ip] = 0; }
        }

        // Sort eigenvalues descending
        for(int j = 0; j < n-1; j++) {
          int k = j; T tmp = w[k];
          for(int i = j+1; i < n; i++) { if(w[i] >= tmp) { k = i; tmp = w[k]; } }
          if(k != j) {
            w[k] = w[j]; w[j] = tmp;
            for(int i = 0; i < n; i++) { tmp = v[i][j]; v[i][j] = v[i][k]; v[i][k] = tmp; }
          }
        }
        // Ensure eigenvector consistency (most-positive)
        int half = (n >> 1) + (n & 1);
        for(int j = 0; j < n; j++) {
          int pos = 0;
          for(int i = 0; i < n; i++) if(v[i][j] >= 0) pos++;
          if(pos < half) for(int i = 0; i < n; i++) v[i][j] *= -1;
        }
        if(n > 4) { delete[] b; delete[] z; }
        return 0;
      }

      template<class T>
      int cpp_syev(char jobz, int N, T* A, int lda, T* W) {
        // Convert row-major raw pointer to T** for Jacobi iteration
        T** pa = new T*[N];
        T** pv = new T*[N];
        for(int i = 0; i < N; i++) {
          pa[i] = new T[N];
          pv[i] = new T[N];
          for(int j = 0; j < N; j++)
            pa[i][j] = A[i * lda + j];
        }

        jacobi_iterate(pa, N, W, pv);

        // Copy eigenvectors back to A if requested
        if(jobz == 'V' || jobz == 'v') {
          for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++)
              A[i * lda + j] = pv[i][j];
        }

        for(int i = 0; i < N; i++) { delete[] pa[i]; delete[] pv[i]; }
        delete[] pa; delete[] pv;
        return 0;
      }

      // ================================================================
      // GETRF: LU factorization with partial pivoting
      // A = P * L * U. A is overwritten with L (unit lower) and U (upper).
      // ipiv uses 1-based indexing (LAPACK convention).
      // ================================================================

      template<class T>
      int cpp_getrf(int M, int N, T* A, int lda, int* ipiv) {
        int mn = std::min(M, N);
        for(int k = 0; k < mn; k++) {
          // Find pivot
          int maxRow = k;
          T maxVal = std::abs(A[k * lda + k]);
          for(int i = k + 1; i < M; i++) {
            T v = std::abs(A[i * lda + k]);
            if(v > maxVal) { maxVal = v; maxRow = i; }
          }
          ipiv[k] = maxRow + 1; // 1-based

          if(maxVal == T(0)) return k + 1; // singular

          // Swap rows k and maxRow
          if(maxRow != k) {
            for(int j = 0; j < N; j++)
              std::swap(A[k * lda + j], A[maxRow * lda + j]);
          }

          // Eliminate below
          T pivot = A[k * lda + k];
          for(int i = k + 1; i < M; i++) {
            T factor = A[i * lda + k] / pivot;
            A[i * lda + k] = factor; // store L
            for(int j = k + 1; j < N; j++)
              A[i * lda + j] -= factor * A[k * lda + j];
          }
        }
        return 0;
      }

      // ================================================================
      // GETRI: Matrix inverse from LU factorization
      // A (N×N LU from getrf) is overwritten with A^{-1}.
      // ================================================================

      template<class T>
      int cpp_getri(int N, T* A, int lda, const int* ipiv) {
        // Invert U in place
        for(int j = N - 1; j >= 0; j--) {
          if(A[j * lda + j] == T(0)) return j + 1;
          A[j * lda + j] = T(1) / A[j * lda + j];
          for(int i = j - 1; i >= 0; i--) {
            T sum = T(0);
            for(int k = i + 1; k <= j; k++)
              sum += A[i * lda + k] * A[k * lda + j];
            A[i * lda + j] = -sum * A[i * lda + i];
          }
        }

        // Solve U^{-1} * L^{-1} by back-substitution
        // Work column by column from right to left
        std::vector<T> work(N);
        for(int j = N - 2; j >= 0; j--) {
          // Save column j below diagonal (L part)
          for(int i = j + 1; i < N; i++) { work[i] = A[i * lda + j]; A[i * lda + j] = T(0); }
          // Subtract L column from inverse
          for(int i = 0; i < N; i++) {
            T sum = T(0);
            for(int k = j + 1; k < N; k++)
              sum += A[i * lda + k] * work[k];
            A[i * lda + j] -= sum;
          }
        }

        // Apply pivot permutation in reverse
        for(int j = N - 1; j >= 0; j--) {
          int jp = ipiv[j] - 1; // 0-based
          if(jp != j) {
            for(int i = 0; i < N; i++)
              std::swap(A[i * lda + j], A[i * lda + jp]);
          }
        }
        return 0;
      }

    } // anonymous namespace

    static const int _cpp_lapack_reg = []() {
      auto cpp_f = LapackOps<float>::instance().backends(Backend::Cpp);
      cpp_f.add<LapackOps<float>::GesddSig>(LapackOp::gesdd, cpp_gesdd<float>, "C++ Golub-Kahan SVD");
      cpp_f.add<LapackOps<float>::SyevSig>(LapackOp::syev, cpp_syev<float>, "C++ Jacobi eigenvalue");
      cpp_f.add<LapackOps<float>::GetrfSig>(LapackOp::getrf, cpp_getrf<float>, "C++ LU factorization");
      cpp_f.add<LapackOps<float>::GetriSig>(LapackOp::getri, cpp_getri<float>, "C++ LU inverse");

      auto cpp_d = LapackOps<double>::instance().backends(Backend::Cpp);
      cpp_d.add<LapackOps<double>::GesddSig>(LapackOp::gesdd, cpp_gesdd<double>, "C++ Golub-Kahan SVD");
      cpp_d.add<LapackOps<double>::SyevSig>(LapackOp::syev, cpp_syev<double>, "C++ Jacobi eigenvalue");
      cpp_d.add<LapackOps<double>::GetrfSig>(LapackOp::getrf, cpp_getrf<double>, "C++ LU factorization");
      cpp_d.add<LapackOps<double>::GetriSig>(LapackOp::getri, cpp_getri<double>, "C++ LU inverse");

      return 0;
    }();

  } // namespace math
} // namespace icl
