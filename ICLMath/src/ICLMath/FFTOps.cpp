/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/FFTOps.cpp                         **
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

#include <ICLMath/FFTOps.h>

namespace icl {
  namespace math {

    const char* toString(FFTOp op) {
      switch(op) {
        case FFTOp::r2c:     return "r2c";
        case FFTOp::c2c:     return "c2c";
        case FFTOp::inv_c2c: return "inv_c2c";
      }
      return "?";
    }

    template<class T>
    FFTOps<T>::FFTOps() {
      addSelector<R2CSig>(FFTOp::r2c);
      addSelector<C2CSig>(FFTOp::c2c);
      addSelector<InvC2CSig>(FFTOp::inv_c2c);
    }

    template<class T>
    FFTOps<T>& FFTOps<T>::instance() {
      static FFTOps<T> ops;
      return ops;
    }

    template struct FFTOps<float>;
    template struct FFTOps<double>;

  } // namespace math
} // namespace icl
