// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

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
