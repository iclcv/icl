// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Tobias Roehlig, Christof Elbrechter

#include <icl/filter/BilateralFilterOp.h>
#include <icl/utils/prop/Constraints.h>

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
  addProperty("radius",utils::prop::Range{.min=1, .max=24, .ui=utils::prop::UI::Spinbox}, radius);
  addProperty("sigma_s",utils::prop::Range{.min=0.1f, .max=200.f, .step=0.1f}, sigma_s);
  addProperty("sigma_r",utils::prop::Range{.min=0.1f, .max=200.f, .step=0.1f}, sigma_r);
  addProperty("use LAB",utils::prop::Flag{}, use_lab);
  registerCallback([this](const Property &p){
    if(p.name == "radius")        m_radius = p.as<int>();
    else if(p.name == "sigma_s")  m_sigmaS = p.as<float>();
    else if(p.name == "sigma_r")  m_sigmaR = p.as<float>();
    else if(p.name == "use LAB")  m_useLAB = p.as<bool>();
  });
}

void BilateralFilterOp::setRadius(int r){ prop("radius").value = r; }
void BilateralFilterOp::setSigmaS(float s){ prop("sigma_s").value = s; }
void BilateralFilterOp::setSigmaR(float r){ prop("sigma_r").value = r; }
void BilateralFilterOp::setUseLAB(bool b){ prop("use LAB").value = b; }

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