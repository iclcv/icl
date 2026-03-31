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

    } // anonymous namespace

    static const int _eigen_lapack_reg = []() {
      auto eig_f = LapackOps<float>::instance().backends(Backend::Eigen);
      eig_f.add<LapackOps<float>::GesddSig>(LapackOp::gesdd, eigen_gesdd<float>, "Eigen JacobiSVD");
      eig_f.add<LapackOps<float>::SyevSig>(LapackOp::syev, eigen_syev<float>, "Eigen SelfAdjointEigenSolver");

      auto eig_d = LapackOps<double>::instance().backends(Backend::Eigen);
      eig_d.add<LapackOps<double>::GesddSig>(LapackOp::gesdd, eigen_gesdd<double>, "Eigen JacobiSVD");
      eig_d.add<LapackOps<double>::SyevSig>(LapackOp::syev, eigen_syev<double>, "Eigen SelfAdjointEigenSolver");

      return 0;
    }();

  } // namespace math
} // namespace icl
