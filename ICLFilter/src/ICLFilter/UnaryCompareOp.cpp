/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryCompareOp.cpp          **
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

#include <ICLFilter/UnaryCompareOp.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLUtils/EnumDispatch.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

    // ================================================================
    // C++ fallback implementations
    // ================================================================

    namespace {
      using UCO = UnaryCompareOp;

      template<UCO::optype OT, class T>
      void cmpTyped(const Img<T> &src, Img8u &dst, T value) {
        visitROILinesPerChannelWith(src, dst, [value](const T *s, icl8u *d, int, int w) {
          for(int i = 0; i < w; ++i) {
            if constexpr (OT == UCO::lt)   d[i] = s[i] < value ? 255 : 0;
            else if constexpr (OT == UCO::lteq) d[i] = s[i] <= value ? 255 : 0;
            else if constexpr (OT == UCO::eq)   d[i] = s[i] == value ? 255 : 0;
            else if constexpr (OT == UCO::gteq) d[i] = s[i] >= value ? 255 : 0;
            else if constexpr (OT == UCO::gt)   d[i] = s[i] > value ? 255 : 0;
          }
        });
      }

      template<UCO::optype OT>
      void compare_typed(const Image &src, Img8u &d, double value) {
        src.visit([&](const auto &s) {
          using T = typename std::remove_reference_t<decltype(s)>::type;
          cmpTyped<OT>(s, d, clipped_cast<double,T>(value));
        });
      }

      void cpp_compare(const Image &src, Image &dst, double value, int optype) {
        Img8u &d = dst.as8u();
        dispatchEnum<UCO::lt, UCO::lteq, UCO::eq, UCO::gteq, UCO::gt>(optype, [&](auto tag) {
          compare_typed<decltype(tag)::value>(src, d, value);
        });
      }

      void cpp_compare_eqt(const Image &src, Image &dst, double value, double tolerance) {
        Img8u &d = dst.as8u();
        src.visit([&](const auto &s) {
          using T = typename std::remove_reference_t<decltype(s)>::type;
          T v = clipped_cast<double,T>(value);
          T t = clipped_cast<double,T>(tolerance);
          visitROILinesPerChannelWith(s, d, [v, t](const T *sp, icl8u *dp, int, int w) {
            for(int i = 0; i < w; ++i)
              dp[i] = std::abs(sp[i] - v) <= t ? 255 : 0;
          });
        });
      }
    } // anon namespace

    // ================================================================
    // Constructor
    // ================================================================

    UnaryCompareOp::UnaryCompareOp(optype ot, icl64f value, icl64f tolerance)
      : m_eOpType(ot), m_dValue(value), m_dTolerance(tolerance)
    {
      initDispatching("UnaryCompareOp");

      auto& cmp    = addSelector<CmpSig>("compare");
      auto& cmpEqt = addSelector<CmpEqtSig>("compareEqTol");


      cmp.add(Backend::Cpp, cpp_compare);
      cmpEqt.add(Backend::Cpp, cpp_compare_eqt);
    }

    UnaryCompareOp::UnaryCompareOp(const std::string &op, icl64f value, icl64f tolerance)
      : UnaryCompareOp(translate_op_type(op), value, tolerance) {}

    // ================================================================
    // apply()
    // ================================================================

    void UnaryCompareOp::apply(const Image &src, Image &dst) {
      if(!prepare(dst, src, depth8u)) return;

      if(m_eOpType == eqt) {
        auto& sel = getSelector<CmpEqtSig>("compareEqTol");
        sel.resolve(src)->apply(src, dst, m_dValue, m_dTolerance);
      } else {
        auto& sel = getSelector<CmpSig>("compare");
        sel.resolve(src)->apply(src, dst, m_dValue, static_cast<int>(m_eOpType));
      }
    }

  } // namespace filter
} // namespace icl
