// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Tobias Roehlig, Christof Elbrechter

#include <icl/filter/BilateralFilterOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
const char* toString(BilateralFilterOp::Op op) {
  switch(op) {
    case BilateralFilterOp::Op::apply: return "apply";
  }
  return "?";
}

core::ImageBackendDispatching& BilateralFilterOp::prototype() {
  static core::ImageBackendDispatching proto;
  [[maybe_unused]] static bool init = [&] {
    proto.addSelector<ApplySig>(Op::apply);
    return true;
  }();
  return proto;
}

BilateralFilterOp::BilateralFilterOp(int radius, float sigma_s, float sigma_r, bool use_lab)
  : ImageBackendDispatching(prototype()),
    m_radius(radius), m_sigmaS(sigma_s), m_sigmaR(sigma_r), m_useLAB(use_lab)
{
  addProperty("radius","range:spinbox","[1,24]",str(radius));
  addProperty("sigma_s","range:slider","[0.1,200]:0.1",str(sigma_s));
  addProperty("sigma_r","range:slider","[0.1,200]:0.1",str(sigma_r));
  addProperty("use LAB","flag","",use_lab);
  registerCallback([this](const Property &p){
    if(p.name == "radius")        m_radius = parse<int>(p.value);
    else if(p.name == "sigma_s")  m_sigmaS = parse<float>(p.value);
    else if(p.name == "sigma_r")  m_sigmaR = parse<float>(p.value);
    else if(p.name == "use LAB")  m_useLAB = parse<bool>(p.value);
  });
}

void BilateralFilterOp::setRadius(int r){ setPropertyValue("radius", r); }
void BilateralFilterOp::setSigmaS(float s){ setPropertyValue("sigma_s", s); }
void BilateralFilterOp::setSigmaR(float r){ setPropertyValue("sigma_r", r); }
void BilateralFilterOp::setUseLAB(bool b){ setPropertyValue("use LAB", b); }

void BilateralFilterOp::apply(const Image &src, Image &dst) {
  if(!prepare(dst, src)) return;

  auto* impl = getSelector<ApplySig>(Op::apply).resolve(src);
  if(!impl) {
    ERROR_LOG("no applicable backend for BilateralFilterOp");
    return;
  }
  impl->apply(src, dst, m_radius, m_sigmaS, m_sigmaR, m_useLAB);
}

REGISTER_CONFIGURABLE_DEFAULT(BilateralFilterOp);

} // namespace icl::filter