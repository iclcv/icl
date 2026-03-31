/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/MathOps.h                          **
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

    /// Selector keys for math backend dispatch.
    /// Used by MathOps<T> singletons (one per scalar type).
    enum class MathOp : int {
      // Stats
      mean, var, meanvar,
      // Min/Max
      min, max, minmax,
      // Unary element-wise (sub-dispatched by UnaryMathFunc)
      unaryInplace, unaryCopy,
      // Unary with scalar constant (sub-dispatched by UnaryConstFunc)
      unaryConstInplace, unaryConstCopy,
      // Binary element-wise (sub-dispatched by BinaryMathFunc)
      binaryCopy,
    };

    ICLMath_API const char* toString(MathOp op);

    /// Sub-operation enums for grouped selectors
    enum class UnaryMathFunc : int {
      abs, log, exp, sqrt, sqr,
      sin, cos, tan, arcsin, arccos, arctan, reciprocal
    };

    enum class UnaryConstFunc : int {
      powc, addc, subc, mulc, divc
    };

    enum class BinaryMathFunc : int {
      add, sub, mul, div, pow, arctan2
    };

    /// Type-parameterized singleton for math backend dispatch.
    /// Context is int (unused — no applicability checks needed).
    /// Separate instances for float and double.
    template<class T>
    struct ICLMath_API MathOps : utils::BackendDispatching<int> {

      // --- Dispatch signatures ---
      using MeanSig    = T(const T*, int);
      using VarSig     = T(const T*, int);
      using MeanVarSig = void(const T*, int, T*, T*);

      using MinSig     = T(const T*, int, int, int*, int*);
      using MaxSig     = T(const T*, int, int, int*, int*);
      using MinMaxSig  = void(const T*, int, int, T*, int*, int*, int*, int*);

      using UnaryInplaceSig     = void(UnaryMathFunc, T*, int);
      using UnaryCopySig        = void(UnaryMathFunc, const T*, T*, int);
      using UnaryConstInplaceSig = void(UnaryConstFunc, T*, T, int);
      using UnaryConstCopySig   = void(UnaryConstFunc, const T*, T, T*, int);
      using BinaryCopySig       = void(BinaryMathFunc, const T*, const T*, T*, int);

      MathOps();
      static MathOps& instance();
    };

  } // namespace math
} // namespace icl
