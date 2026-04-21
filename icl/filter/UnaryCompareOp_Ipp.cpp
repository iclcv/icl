// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/ImageBackendDispatching.h>
#include <icl/core/Image.h>
#include <icl/core/Visitors.h>
#include <icl/filter/UnaryCompareOp.h>
#include <ipp.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using CmpOp = icl::filter::UnaryCompareOp;

  // Map our optype enum to IPP's IppCmpOp
  static IppCmpOp toIppCmpOp(int ot) {
    switch(ot) {
      case CmpOp::lt:   return ippCmpLess;
      case CmpOp::lteq: return ippCmpLessEq;
      case CmpOp::eq:   return ippCmpEq;
      case CmpOp::gteq: return ippCmpGreaterEq;
      case CmpOp::gt:   return ippCmpGreater;
      default:           return ippCmpLess;
    }
  }

  template<class T, IppStatus (IPP_DECL *ippiFunc)(const T*, int, T, icl8u*, int, IppiSize, IppCmpOp)>
  void ippCompare(const Img<T> &src, Img8u &dst, T value, IppCmpOp op) {
    for(int c = src.getChannels()-1; c >= 0; --c) {
      ippiFunc(src.getROIData(c), src.getLineStep(), value,
               dst.getROIData(c), dst.getLineStep(),
               dst.getROISize(), op);
    }
  }


  void ipp_compare(const Image &src, Image &dst, double value, int optype) {
    Img8u &d = dst.as8u();
    IppCmpOp op = toIppCmpOp(optype);
    src.visit([&](const auto &s) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u>)
        ippCompare<icl8u, ippiCompareC_8u_C1R>(s, d, static_cast<icl8u>(value), op);
      else if constexpr (std::is_same_v<T, icl16s>)
        ippCompare<icl16s, ippiCompareC_16s_C1R>(s, d, static_cast<icl16s>(value), op);
      else if constexpr (std::is_same_v<T, icl32f>)
        ippCompare<icl32f, ippiCompareC_32f_C1R>(s, d, static_cast<icl32f>(value), op);
    });
  }


  void ipp_compare_eqt(const Image &src, Image &dst, double value, double tolerance) {
    Img8u &d = dst.as8u();
    const Img32f &s = src.as32f();
    for(int c = s.getChannels()-1; c >= 0; --c) {
      ippiCompareEqualEpsC_32f_C1R(s.getROIData(c), s.getLineStep(),
                                    static_cast<icl32f>(value),
                                    d.getROIData(c), d.getLineStep(),
                                    d.getROISize(),
                                    static_cast<icl32f>(tolerance));
    }
  }

  // --- Direct registration into prototype ---
  using Op = CmpOp::Op;

  static int _reg = [] {
    auto ipp = CmpOp::prototype().backends(Backend::Ipp);
    ipp.add<CmpOp::CmpSig>(Op::compare, ipp_compare, applicableTo<icl8u, icl16s, icl32f>, "IPP compare (8u/16s/32f)");
    ipp.add<CmpOp::CmpEqtSig>(Op::compareEqTol, ipp_compare_eqt, applicableTo<icl32f>, "IPP compareEqualEps (32f)");
    return 0;
  }();

} // anon namespace
