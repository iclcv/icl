/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/FFTOps.h                           **
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
#include <complex>

namespace icl {
  namespace math {

    /// Selector keys for FFT backend dispatch.
    enum class FFTOp : int { r2c, c2c, inv_c2c };

    ICLMath_API const char* toString(FFTOp op);

    /// FFT dispatch — parameterized on scalar type (float or double).
    /// Operates on raw data pointers. DynMatrix wrapping stays in FFTUtils.
    ///
    /// All operations work on row-major 2D data of size rows x cols.
    /// Output must be pre-allocated by the caller (rows * cols complex values).
    ///
    /// Backends: C++ fallback (always), MKL DFTI, FFTW, Accelerate vDSP.
    /// Context is int (unused — no applicability checks needed).
    template<class T>
    struct ICLMath_API FFTOps : utils::BackendDispatching<int> {
      using C = std::complex<T>;

      /// Real-to-complex 2D forward FFT.
      /// src: row-major T[rows*cols], dst: pre-allocated C[rows*cols]
      using R2CSig = void(const T* src, int rows, int cols, C* dst);

      /// Complex-to-complex 2D forward FFT.
      /// src: row-major C[rows*cols], dst: pre-allocated C[rows*cols]
      using C2CSig = void(const C* src, int rows, int cols, C* dst);

      /// Complex-to-complex 2D inverse FFT (includes 1/(rows*cols) normalization).
      /// src: row-major C[rows*cols], dst: pre-allocated C[rows*cols]
      using InvC2CSig = void(const C* src, int rows, int cols, C* dst);

      FFTOps();
      static FFTOps& instance();
    };

  } // namespace math
} // namespace icl
