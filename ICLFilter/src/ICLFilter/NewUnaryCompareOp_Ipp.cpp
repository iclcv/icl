/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/NewUnaryCompareOp_Ipp.cpp      **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLCore/BackendDispatch.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLFilter/NewUnaryCompareOp.h>

#ifdef ICL_HAVE_IPP
#include <ipp.h>

using namespace icl::utils;
using namespace icl::core;

namespace {

  using CmpOp = icl::filter::NewUnaryCompareOp;

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

  bool supports_8u_16s_32f(const Image& src) {
    return src.getDepth() == depth8u || src.getDepth() == depth16s || src.getDepth() == depth32f;
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

  bool supports_32f(const Image& src) {
    return src.getDepth() == depth32f;
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

  // --- Self-registration ---
  static const int _reg1 = registerBackend<CmpOp::CmpSig>(
    "NewUnaryCompareOp.compare", Backend::Ipp, ipp_compare,
    supports_8u_16s_32f, "IPP compare (8u/16s/32f)");

  static const int _reg2 = registerBackend<CmpOp::CmpEqtSig>(
    "NewUnaryCompareOp.compareEqTol", Backend::Ipp, ipp_compare_eqt,
    supports_32f, "IPP compareEqualEps (32f)");

} // anon namespace

#endif // ICL_HAVE_IPP
