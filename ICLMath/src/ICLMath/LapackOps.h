/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/LapackOps.h                        **
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

#pragma once

#include <ICLUtils/BackendDispatching.h>
#include <ICLUtils/CompatMacros.h>

namespace icl {
  namespace math {

    /// Selector keys for LAPACK backend dispatch.
    enum class LapackOp : int { gesdd, syev, getrf, getri };

    ICLMath_API const char* toString(LapackOp op);

    /// LAPACK dispatch — parameterized on scalar type (float or double).
    /// Operates on raw data pointers. Higher-level DynMatrix wrapping stays
    /// in consumer code (DynMatrix.cpp, DynMatrixUtils.cpp).
    ///
    /// Backends: C++ fallback (always), MKL, Accelerate, OpenBLAS.
    /// Context is int (unused — no applicability checks needed).
    template<class T>
    struct ICLMath_API LapackOps : utils::BackendDispatching<int> {

      /// SVD via divide-and-conquer: A = U * diag(S) * Vt
      using GesddSig = int(char jobz, int M, int N, T* A, int lda,
                            T* S, T* U, int ldu, T* Vt, int ldvt);

      /// Symmetric eigenvalue decomposition: A = V * diag(W) * V^T
      using SyevSig = int(char jobz, int N, T* A, int lda, T* W);

      /// LU factorization with partial pivoting: A = P * L * U
      /// A is M×N, overwritten with L (unit lower) and U (upper).
      /// ipiv[min(M,N)] receives 1-based pivot indices.
      /// Returns info (0 = success, >0 = singular).
      using GetrfSig = int(int M, int N, T* A, int lda, int* ipiv);

      /// Matrix inverse from LU factorization (getrf output).
      /// A is N×N (LU from getrf), overwritten with A^{-1}.
      /// ipiv from getrf.
      /// Returns info (0 = success).
      using GetriSig = int(int N, T* A, int lda, const int* ipiv);

      LapackOps();
      static LapackOps& instance();
    };

  } // namespace math
} // namespace icl
