/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/LapackOps_Mkl.cpp                  **
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

// MKL backend for LAPACK operations.
// This file is excluded from the build when MKL is not found.

#include <ICLMath/LapackOps.h>
#include "mkl_lapack.h"
#include <vector>
#include <algorithm>

using namespace icl::utils;

namespace icl {
  namespace math {

    namespace {

      // ---- GESDD (SVD) ----

      int mkl_gesdd_f(char jobz, int M, int N, float* A, int lda,
                      float* S, float* U, int ldu, float* Vt, int ldvt) {
        int info;
        int mn = std::min(M, N);
        std::vector<int> iwork(std::max(1, 8 * mn));

        float work_query;
        int lwork = -1;
        sgesdd(&jobz, &N, &M, A, &lda, S, Vt, &ldvt, U, &ldu,
               &work_query, &lwork, iwork.data(), &info);
        if(info != 0) return info;

        lwork = static_cast<int>(work_query);
        std::vector<float> work(lwork);

        sgesdd(&jobz, &N, &M, A, &lda, S, Vt, &ldvt, U, &ldu,
               work.data(), &lwork, iwork.data(), &info);
        return info;
      }

      int mkl_gesdd_d(char jobz, int M, int N, double* A, int lda,
                      double* S, double* U, int ldu, double* Vt, int ldvt) {
        int info;
        int mn = std::min(M, N);
        std::vector<int> iwork(std::max(1, 8 * mn));

        double work_query;
        int lwork = -1;
        dgesdd(&jobz, &N, &M, A, &lda, S, Vt, &ldvt, U, &ldu,
               &work_query, &lwork, iwork.data(), &info);
        if(info != 0) return info;

        lwork = static_cast<int>(work_query);
        std::vector<double> work(lwork);

        dgesdd(&jobz, &N, &M, A, &lda, S, Vt, &ldvt, U, &ldu,
               work.data(), &lwork, iwork.data(), &info);
        return info;
      }

      // ---- SYEV (symmetric eigenvalue) ----

      int mkl_syev_f(char jobz, int N, float* A, int lda, float* W) {
        int info;
        char uplo = 'U';

        float work_query;
        int lwork = -1;
        ssyev(&jobz, &uplo, &N, A, &lda, W, &work_query, &lwork, &info);
        if(info != 0) return info;

        lwork = static_cast<int>(work_query);
        std::vector<float> work(lwork);

        ssyev(&jobz, &uplo, &N, A, &lda, W, work.data(), &lwork, &info);
        return info;
      }

      int mkl_syev_d(char jobz, int N, double* A, int lda, double* W) {
        int info;
        char uplo = 'U';

        double work_query;
        int lwork = -1;
        dsyev(&jobz, &uplo, &N, A, &lda, W, &work_query, &lwork, &info);
        if(info != 0) return info;

        lwork = static_cast<int>(work_query);
        std::vector<double> work(lwork);

        dsyev(&jobz, &uplo, &N, A, &lda, W, work.data(), &lwork, &info);
        return info;
      }

    } // anonymous namespace

    static const int _mkl_lapack_reg = []() {
      auto mkl_f = LapackOps<float>::instance().backends(Backend::Mkl);
      mkl_f.add<LapackOps<float>::GesddSig>(LapackOp::gesdd, mkl_gesdd_f, "MKL sgesdd");
      mkl_f.add<LapackOps<float>::SyevSig>(LapackOp::syev, mkl_syev_f, "MKL ssyev");

      auto mkl_d = LapackOps<double>::instance().backends(Backend::Mkl);
      mkl_d.add<LapackOps<double>::GesddSig>(LapackOp::gesdd, mkl_gesdd_d, "MKL dgesdd");
      mkl_d.add<LapackOps<double>::SyevSig>(LapackOp::syev, mkl_syev_d, "MKL dsyev");

      return 0;
    }();

  } // namespace math
} // namespace icl
