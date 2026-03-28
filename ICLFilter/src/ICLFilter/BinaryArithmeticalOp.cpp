/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryArithmeticalOp.cpp       **
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

#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/EnumDispatch.h>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

    using Sig = BinaryArithmeticalOp::Sig;

    namespace {
      using Op = BinaryArithmeticalOp;

      template<Op::optype OT, class T>
      void arithOp(const Img<T> &s1, const Img<T> &s2, Img<T> &dst) {
        visitROILinesPerChannel2With(s1, s2, dst, [](const T *a, const T *b, T *d, int, int w) {
          for(int i = 0; i < w; ++i) {
            if constexpr (OT == Op::addOp)    d[i] = a[i] + b[i];
            else if constexpr (OT == Op::subOp)    d[i] = a[i] - b[i];
            else if constexpr (OT == Op::mulOp)    d[i] = a[i] * b[i];
            else if constexpr (OT == Op::divOp)    d[i] = a[i] / b[i];
            else if constexpr (OT == Op::absSubOp) d[i] = std::abs(a[i] - b[i]);
          }
        });
      }

      template<Op::optype OT>
      void apply_typed(const Image &s1, const Image &s2, Image &dst) {
        s1.visitWith(dst, [&](const auto &a, auto &d) {
          using T = typename std::remove_reference_t<decltype(a)>::type;
          arithOp<OT>(a, s2.as<T>(), d);
        });
      }

      void cpp_apply(const Image &s1, const Image &s2, Image &dst, int ot) {
        dispatchEnum<Op::addOp, Op::subOp, Op::mulOp, Op::divOp, Op::absSubOp>(ot, [&](auto tag) {
          apply_typed<decltype(tag)::value>(s1, s2, dst);
        });
      }
    } // anon

    BinaryArithmeticalOp::BinaryArithmeticalOp(optype t) : m_eOpType(t) {
      initDispatching("BinaryArithmeticalOp");
      auto& sel = addSelector<Sig>("apply");
      sel.add(Backend::Cpp, cpp_apply);
    }

    void BinaryArithmeticalOp::apply(const Image &src1, const Image &src2, Image &dst) {
      if(!check(src1, src2)) return;
      if(!prepare(dst, src1)) return;
      getSelector<Sig>("apply").resolve(src1)->apply(src1, src2, dst, static_cast<int>(m_eOpType));
    }

  } // namespace filter
} // namespace icl
