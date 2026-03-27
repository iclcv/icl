/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/NewThresholdOp_Ipp.cpp         **
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

#include <ICLFilter/FilterDispatch.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>

#ifdef ICL_HAVE_IPP
#include <ipp.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::filter;

namespace {

  bool supports_8u_16s_32f(const Image& src) {
    return src.getDepth() == depth8u || src.getDepth() == depth16s || src.getDepth() == depth32f;
  }

  // IPP helper: call a 2-param threshold function per channel
  template<typename T, IppStatus (IPP_DECL *ippiFunc)(const T*, int, T*, int, IppiSize, T, T)>
  void ippiThresh2(const Img<T> &src, Img<T> &dst, T t, T v) {
    for(int c = src.getChannels()-1; c >= 0; --c) {
      ippiFunc(src.getROIData(c), src.getLineStep(),
               dst.getROIData(c), dst.getLineStep(),
               dst.getROISize(), t, v);
    }
  }

  // IPP helper: call a 4-param threshold function per channel
  template<typename T, IppStatus (IPP_DECL *ippiFunc)(const T*, int, T*, int, IppiSize, T, T, T, T)>
  void ippiThresh4(const Img<T> &src, Img<T> &dst, T t1, T v1, T t2, T v2) {
    for(int c = src.getChannels()-1; c >= 0; --c) {
      ippiFunc(src.getROIData(c), src.getLineStep(),
               dst.getROIData(c), dst.getLineStep(),
               dst.getROISize(), t1, v1, t2, v2);
    }
  }

  void ipp_ltval(const Image &src, Image &dst, double threshold, double value) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u>)
        ippiThresh2<icl8u, ippiThreshold_LTVal_8u_C1R>(s, d,
          static_cast<icl8u>(threshold), static_cast<icl8u>(value));
      else if constexpr (std::is_same_v<T, icl16s>)
        ippiThresh2<icl16s, ippiThreshold_LTVal_16s_C1R>(s, d,
          static_cast<icl16s>(threshold), static_cast<icl16s>(value));
      else if constexpr (std::is_same_v<T, icl32f>)
        ippiThresh2<icl32f, ippiThreshold_LTVal_32f_C1R>(s, d,
          static_cast<icl32f>(threshold), static_cast<icl32f>(value));
    });
  }

  void ipp_gtval(const Image &src, Image &dst, double threshold, double value) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u>)
        ippiThresh2<icl8u, ippiThreshold_GTVal_8u_C1R>(s, d,
          static_cast<icl8u>(threshold), static_cast<icl8u>(value));
      else if constexpr (std::is_same_v<T, icl16s>)
        ippiThresh2<icl16s, ippiThreshold_GTVal_16s_C1R>(s, d,
          static_cast<icl16s>(threshold), static_cast<icl16s>(value));
      else if constexpr (std::is_same_v<T, icl32f>)
        ippiThresh2<icl32f, ippiThreshold_GTVal_32f_C1R>(s, d,
          static_cast<icl32f>(threshold), static_cast<icl32f>(value));
    });
  }

  void ipp_ltgtval(const Image &src, Image &dst,
                   double tLo, double vLo, double tHi, double vHi) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u>)
        ippiThresh4<icl8u, ippiThreshold_LTValGTVal_8u_C1R>(s, d,
          static_cast<icl8u>(tLo), static_cast<icl8u>(vLo),
          static_cast<icl8u>(tHi), static_cast<icl8u>(vHi));
      else if constexpr (std::is_same_v<T, icl16s>)
        ippiThresh4<icl16s, ippiThreshold_LTValGTVal_16s_C1R>(s, d,
          static_cast<icl16s>(tLo), static_cast<icl16s>(vLo),
          static_cast<icl16s>(tHi), static_cast<icl16s>(vHi));
      else if constexpr (std::is_same_v<T, icl32f>)
        ippiThresh4<icl32f, ippiThreshold_LTValGTVal_32f_C1R>(s, d,
          static_cast<icl32f>(tLo), static_cast<icl32f>(vLo),
          static_cast<icl32f>(tHi), static_cast<icl32f>(vHi));
    });
  }

  // --- Self-registration ---
  using ThreshSig = void(const Image&, Image&, double, double);
  using ThreshDualSig = void(const Image&, Image&, double, double, double, double);

  static const int _reg1 = registerBackend<ThreshSig>(
    "NewThresholdOp.ltVal", Backend::Ipp, ipp_ltval,
    supports_8u_16s_32f, "IPP threshold ltVal (8u/16s/32f)");

  static const int _reg2 = registerBackend<ThreshSig>(
    "NewThresholdOp.gtVal", Backend::Ipp, ipp_gtval,
    supports_8u_16s_32f, "IPP threshold gtVal (8u/16s/32f)");

  static const int _reg3 = registerBackend<ThreshDualSig>(
    "NewThresholdOp.ltgtVal", Backend::Ipp, ipp_ltgtval,
    supports_8u_16s_32f, "IPP threshold ltgtVal (8u/16s/32f)");

} // anon namespace

#endif // ICL_HAVE_IPP
