/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/LapackOps.cpp                      **
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

#include <ICLMath/LapackOps.h>

using namespace icl::utils;

namespace icl {
  namespace math {

    const char* toString(LapackOp op) {
      switch(op) {
        case LapackOp::gesdd: return "gesdd";
        case LapackOp::syev:  return "syev";
        case LapackOp::getrf: return "getrf";
        case LapackOp::getri: return "getri";
        case LapackOp::geqrf: return "geqrf";
        case LapackOp::orgqr: return "orgqr";
        case LapackOp::gelsd: return "gelsd";
      }
      return "?";
    }

    template<class T>
    LapackOps<T>::LapackOps() {
      addSelector<GesddSig>(LapackOp::gesdd);
      addSelector<SyevSig>(LapackOp::syev);
      addSelector<GetrfSig>(LapackOp::getrf);
      addSelector<GetriSig>(LapackOp::getri);
      addSelector<GeqrfSig>(LapackOp::geqrf);
      addSelector<OrgqrSig>(LapackOp::orgqr);
      addSelector<GelsdSig>(LapackOp::gelsd);
    }

    template<class T>
    LapackOps<T>& LapackOps<T>::instance() {
      static LapackOps<T> ops;
      return ops;
    }

    template struct LapackOps<float>;
    template struct LapackOps<double>;

  } // namespace math
} // namespace icl
