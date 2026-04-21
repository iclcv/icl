// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/LUTOp.h>
#include <icl/core/Img.h>
#include <ipp.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using LOp = filter::LUTOp;

  void ipp_reduceBits(const Img8u &src, Img8u &dst, icl8u n) {
    for(int c = src.getChannels()-1; c >= 0; --c) {
      ippiReduceBits_8u_C1R(src.getROIData(c), src.getLineStep(),
                            dst.getROIData(c), dst.getLineStep(),
                            dst.getROISize(), 0, ippDitherNone,
                            static_cast<int>(n), nullptr);
    }
  }

  static int _reg = [] {
    using Op = LOp::Op;
    auto ipp = LOp::prototype().backends(Backend::Ipp);
    ipp.add<LOp::ReduceBitsSig>(Op::reduceBits, ipp_reduceBits, "IPP reduceBits (8u)");
    return 0;
  }();

} // anon namespace
