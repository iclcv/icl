// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <Accelerate/Accelerate.h>

#include <icl/filter/BinaryArithmeticalOp.h>
#include <icl/core/Visitors.h>

namespace {

  using namespace icl::core;
  using icl::icl32f;
  using BOp = icl::filter::BinaryArithmeticalOp;

  void acc_binary_arith(const Image &src1, const Image &src2, Image &dst, int optype) {
    // vDSP note: vsub(B,stride,A,stride,C,stride,N) → C=A-B
    //            vdiv(B,stride,A,stride,C,stride,N) → C=A/B
    visitROILinesPerChannel2With(src1.as32f(), src2.as32f(), dst.as32f(),
      [optype](const icl32f *a, const icl32f *b, icl32f *d, int, int w) {
        vImagePixelCount n = static_cast<vImagePixelCount>(w);
        switch(optype) {
          case BOp::addOp:    vDSP_vadd(a, 1, b, 1, d, 1, n); break;
          case BOp::subOp:    vDSP_vsub(b, 1, a, 1, d, 1, n); break;
          case BOp::mulOp:    vDSP_vmul(a, 1, b, 1, d, 1, n); break;
          case BOp::divOp:    vDSP_vdiv(b, 1, a, 1, d, 1, n); break;
          case BOp::absSubOp:
            vDSP_vsub(b, 1, a, 1, d, 1, n);
            vDSP_vabs(d, 1, d, 1, n);
            break;
        }
      });
  }

  static int _reg = [] {
    auto acc = BOp::prototype().backends(Backend::Accelerate);
    acc.add<BOp::Sig>(BOp::Op::apply, acc_binary_arith,
      applicableTo<icl32f>,
      "Accelerate vDSP binary arithmetic (32f)");
    return 0;
  }();

} // anonymous namespace
