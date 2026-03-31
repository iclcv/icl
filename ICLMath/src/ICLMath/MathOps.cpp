/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/MathOps.cpp                        **
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

#include <ICLMath/MathOps.h>

namespace icl {
  namespace math {

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

  } // namespace math
} // namespace icl
