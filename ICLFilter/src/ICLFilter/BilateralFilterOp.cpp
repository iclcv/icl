/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BilateralFilterOp.cpp          **
** Module : ICLFilter                                              **
** Authors: Tobias Roehlig, Christof Elbrechter                    **
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

#include <ICLFilter/BilateralFilterOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
namespace filter {

  const char* toString(BilateralFilterOp::Op op) {
    switch(op) {
      case BilateralFilterOp::Op::apply: return "apply";
    }
    return "?";
  }

  core::ImageBackendDispatching& BilateralFilterOp::prototype() {
    static core::ImageBackendDispatching proto;
    static bool init = [&] {
      proto.addSelector<ApplySig>(Op::apply);
      return true;
    }();
    (void)init;
    return proto;
  }

  BilateralFilterOp::BilateralFilterOp(int radius, float sigma_s, float sigma_r, bool use_lab)
    : ImageBackendDispatching(prototype()),
      m_radius(radius), m_sigmaS(sigma_s), m_sigmaR(sigma_r), m_useLAB(use_lab)
  {}

  void BilateralFilterOp::apply(const Image &src, Image &dst) {
    if(!prepare(dst, src)) return;

    auto* impl = getSelector<ApplySig>(Op::apply).resolve(src);
    if(!impl) {
      ERROR_LOG("no applicable backend for BilateralFilterOp");
      return;
    }
    impl->apply(src, dst, m_radius, m_sigmaS, m_sigmaR, m_useLAB);
  }

} // namespace filter
} // namespace icl
