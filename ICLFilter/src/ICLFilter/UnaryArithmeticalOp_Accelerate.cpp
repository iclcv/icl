// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <Accelerate/Accelerate.h>

#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ICLCore/Visitors.h>

namespace {

  using namespace icl::core;
  using icl::icl32f;
  using UAOp = icl::filter::UnaryArithmeticalOp;

  // ================================================================
  // vDSP with-value ops (add/sub/mul/div scalar)
  // ================================================================

  void acc_arith_with_val(const Image &src, Image &dst, double value, int optype) {
    float fval = static_cast<float>(value);
    float neg  = -fval;
    float inv  = fval != 0.f ? 1.f / fval : 0.f;

    visitROILinesPerChannelWith(src.as32f(), dst.as32f(),
      [&](const icl32f *s, icl32f *d, int, int w) {
        vImagePixelCount n = static_cast<vImagePixelCount>(w);
        switch(optype) {
          case UAOp::addOp: vDSP_vsadd(s, 1, &fval, d, 1, n); break;
          case UAOp::subOp: vDSP_vsadd(s, 1, &neg,  d, 1, n); break;
          case UAOp::mulOp: vDSP_vsmul(s, 1, &fval, d, 1, n); break;
          case UAOp::divOp: vDSP_vsmul(s, 1, &inv,  d, 1, n); break;
        }
      });
  }

  // ================================================================
  // vDSP/vForce no-value ops (sqr/sqrt/ln/exp/abs)
  // ================================================================

  void acc_arith_no_val(const Image &src, Image &dst, int optype) {
    visitROILinesPerChannelWith(src.as32f(), dst.as32f(),
      [optype](const icl32f *s, icl32f *d, int, int w) {
        vImagePixelCount n = static_cast<vImagePixelCount>(w);
        int iw = w;
        switch(optype) {
          case UAOp::sqrOp:  vDSP_vsq(s, 1, d, 1, n); break;
          case UAOp::sqrtOp: vvsqrtf(d, s, &iw); break;
          case UAOp::lnOp:   vvlogf(d, s, &iw); break;
          case UAOp::expOp:  vvexpf(d, s, &iw); break;
          case UAOp::absOp:  vDSP_vabs(s, 1, d, 1, n); break;
        }
      });
  }

  // ================================================================
  // Registration
  // ================================================================

  static int _reg = [] {
    using Op = UAOp::Op;
    auto acc = UAOp::prototype().backends(Backend::Accelerate);
    acc.add<UAOp::ArithValSig>(Op::withVal, acc_arith_with_val,
      applicableTo<icl32f>, "Accelerate vDSP arithmetic with-val (32f)");
    acc.add<UAOp::ArithNoValSig>(Op::noVal, acc_arith_no_val,
      applicableTo<icl32f>, "Accelerate vDSP/vForce arithmetic no-val (32f)");
    return 0;
  }();

} // anonymous namespace
