// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ipp.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using UAOp = filter::UnaryArithmeticalOp;
  using Op = UAOp::Op;

  void ipp_arith_with_val(const Image &src, Image &dst, double value, int optype) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;

      if constexpr (std::is_same_v<T, icl8u>) {
        icl8u v = clipped_cast<double, icl8u>(value);
        for(int c = s.getChannels()-1; c >= 0; --c) {
          const icl8u *sr = s.getROIData(c); int ss = s.getLineStep();
          icl8u *dr = d.getROIData(c); int ds = d.getLineStep();
          auto sz = d.getROISize();
          switch(optype) {
            case UAOp::addOp: ippiAddC_8u_C1RSfs(sr,ss,v,dr,ds,sz,0); break;
            case UAOp::subOp: ippiSubC_8u_C1RSfs(sr,ss,v,dr,ds,sz,0); break;
            case UAOp::mulOp: ippiMulC_8u_C1RSfs(sr,ss,v,dr,ds,sz,0); break;
            case UAOp::divOp: ippiDivC_8u_C1RSfs(sr,ss,v,dr,ds,sz,0); break;
          }
        }
      } else if constexpr (std::is_same_v<T, icl16s>) {
        icl16s v = clipped_cast<double, icl16s>(value);
        for(int c = s.getChannels()-1; c >= 0; --c) {
          const icl16s *sr = s.getROIData(c); int ss = s.getLineStep();
          icl16s *dr = d.getROIData(c); int ds = d.getLineStep();
          auto sz = d.getROISize();
          switch(optype) {
            case UAOp::addOp: ippiAddC_16s_C1RSfs(sr,ss,v,dr,ds,sz,0); break;
            case UAOp::subOp: ippiSubC_16s_C1RSfs(sr,ss,v,dr,ds,sz,0); break;
            case UAOp::mulOp: ippiMulC_16s_C1RSfs(sr,ss,v,dr,ds,sz,0); break;
            case UAOp::divOp: ippiDivC_16s_C1RSfs(sr,ss,v,dr,ds,sz,0); break;
          }
        }
      } else if constexpr (std::is_same_v<T, icl32f>) {
        icl32f v = static_cast<icl32f>(value);
        for(int c = s.getChannels()-1; c >= 0; --c) {
          const icl32f *sr = s.getROIData(c); int ss = s.getLineStep();
          icl32f *dr = d.getROIData(c); int ds = d.getLineStep();
          auto sz = d.getROISize();
          switch(optype) {
            case UAOp::addOp: ippiAddC_32f_C1R(sr,ss,v,dr,ds,sz); break;
            case UAOp::subOp: ippiSubC_32f_C1R(sr,ss,v,dr,ds,sz); break;
            case UAOp::mulOp: ippiMulC_32f_C1R(sr,ss,v,dr,ds,sz); break;
            case UAOp::divOp: ippiDivC_32f_C1R(sr,ss,v,dr,ds,sz); break;
          }
        }
      }
    });
  }

  void ipp_arith_no_val(const Image &src, Image &dst, int optype) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;

      if constexpr (std::is_same_v<T, icl8u>) {
        for(int c = s.getChannels()-1; c >= 0; --c) {
          const icl8u *sr = s.getROIData(c); int ss = s.getLineStep();
          icl8u *dr = d.getROIData(c); int ds = d.getLineStep();
          auto sz = d.getROISize();
          switch(optype) {
            case UAOp::sqrOp:  ippiSqr_8u_C1RSfs(sr,ss,dr,ds,sz,0); break;
            case UAOp::sqrtOp: ippiSqrt_8u_C1RSfs(sr,ss,dr,ds,sz,0); break;
            case UAOp::lnOp:   ippiLn_8u_C1RSfs(sr,ss,dr,ds,sz,0); break;
            case UAOp::expOp:  ippiExp_8u_C1RSfs(sr,ss,dr,ds,sz,0); break;
            case UAOp::absOp:  break; // 8u is always non-negative
          }
        }
      } else if constexpr (std::is_same_v<T, icl16s>) {
        for(int c = s.getChannels()-1; c >= 0; --c) {
          const icl16s *sr = s.getROIData(c); int ss = s.getLineStep();
          icl16s *dr = d.getROIData(c); int ds = d.getLineStep();
          auto sz = d.getROISize();
          switch(optype) {
            case UAOp::sqrOp:  ippiSqr_16s_C1RSfs(sr,ss,dr,ds,sz,0); break;
            case UAOp::sqrtOp: ippiSqrt_16s_C1RSfs(sr,ss,dr,ds,sz,0); break;
            case UAOp::lnOp:   ippiLn_16s_C1RSfs(sr,ss,dr,ds,sz,0); break;
            case UAOp::expOp:  ippiExp_16s_C1RSfs(sr,ss,dr,ds,sz,0); break;
            case UAOp::absOp:  ippiAbs_16s_C1R(sr,ss,dr,ds,sz); break;
          }
        }
      } else if constexpr (std::is_same_v<T, icl32f>) {
        for(int c = s.getChannels()-1; c >= 0; --c) {
          const icl32f *sr = s.getROIData(c); int ss = s.getLineStep();
          icl32f *dr = d.getROIData(c); int ds = d.getLineStep();
          auto sz = d.getROISize();
          switch(optype) {
            case UAOp::sqrOp:  ippiSqr_32f_C1R(sr,ss,dr,ds,sz); break;
            case UAOp::sqrtOp: ippiSqrt_32f_C1R(sr,ss,dr,ds,sz); break;
            case UAOp::lnOp:   ippiLn_32f_C1R(sr,ss,dr,ds,sz); break;
            case UAOp::expOp:  ippiExp_32f_C1R(sr,ss,dr,ds,sz); break;
            case UAOp::absOp:  ippiAbs_32f_C1R(sr,ss,dr,ds,sz); break;
          }
        }
      }
    });
  }

  static int _reg = [] {
    auto ipp = UAOp::prototype().backends(Backend::Ipp);
    ipp.add<UAOp::ArithValSig>(Op::withVal, ipp_arith_with_val,
      applicableTo<icl8u, icl16s, icl32f>, "IPP addC/subC/mulC/divC (8u/16s/32f)");
    ipp.add<UAOp::ArithNoValSig>(Op::noVal, ipp_arith_no_val,
      applicableTo<icl8u, icl16s, icl32f>, "IPP sqr/sqrt/ln/exp/abs (8u/16s/32f)");
    return 0;
  }();

} // anon namespace
