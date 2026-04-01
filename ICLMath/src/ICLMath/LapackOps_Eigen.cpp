/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/LapackOps_Eigen.cpp                **
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

// Eigen backend for LAPACK operations.
// This file is excluded from the build when Eigen3 is not found.

#include <ICLMath/LapackOps.h>

#include <Eigen/SVD>
#include <Eigen/Eigenvalues>
#include <Eigen/LU>
#include <Eigen/QR>

#include <algorithm>

using namespace icl::utils;

namespace icl {
  namespace math {

    namespace {

      // ---- GESDD (SVD via Eigen JacobiSVD) ----

      template<class T>
      int eigen_gesdd(char jobz, int M, int N, T* A, int lda,
                      T* S, T* U, int ldu, T* Vt, int ldvt) {
        using EigenMat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

        int mn = std::min(M, N);

        // Map input (row-major)
        Eigen::Map<EigenMat> mA(A, M, N);
        Eigen::JacobiSVD<EigenMat> svd(mA, Eigen::ComputeFullU | Eigen::ComputeFullV);

        // Copy singular values
        auto sv = svd.singularValues();
        for(int i = 0; i < mn; i++) S[i] = sv(i);

        if(jobz != 'N' && jobz != 'n') {
          // Copy U (M×M row-major)
          auto eU = svd.matrixU();
          for(int i = 0; i < M; i++)
            for(int j = 0; j < M; j++)
              U[i * ldu + j] = eU(i, j);

          // Copy Vt = V^T (N×N row-major)
          auto eV = svd.matrixV();
          for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++)
              Vt[i * ldvt + j] = eV(j, i); // transpose
        }

        return 0;
      }

      // ---- SYEV (Symmetric eigenvalue via Eigen SelfAdjointEigenSolver) ----

      template<class T>
      int eigen_syev(char jobz, int N, T* A, int lda, T* W) {
        using EigenMat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

        Eigen::Map<EigenMat> mA(A, N, N);
        Eigen::SelfAdjointEigenSolver<EigenMat> solver(mA,
            (jobz == 'V' || jobz == 'v') ? Eigen::ComputeEigenvectors : Eigen::EigenvaluesOnly);

        if(solver.info() != Eigen::Success) return -1;

        // Eigenvalues (ascending order, matches LAPACK convention)
        auto ev = solver.eigenvalues();
        for(int i = 0; i < N; i++) W[i] = ev(i);

        if(jobz == 'V' || jobz == 'v') {
          auto eV = solver.eigenvectors();
          for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++)
              A[i * lda + j] = eV(i, j);
        }

        return 0;
      }

      // ---- GETRF (LU factorization via Eigen PartialPivLU) ----

      template<class T>
      int eigen_getrf(int M, int N, T* A, int lda, int* ipiv) {
        using EigenMat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

        int mn = std::min(M, N);
        Eigen::Map<EigenMat> mA(A, M, N);
        Eigen::PartialPivLU<EigenMat> lu(mA.topLeftCorner(mn, mn));

        // Extract permutation as 1-based pivot indices (LAPACK convention)
        auto perm = lu.permutationP().indices();
        // Convert permutation vector to sequential swap representation
        std::vector<int> p(mn);
        for(int i = 0; i < mn; i++) p[i] = i;
        for(int i = 0; i < mn; i++) {
          int target = perm(i);
          // Find where target currently is
          int j = i;
          for(; j < mn; j++) if(p[j] == target) break;
          std::swap(p[i], p[j]);
          ipiv[i] = j + 1; // 1-based
        }

        // Write packed LU back to A
        auto luMat = lu.matrixLU();
        for(int i = 0; i < mn; i++)
          for(int j = 0; j < mn; j++)
            A[i * lda + j] = luMat(i, j);

        return 0;
      }

      // ---- GETRI (inverse from LU via Eigen) ----

