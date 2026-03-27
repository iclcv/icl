/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/NewThresholdOp.cpp             **
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

#include <ICLFilter/NewThresholdOp.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

    // ================================================================
    // C++ fallback implementations
    // ================================================================

    namespace {
      void cpp_ltval(const Image &src, Image &dst, double threshold, double value) {
        src.visitWith(dst, [&](const auto &s, auto &d) {
          using T = typename std::remove_reference_t<decltype(s)>::type;
          T t = clipped_cast<double,T>(threshold);
          T v = clipped_cast<double,T>(value);
          visitROILinesPerChannelWith(s, d, [t, v](const T *sp, T *dp, int, int w) {
            for(int i = 0; i < w; ++i) dp[i] = sp[i] < t ? v : sp[i];
          });
        });
      }

      void cpp_gtval(const Image &src, Image &dst, double threshold, double value) {
        src.visitWith(dst, [&](const auto &s, auto &d) {
          using T = typename std::remove_reference_t<decltype(s)>::type;
          T t = clipped_cast<double,T>(threshold);
          T v = clipped_cast<double,T>(value);
          visitROILinesPerChannelWith(s, d, [t, v](const T *sp, T *dp, int, int w) {
            for(int i = 0; i < w; ++i) dp[i] = sp[i] > t ? v : sp[i];
          });
        });
      }

      void cpp_ltgtval(const Image &src, Image &dst,
                       double tLo, double vLo, double tHi, double vHi) {
        src.visitWith(dst, [&](const auto &s, auto &d) {
          using T = typename std::remove_reference_t<decltype(s)>::type;
          T lo = clipped_cast<double,T>(tLo);
          T vl = clipped_cast<double,T>(vLo);
          T hi = clipped_cast<double,T>(tHi);
          T vh = clipped_cast<double,T>(vHi);
          visitROILinesPerChannelWith(s, d, [lo, vl, hi, vh](const T *sp, T *dp, int, int w) {
            for(int i = 0; i < w; ++i) {
              T val = sp[i];
              dp[i] = val < lo ? vl : (val > hi ? vh : val);
            }
          });
        });
      }
    } // anon namespace

    // ================================================================
    // Constructor — registers C++ fallback + loads from global registry
    // ================================================================

    // Sub-op signatures (internal to this TU)
    using ThreshSig = void(const Image&, Image&, double, double);
    using ThreshDualSig = void(const Image&, Image&, double, double, double, double);

    NewThresholdOp::NewThresholdOp(optype ttype, float lowThreshold,
                                   float highThreshold, float lowVal, float highVal)
      : m_eType(ttype), m_fLowThreshold(lowThreshold),
        m_fHighThreshold(highThreshold), m_fLowVal(lowVal), m_fHighVal(highVal)
    {
      initDispatching("NewThresholdOp");

      // Create switches (auto-loads self-registered backends from registry)
      auto& ltVal   = addSelector<ThreshSig>("ltVal");
      auto& gtVal   = addSelector<ThreshSig>("gtVal");
      auto& ltgtVal = addSelector<ThreshDualSig>("ltgtVal");

      // C++ fallback — always present, supports all depths
      ltVal.add(Backend::Cpp, cpp_ltval);
      gtVal.add(Backend::Cpp, cpp_gtval);
      ltgtVal.add(Backend::Cpp, cpp_ltgtval);
    }

    // ================================================================
    // apply() — dispatches to best backend per sub-op
    // ================================================================

    void NewThresholdOp::apply(const Image &src, Image &dst) {
      if(!prepare(dst, src)) return;

      auto& swLT   = getSelector<ThreshSig>("ltVal");
      auto& swGT   = getSelector<ThreshSig>("gtVal");
      auto& swLTGT = getSelector<ThreshDualSig>("ltgtVal");

      switch(m_eType) {
        case lt:
          swLT.resolve(src)->apply(src, dst, m_fLowThreshold, m_fLowThreshold);
          break;
        case gt:
          swGT.resolve(src)->apply(src, dst, m_fHighThreshold, m_fHighThreshold);
          break;
        case ltgt:
          swLTGT.resolve(src)->apply(src, dst,
                        m_fLowThreshold, m_fLowThreshold,
                        m_fHighThreshold, m_fHighThreshold);
          break;
        case ltVal:
          swLT.resolve(src)->apply(src, dst, m_fLowThreshold, m_fLowVal);
          break;
        case gtVal:
          swGT.resolve(src)->apply(src, dst, m_fHighThreshold, m_fHighVal);
          break;
        case ltgtVal:
          swLTGT.resolve(src)->apply(src, dst,
                        m_fLowThreshold, m_fLowVal,
                        m_fHighThreshold, m_fHighVal);
          break;
      }
    }

  } // namespace filter
} // namespace icl
