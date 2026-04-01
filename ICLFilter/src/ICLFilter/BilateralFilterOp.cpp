// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Tobias Roehlig, Christof Elbrechter

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