      template<class T>
      int eigen_getri(int N, T* A, int lda, const int* ipiv) {
        using EigenMat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
        using EigenPerm = Eigen::PermutationMatrix<Eigen::Dynamic, Eigen::Dynamic>;

        // Reconstruct LU and permutation
        Eigen::Map<EigenMat> mA(A, N, N);
        EigenMat L = EigenMat::Identity(N, N);
        EigenMat U = EigenMat::Zero(N, N);
        for(int i = 0; i < N; i++) {
          for(int j = 0; j < i; j++) L(i, j) = mA(i, j);
          for(int j = i; j < N; j++) U(i, j) = mA(i, j);
        }

        // Reconstruct permutation from ipiv
        EigenPerm P(N);
        P.setIdentity();
        for(int i = N - 1; i >= 0; i--) {
          int j = ipiv[i] - 1; // 0-based
          if(j != i) {
            P.applyTranspositionOnTheLeft(i, j);
          }
        }

        EigenMat inv = (L * U).inverse();
        inv = inv * P.transpose();
        for(int i = 0; i < N; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = inv(i, j);

        return 0;
      }

      // ---- GEQRF (QR factorization via Eigen HouseholderQR) ----

      template<class T>
      int eigen_geqrf(int M, int N, T* A, int lda, T* tau) {
        using EigenMat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

        int mn = std::min(M, N);
        Eigen::Map<EigenMat> mA(A, M, N);
        Eigen::HouseholderQR<EigenMat> qr(mA);

        // Extract R and Householder coefficients
        auto matQR = qr.matrixQR();
        auto hCoeffs = qr.hCoeffs();

        // Write packed QR back to A (R in upper triangle, reflectors below)
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = matQR(i, j);

        // Copy Householder coefficients to tau
        for(int i = 0; i < mn; i++)
          tau[i] = hCoeffs(i);

        return 0;
      }

      // ---- ORGQR (form Q from Householder reflectors) ----
      // We use Eigen's householderQ() to reconstruct Q.

      template<class T>
      int eigen_orgqr(int M, int N, int K, T* A, int lda, const T* tau) {
        using EigenMat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
        using EigenVec = Eigen::Matrix<T, Eigen::Dynamic, 1>;

        // Reconstruct Q using HouseholderSequence
        // Build the HouseholderQR result manually
        EigenMat matQR(M, N);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            matQR(i, j) = A[i * lda + j];

        EigenVec hCoeffs(K);
        for(int i = 0; i < K; i++)
          hCoeffs(i) = tau[i];

        auto hh = Eigen::HouseholderSequence<EigenMat, EigenVec>(matQR, hCoeffs);
        EigenMat Q = EigenMat::Identity(M, N);
        Q.applyOnTheLeft(hh);

        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = Q(i, j);

        return 0;
      }

      // ---- GELSD (least-squares solve via Eigen BDCSVD) ----

      template<class T>
      int eigen_gelsd(int M, int N, int NRHS, T* A, int lda,
                      T* B, int ldb, T* S, T rcond, int* rank) {
        using EigenMat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

        Eigen::Map<EigenMat> mA(A, M, N);
        int mx = std::max(M, N);

        // Map B (mx × NRHS)
        EigenMat mB(mx, NRHS);
        for(int i = 0; i < mx; i++)
          for(int j = 0; j < NRHS; j++)
            mB(i, j) = B[i * ldb + j];

        Eigen::JacobiSVD<EigenMat> svd(mA, Eigen::ComputeThinU | Eigen::ComputeThinV);

        // Copy singular values
        int mn = std::min(M, N);
        auto sv = svd.singularValues();
        for(int i = 0; i < mn; i++) S[i] = sv(i);

        // Determine rank
        T threshold = rcond;
        if(threshold < T(0)) threshold = std::numeric_limits<T>::epsilon() * mx * S[0];
        else threshold *= S[0];
        int r = 0;
        for(int i = 0; i < mn; i++)
          if(S[i] > threshold) r++;
        *rank = r;

        // Solve using SVD
        svd.setThreshold(rcond < T(0) ? std::numeric_limits<T>::epsilon() * mx : rcond);
        EigenMat X = svd.solve(mB.topRows(M));

        // Copy solution into B (first N rows)
        for(int i = 0; i < N; i++)
          for(int j = 0; j < NRHS; j++)
            B[i * ldb + j] = X(i, j);

        return 0;
      }

    } // anonymous namespace

    static const int _eigen_lapack_reg = []() {
      auto eig_f = LapackOps<float>::instance().backends(Backend::Eigen);
      eig_f.add<LapackOps<float>::GesddSig>(LapackOp::gesdd, eigen_gesdd<float>, "Eigen JacobiSVD");
      eig_f.add<LapackOps<float>::SyevSig>(LapackOp::syev, eigen_syev<float>, "Eigen SelfAdjointEigenSolver");
      eig_f.add<LapackOps<float>::GetrfSig>(LapackOp::getrf, eigen_getrf<float>, "Eigen PartialPivLU");
      eig_f.add<LapackOps<float>::GetriSig>(LapackOp::getri, eigen_getri<float>, "Eigen LU inverse");
      eig_f.add<LapackOps<float>::GeqrfSig>(LapackOp::geqrf, eigen_geqrf<float>, "Eigen HouseholderQR");
      eig_f.add<LapackOps<float>::OrgqrSig>(LapackOp::orgqr, eigen_orgqr<float>, "Eigen form Q");
      eig_f.add<LapackOps<float>::GelsdSig>(LapackOp::gelsd, eigen_gelsd<float>, "Eigen BDCSVD solve");

      auto eig_d = LapackOps<double>::instance().backends(Backend::Eigen);
      eig_d.add<LapackOps<double>::GesddSig>(LapackOp::gesdd, eigen_gesdd<double>, "Eigen JacobiSVD");
      eig_d.add<LapackOps<double>::SyevSig>(LapackOp::syev, eigen_syev<double>, "Eigen SelfAdjointEigenSolver");
      eig_d.add<LapackOps<double>::GetrfSig>(LapackOp::getrf, eigen_getrf<double>, "Eigen PartialPivLU");
      eig_d.add<LapackOps<double>::GetriSig>(LapackOp::getri, eigen_getri<double>, "Eigen LU inverse");
      eig_d.add<LapackOps<double>::GeqrfSig>(LapackOp::geqrf, eigen_geqrf<double>, "Eigen HouseholderQR");
      eig_d.add<LapackOps<double>::OrgqrSig>(LapackOp::orgqr, eigen_orgqr<double>, "Eigen form Q");
      eig_d.add<LapackOps<double>::GelsdSig>(LapackOp::gelsd, eigen_gelsd<double>, "Eigen BDCSVD solve");

      return 0;
    }();

  } // namespace math
} // namespace icl
