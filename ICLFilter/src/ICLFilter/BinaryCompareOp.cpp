/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryCompareOp.cpp            **
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

#include <ICLFilter/BinaryCompareOp.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLUtils/EnumDispatch.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

    using CmpSig    = BinaryCompareOp::CmpSig;
    using CmpEqtSig = BinaryCompareOp::CmpEqtSig;

    namespace {
      using Op = BinaryCompareOp;

      template<Op::optype OT, class T>
      void cmpTyped(const Img<T> &s1, const Img<T> &s2, Img8u &dst) {
        visitROILinesPerChannel2With(s1, s2, dst,
          [](const T *a, const T *b, icl8u *d, int, int w) {
            for(int i = 0; i < w; ++i) {
              if constexpr (OT == Op::lt)   d[i] = a[i] < b[i] ? 255 : 0;
              else if constexpr (OT == Op::lteq) d[i] = a[i] <= b[i] ? 255 : 0;
              else if constexpr (OT == Op::eq)   d[i] = a[i] == b[i] ? 255 : 0;
              else if constexpr (OT == Op::gteq) d[i] = a[i] >= b[i] ? 255 : 0;
              else if constexpr (OT == Op::gt)   d[i] = a[i] > b[i] ? 255 : 0;
            }
          });
      }

      template<Op::optype OT>
      void compare_typed(const Image &s1, const Image &s2, Img8u &d) {
        s1.visit([&](const auto &a) {
          using T = typename std::remove_reference_t<decltype(a)>::type;
          cmpTyped<OT>(a, s2.as<T>(), d);
        });
      }

      void cpp_compare(const Image &s1, const Image &s2, Image &dst, int ot) {
        Img8u &d = dst.as8u();
        dispatchEnum<Op::lt, Op::lteq, Op::eq, Op::gteq, Op::gt>(ot, [&](auto tag) {
          compare_typed<decltype(tag)::value>(s1, s2, d);
        });
      }

      void cpp_compare_eqt(const Image &s1, const Image &s2, Image &dst, double tolerance) {
        Img8u &d = dst.as8u();
        s1.visit([&](const auto &a) {
          using T = typename std::remove_reference_t<decltype(a)>::type;
          T tol = clipped_cast<double,T>(tolerance);
          visitROILinesPerChannel2With(a, s2.as<T>(), d,
            [tol](const T *ap, const T *bp, icl8u *dp, int, int w) {
              for(int i = 0; i < w; ++i)
                dp[i] = std::abs(ap[i] - bp[i]) <= tol ? 255 : 0;
            });
        });
      }
    } // anon

    BinaryCompareOp::BinaryCompareOp(optype ot, icl64f tolerance)
      : m_eOpType(ot), m_dTolerance(tolerance)
    {
      initDispatching("BinaryCompareOp");
      auto& cmp    = addSelector<CmpSig>("compare");
      auto& cmpEqt = addSelector<CmpEqtSig>("compareEqTol");
      cmp.add(Backend::Cpp, cpp_compare);
      cmpEqt.add(Backend::Cpp, cpp_compare_eqt);
    }

    void BinaryCompareOp::apply(const Image &src1, const Image &src2, Image &dst) {
      if(!check(src1, src2)) return;
      if(!prepare(dst, src1, depth8u)) return;
      if(m_eOpType == eqt) {
        getSelector<CmpEqtSig>("compareEqTol").resolve(src1)->apply(src1, src2, dst, m_dTolerance);
      } else {
        getSelector<CmpSig>("compare").resolve(src1)->apply(src1, src2, dst, static_cast<int>(m_eOpType));
      }
    }

  } // namespace filter
} // namespace icl
