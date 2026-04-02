// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLMath/MathOps.h>

namespace icl::math {
    const char* toString(MathOp op) {
      switch(op) {
        case MathOp::mean:              return "mean";
        case MathOp::var:               return "var";
        case MathOp::meanvar:           return "meanvar";
        case MathOp::min:               return "min";
        case MathOp::max:               return "max";
        case MathOp::minmax:            return "minmax";
        case MathOp::unaryInplace:      return "unaryInplace";
        case MathOp::unaryCopy:         return "unaryCopy";
        case MathOp::unaryConstInplace: return "unaryConstInplace";
        case MathOp::unaryConstCopy:    return "unaryConstCopy";
        case MathOp::binaryCopy:        return "binaryCopy";
      }
      return "?";
    }

    template<class T>
    MathOps<T>::MathOps() {
      addSelector<MeanSig>(MathOp::mean);
      addSelector<VarSig>(MathOp::var);
      addSelector<MeanVarSig>(MathOp::meanvar);
      addSelector<MinSig>(MathOp::min);
      addSelector<MaxSig>(MathOp::max);
      addSelector<MinMaxSig>(MathOp::minmax);
      addSelector<UnaryInplaceSig>(MathOp::unaryInplace);
      addSelector<UnaryCopySig>(MathOp::unaryCopy);
      addSelector<UnaryConstInplaceSig>(MathOp::unaryConstInplace);
      addSelector<UnaryConstCopySig>(MathOp::unaryConstCopy);
      addSelector<BinaryCopySig>(MathOp::binaryCopy);
    }

    template<class T>
    MathOps<T>& MathOps<T>::instance() {
      static MathOps<T> ops;
      return ops;
    }

    // Explicit instantiations
    template struct MathOps<float>;
    template struct MathOps<double>;

  } // namespace icl::math