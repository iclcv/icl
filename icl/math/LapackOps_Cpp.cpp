// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// C++ fallback backends for LAPACK operations.

#include <icl/math/LapackOps.h>
#include <icl/math/BlasOps.h>
#include <icl/math/DynMatrix.h>
#include <icl/utils/BasicTypes.h>

#include <vector>
#include <algorithm>
#include <cmath>

using namespace icl::utils;

namespace icl::math {
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
          u(i, j) = a(i, j);

      // Householder's reduction to bidiagonal form
      g = x = 0.0;
      for (i=0;i<n;i++) {
        e[i] = g;
        s = 0.0;
        l = i+1;
        for (j=i;j<m;j++) s += u(j, i)*u(j, i);
        if (s < tol) { g = 0.0; }
        else {
          f = u(i, i);
          g = (f < 0) ? sqrt(s) : -sqrt(s);
          h = f * g - s;
          u(i, i) = f - g;
          for (j=l;j<n;j++) {
            s = 0.0;
            for (k=i;k<m;k++) s += u(k, i) * u(k, j);
            f = s / h;
            for (k=i;k<m;k++) u(k, j) += f * u(k, i);
          }
        }
        q[i] = g;
        s = 0.0;
        for (j=l;j<n;j++) s += u(i, j) * u(i, j);
        if (s < tol) { g = 0.0; }
        else {
          f = u(i, i+1);
          g = (f < 0) ? sqrt(s) : -sqrt(s);
          h = f * g - s;
          u(i, i+1) = f - g;
          for (j=l;j<n;j++) e[j] = u(i, j)/h;
          for (j=l;j<m;j++) {
            s = 0.0;
            for (k=l;k<n;k++) s += u(j, k) * u(i, k);
            for (k=l;k<n;k++) u(j, k) += s * e[k];
          }
        }
        y = fabs(q[i]) + fabs(e[i]);
        if (y > x) x = y;
      }

      // Accumulation of right-hand transformations
      if (withv) {
        for (i=n-1;i>=0;i--) {
          if (g != 0.0) {
            h = u(i, i+1) * g;
            for (j=l;j<n;j++) v(j, i) = u(i, j)/h;
            for (j=l;j<n;j++) {
              s = 0.0;
              for (k=l;k<n;k++) s += u(i, k) * v(k, j);
              for (k=l;k<n;k++) v(k, j) += s * v(k, i);
            }
          }
          for (j=l;j<n;j++) v(i, j) = v(j, i) = 0.0;
          v(i, i) = 1.0;
          g = e[i];
          l = i;
        }
      }

