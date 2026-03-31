/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/BlasOps.h                          **
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

    /// Selector keys for BLAS backend dispatch.
    enum class BlasOp : int { gemm };

    ICLMath_API const char* toString(BlasOp op);

    /// BLAS dispatch — parameterized on scalar type (float or double).
    /// Operates on raw data pointers. Higher-level DynMatrix wrapping stays
    /// in consumer code (DynMatrix.cpp, DynMatrixUtils.cpp).
    ///
    /// Backends: C++ fallback (always), MKL, Accelerate, OpenBLAS.
    /// Context is int (unused — no applicability checks needed).
    ///
    /// Note: LAPACK operations (gesdd, syev, etc.) are in LapackOps.
    template<class T>
    struct ICLMath_API BlasOps : utils::BackendDispatching<int> {

      /// General matrix multiply: C = alpha * op(A) * op(B) + beta * C
      /// transA/transB: false = no transpose, true = transpose
      /// M = rows of op(A), N = cols of op(B), K = cols of op(A) = rows of op(B)
      using GemmSig = void(bool transA, bool transB,
                            int M, int N, int K, T alpha,
                            const T* A, int lda, const T* B, int ldb,
                            T beta, T* C, int ldc);

      BlasOps();
      static BlasOps& instance();
    };

  } // namespace math
} // namespace icl
