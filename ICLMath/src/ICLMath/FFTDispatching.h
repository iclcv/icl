/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/FFTDispatching.h                   **
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
#include <ICLMath/DynMatrix.h>
#include <complex>

namespace icl {
  namespace math {

    /// Context for FFT backend dispatch — carries problem dimensions
    struct FFTContext {
      unsigned rows, cols;
    };

    /// Forward FFT: real icl32f → complex icl32c
    using FFTFwd32fSig = DynMatrix<icl32c>&(
      const DynMatrix<icl32f>&, DynMatrix<icl32c>&, DynMatrix<icl32c>&);

    /// Inverse FFT: complex icl32c → complex icl32c
    using FFTInv32fSig = DynMatrix<icl32c>&(
      const DynMatrix<icl32c>&, DynMatrix<icl32c>&, DynMatrix<icl32c>&);

    /// Forward FFT: complex icl32c → complex icl32c (for already-complex input)
    using FFTFwd32fcSig = DynMatrix<icl32c>&(
      const DynMatrix<icl32c>&, DynMatrix<icl32c>&, DynMatrix<icl32c>&);

    /// Applicability: power-of-2 dimensions (required by IPP)
    inline bool fftPowerOf2(const FFTContext& ctx) {
      return ctx.rows > 0 && (ctx.rows & (ctx.rows - 1)) == 0
          && ctx.cols > 0 && (ctx.cols & (ctx.cols - 1)) == 0;
    }

    /// Selector keys for FFT dispatch
    enum class FFTOp : int { fwd32f, inv32f, fwd32fc };
    ICLMath_API const char* toString(FFTOp op);

    /// Singleton dispatch holder for FFT backends.
    /// C++ backends are registered here; IPP/MKL/OpenCL backends self-register
    /// from their respective _Ipp.cpp / _Mkl.cpp / _OpenCL.cpp files.
    struct ICLMath_API FFTDispatching : utils::BackendDispatching<FFTContext> {
      FFTDispatching();
      static FFTDispatching& instance();
    };

  } // namespace math
} // namespace icl