      // Accumulation of left-hand transformations
      if (withu) {
        for (i=n;i<m;i++) {
          for (j=n;j<m;j++) u(i, j) = 0.0;
          u(i, i) = 1.0;
        }
        for (i=n-1;i>=0;i--) {
          l = i + 1;
          g = q[i];
          for (j=l;j<m;j++) u(i, j) = 0.0;
          if (g != 0.0) {
            h = u(i, i) * g;
            for (j=l;j<m;j++) {
              s = 0.0;
              for (k=l;k<m;k++) s += u(k, i) * u(k, j);
              f = s / h;
              for (k=i;k<m;k++) u(k, j) += f * u(k, i);
            }
            for (j=i;j<m;j++) u(j, i) /= g;
          } else {
            for (j=i;j<m;j++) u(j, i) = 0.0;
          }
          u(i, i) += 1.0;
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
              y = u(j, l1); z = u(j, i);
              u(j, l1) = y * c + z * s;
              u(j, i) = -y * s + z * c;
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
              x = v(j, i-1); z = v(j, i);
              v(j, i-1) = x * c + z * s;
              v(j, i) = -x * s + z * c;
            }
          }
          q[i-1] = z = sqrt(f*f + h*h);
          c = f/z; s = h/z;
          f = c * g + s * y; x = -s * g + c * y;
          if (withu) {
            for (j=0;j<m;j++) {
              y = u(j, i-1); z = u(j, i);
              u(j, i-1) = y * c + z * s;
              u(j, i) = -y * s + z * c;
            }
          }
        }
        e[l] = 0.0; e[k] = f; q[k] = x;
        goto test_f_splitting;

        convergence:
        if (z < 0.0) {
          q[k] = -z;
          if (withv)
            for (j=0;j<n;j++) v(j, k) = -v(j, k);
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
          Ad(i, j) = static_cast<icl64f>(A[i * lda + j]);

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
            U[i * ldu + j] = static_cast<T>(Ud(i, jidx));
        }
        for(int j = 0; j < mn; ++j) {
          int jidx = sorted[j].idx;
          for(int i = 0; i < N; ++i)
            Vt[j * ldvt + i] = static_cast<T>(Vd(i, jidx));
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
      // Build inverse column-by-column: solve A * x_j = e_j for each j.
      // Since A = P*L*U (from getrf), we solve by:
      //   1. Apply P to e_j
      //   2. Forward-substitute with L (unit lower, stored below diagonal)
      //   3. Back-substitute with U (stored on/above diagonal)
      std::vector<T> inv(N * N, T(0));
      std::vector<T> col(N);

      for(int j = 0; j < N; j++) {
        // Start with e_j
        std::fill(col.begin(), col.end(), T(0));
        col[j] = T(1);

        // Apply row permutation forward: for k=0..N-1, swap col[k] and col[ipiv[k]-1]
        for(int k = 0; k < N; k++) {
          int pk = ipiv[k] - 1;
          if(pk != k) std::swap(col[k], col[pk]);
        }

        // Forward substitution with L (unit diagonal)
        for(int i = 1; i < N; i++) {
          for(int k = 0; k < i; k++)
            col[i] -= A[i * lda + k] * col[k];
        }

        // Back substitution with U
        for(int i = N - 1; i >= 0; i--) {
          if(A[i * lda + i] == T(0)) return i + 1;
          for(int k = i + 1; k < N; k++)
            col[i] -= A[i * lda + k] * col[k];
          col[i] /= A[i * lda + i];
        }

        // Store column j of inverse
        for(int i = 0; i < N; i++)
          inv[i * N + j] = col[i];
      }

      // Copy result back to A
      for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++)
          A[i * lda + j] = inv[i * N + j];

      return 0;
    }

    // ================================================================
    // GEQRF: Householder QR factorization
    // A (M×N, M >= N) is overwritten: R in upper triangle,
    // Householder reflectors v below diagonal (v[0]=1 implicit).
    // tau[min(M,N)] receives scalar factors.
    // ================================================================

    template<class T>
    int cpp_geqrf(int M, int N, T* A, int lda, T* tau) {
      int mn = std::min(M, N);
      std::vector<T> v(M);

      for(int k = 0; k < mn; k++) {
        // Compute Householder reflector for column k, rows k..M-1
        T norm2 = T(0);
        for(int i = k + 1; i < M; i++) {
          T val = A[i * lda + k];
          norm2 += val * val;
        }

        if(norm2 == T(0) && A[k * lda + k] >= T(0)) {
          tau[k] = T(0);
          continue;
        }

        T xk = A[k * lda + k];
        T alpha = std::sqrt(xk * xk + norm2);
        if(xk >= T(0)) alpha = -alpha;

        tau[k] = (alpha - xk) / alpha;
        T scale = T(1) / (xk - alpha);
        for(int i = k + 1; i < M; i++)
          A[i * lda + k] *= scale;
        A[k * lda + k] = alpha;

        // Apply reflector to trailing columns: A[k:M, k+1:N]
        // H = I - tau * v * v^T, where v = [1; A[k+1:M, k]]
        for(int j = k + 1; j < N; j++) {
          T dot = A[k * lda + j];
          for(int i = k + 1; i < M; i++)
            dot += A[i * lda + k] * A[i * lda + j];
          dot *= tau[k];
          A[k * lda + j] -= dot;
          for(int i = k + 1; i < M; i++)
            A[i * lda + j] -= dot * A[i * lda + k];
        }
      }
      return 0;
    }

    // ================================================================
    // ORGQR: Form Q from Householder reflectors (geqrf output).
    // A is M×N, reflectors in columns 0..K-1.
    // On exit, A is overwritten with the first N columns of Q.
    // ================================================================

    template<class T>
    int cpp_orgqr(int M, int N, int K, T* A, int lda, const T* tau) {
      // Initialize columns K..N-1 to identity
      for(int j = K; j < N; j++) {
        for(int i = 0; i < M; i++)
          A[i * lda + j] = T(0);
        if(j < M) A[j * lda + j] = T(1);
      }

      // Accumulate reflectors from right to left
      for(int k = K - 1; k >= 0; k--) {
        // Apply H_k = I - tau[k] * v * v^T to A[k:M, k:N]
        // where v = [1; A[k+1:M, k]]

        // First, set column k above the reflector to identity
        if(tau[k] != T(0)) {
          // Apply to trailing columns k+1..N-1 first
          for(int j = k + 1; j < N; j++) {
            T dot = A[k * lda + j];
            for(int i = k + 1; i < M; i++)
              dot += A[i * lda + k] * A[i * lda + j];
            dot *= tau[k];
            A[k * lda + j] -= dot;
            for(int i = k + 1; i < M; i++)
              A[i * lda + j] -= dot * A[i * lda + k];
          }

          // Apply to column k itself: v * (1 - tau) ... build e_k column
          for(int i = k + 1; i < M; i++)
            A[i * lda + k] *= -tau[k];
        } else {
          for(int i = k + 1; i < M; i++)
            A[i * lda + k] = T(0);
        }
        A[k * lda + k] = T(1) - tau[k];

        // Zero above k
        for(int i = 0; i < k; i++)
          A[i * lda + k] = T(0);
      }
      return 0;
    }

    // ================================================================
    // GELSD: Least-squares solve via SVD.
    // C++ fallback: uses gesdd (reduced) + gemm to compute pinv * B.
    // ================================================================

    template<class T>
    int cpp_gelsd(int M, int N, int NRHS, T* A, int lda,
                  T* B, int ldb, T* S, T rcond, int* rank) {
      int mn = std::min(M, N);
      int mx = std::max(M, N);

      // Resolve gesdd and gemm backends
      auto* svdImpl = LapackOps<T>::instance()
          .template getSelector<typename LapackOps<T>::GesddSig>(LapackOp::gesdd)
          .resolveOrThrow();
      auto* gemmImpl = BlasOps<T>::instance()
          .template getSelector<typename BlasOps<T>::GemmSig>(BlasOp::gemm)
          .resolveOrThrow();

      // Reduced SVD: A = U * diag(S) * Vt
      std::vector<T> Adata(M * N);
      std::copy(A, A + M * N, Adata.data());
      std::vector<T> U(M * mn), Vt(mn * N);

      int info = svdImpl->apply('S', M, N, Adata.data(), N,
                                 S, U.data(), mn, Vt.data(), N);
      if(info != 0) return info;

      // Determine effective rank
      T threshold = rcond;
      if(threshold < T(0)) threshold = std::numeric_limits<T>::epsilon() * mx * S[0];
      else threshold *= S[0];
      int r = 0;
      for(int i = 0; i < mn; i++)
        if(S[i] > threshold) r++;
      *rank = r;

      // Build Sinv (r × r diagonal, zero the rest)
      std::vector<T> Sinv(mn * mn, T(0));
      for(int i = 0; i < r; i++)
        Sinv[i * mn + i] = T(1) / S[i];

      // pinv(A) * B = Vt^T * Sinv * U^T * B
      // Step 1: temp1 = U^T * B (mn × NRHS)
      std::vector<T> temp1(mn * NRHS);
      gemmImpl->apply(true, false, mn, NRHS, M, T(1),
                       U.data(), mn, B, ldb,
                       T(0), temp1.data(), NRHS);

      // Step 2: temp2 = Sinv * temp1 (mn × NRHS)
      std::vector<T> temp2(mn * NRHS);
      gemmImpl->apply(false, false, mn, NRHS, mn, T(1),
                       Sinv.data(), mn, temp1.data(), NRHS,
                       T(0), temp2.data(), NRHS);

      // Step 3: X = Vt^T * temp2 (N × NRHS) — write into B
      // B must be max(M,N) × NRHS; solution goes in first N rows
      std::vector<T> X(N * NRHS);
      gemmImpl->apply(true, false, N, NRHS, mn, T(1),
                       Vt.data(), N, temp2.data(), NRHS,
                       T(0), X.data(), NRHS);

      // Copy solution into B (first N rows)
      for(int i = 0; i < N; i++)
        for(int j = 0; j < NRHS; j++)
          B[i * ldb + j] = X[i * NRHS + j];

      return 0;
    }

  } // anonymous namespace

  static const int _cpp_lapack_reg = []() {
    auto cpp_f = LapackOps<float>::instance().backends(Backend::Cpp);
    cpp_f.add<LapackOps<float>::GesddSig>(LapackOp::gesdd, cpp_gesdd<float>, "C++ Golub-Kahan SVD");
    cpp_f.add<LapackOps<float>::SyevSig>(LapackOp::syev, cpp_syev<float>, "C++ Jacobi eigenvalue");
    cpp_f.add<LapackOps<float>::GetrfSig>(LapackOp::getrf, cpp_getrf<float>, "C++ LU factorization");
    cpp_f.add<LapackOps<float>::GetriSig>(LapackOp::getri, cpp_getri<float>, "C++ LU inverse");
    cpp_f.add<LapackOps<float>::GeqrfSig>(LapackOp::geqrf, cpp_geqrf<float>, "C++ Householder QR");
    cpp_f.add<LapackOps<float>::OrgqrSig>(LapackOp::orgqr, cpp_orgqr<float>, "C++ form Q");
    cpp_f.add<LapackOps<float>::GelsdSig>(LapackOp::gelsd, cpp_gelsd<float>, "C++ SVD least-squares");

    auto cpp_d = LapackOps<double>::instance().backends(Backend::Cpp);
    cpp_d.add<LapackOps<double>::GesddSig>(LapackOp::gesdd, cpp_gesdd<double>, "C++ Golub-Kahan SVD");
    cpp_d.add<LapackOps<double>::SyevSig>(LapackOp::syev, cpp_syev<double>, "C++ Jacobi eigenvalue");
    cpp_d.add<LapackOps<double>::GetrfSig>(LapackOp::getrf, cpp_getrf<double>, "C++ LU factorization");
    cpp_d.add<LapackOps<double>::GetriSig>(LapackOp::getri, cpp_getri<double>, "C++ LU inverse");
    cpp_d.add<LapackOps<double>::GeqrfSig>(LapackOp::geqrf, cpp_geqrf<double>, "C++ Householder QR");
    cpp_d.add<LapackOps<double>::OrgqrSig>(LapackOp::orgqr, cpp_orgqr<double>, "C++ form Q");
    cpp_d.add<LapackOps<double>::GelsdSig>(LapackOp::gelsd, cpp_gelsd<double>, "C++ SVD least-squares");

    return 0;
  }();

  } // namespace icl::math