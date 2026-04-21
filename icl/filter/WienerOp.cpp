// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#include <icl/filter/WienerOp.h>
#include <icl/core/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(WienerOp::Op op) {
    switch(op) {
      case WienerOp::Op::apply: return "apply";
    }
    return "?";
  }

  core::ImageBackendDispatching& WienerOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<WienerSig>(Op::apply);
      return true;
    }();
    return proto;
  }

  WienerOp::WienerOp(const Size &maskSize, icl32f noise)
    : NeighborhoodOp(maskSize), ImageBackendDispatching(prototype()),
      m_fNoise(noise)
  {
    // Default range covers the useful span for 8u-gray inputs: 0 is a no-op
    // (textbook Wiener with "no noise expected" returns src unchanged), ~100
    // is moderate smoothing (σ≈10 gray levels), ~1000 is aggressive.
    addProperty("noise","range:slider","[0,1000]:0.1",str(noise));
    addProperty("mask size.w","range:spinbox","[1,51]",str(maskSize.width));
    addProperty("mask size.h","range:spinbox","[1,51]",str(maskSize.height));
    registerCallback([this](const Property &p){
      // Callback side is auto-locked by UnaryOp::registerCallback.
      if(p.name == "noise") m_fNoise = parse<icl32f>(p.value);
      else if(p.name == "mask size.w" || p.name == "mask size.h"){
        setMask(Size(parse<int>(prop("mask size.w").value),
                     parse<int>(prop("mask size.h").value)));
      }
    });
  }

  void WienerOp::setNoise(icl32f noise){ setPropertyValue("noise", noise); }

  void WienerOp::apply(const Image &src, Image &dst) {
    // Reader-side lock covers prepare() + getMaskSize/Anchor/ROIOffset +
    // backend dispatch as a single critical section against mid-apply
    // property callbacks.
    std::scoped_lock lock(m_applyMutex);
    if(!prepare(dst, src)) return;
    getSelector<WienerSig>(Op::apply).resolve(src)->apply(
      src, dst, getMaskSize(), getAnchor(), getROIOffset(), m_fNoise);
  }

  REGISTER_CONFIGURABLE(WienerOp, return new WienerOp(utils::Size(3,3)));
  } // namespace icl::filter