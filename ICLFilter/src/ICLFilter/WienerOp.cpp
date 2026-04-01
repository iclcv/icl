// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#include <ICLFilter/WienerOp.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    const char* toString(WienerOp::Op op) {
      switch(op) {
        case WienerOp::Op::apply: return "apply";
      }
      return "?";
    }

    core::ImageBackendDispatching& WienerOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.addSelector<WienerSig>(Op::apply);
        return true;
      }();
      (void)init;
      return proto;
    }

    WienerOp::WienerOp(const Size &maskSize, icl32f noise)
      : NeighborhoodOp(maskSize), ImageBackendDispatching(prototype()),
        m_fNoise(noise)
    {}

    void WienerOp::apply(const Image &src, Image &dst) {
      if(!prepare(dst, src)) return;
      getSelector<WienerSig>(Op::apply).resolve(src)->apply(
        src, dst, getMaskSize(), getAnchor(), getROIOffset(), m_fNoise);
    }

  } // namespace filter
}
