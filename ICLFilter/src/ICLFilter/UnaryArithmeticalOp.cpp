/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryArithmeticalOp.cpp     **
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

#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter {

    // ================================================================
    // C++ fallback implementations
    // ================================================================

    namespace {
      using Op = UnaryArithmeticalOp;

      template<class T>
      inline T absVal(T t) { return std::abs(t); }
      template<> inline icl8u absVal(icl8u t) { return t; }
      template<> inline icl32f absVal(icl32f t) { return std::fabs(t); }
      template<> inline icl64f absVal(icl64f t) { return std::fabs(t); }

      template<class T>
      void arithWithVal(const Img<T> &src, Img<T> &dst, T val, int optype) {
        visitROILinesPerChannelWith(src, dst, [val, optype](const T *s, T *d, int, int w) {
          for(int i = 0; i < w; ++i) {
            switch(optype) {
              case Op::addOp: d[i] = clipped_cast<icl64f,T>(static_cast<icl64f>(s[i]) + static_cast<icl64f>(val)); break;
              case Op::subOp: d[i] = clipped_cast<icl64f,T>(static_cast<icl64f>(s[i]) - static_cast<icl64f>(val)); break;
              case Op::mulOp: d[i] = clipped_cast<icl64f,T>(static_cast<icl64f>(s[i]) * static_cast<icl64f>(val)); break;
              case Op::divOp: d[i] = clipped_cast<icl64f,T>(static_cast<icl64f>(s[i]) / static_cast<icl64f>(val)); break;
              default: break;
            }
          }
        });
      }

      template<class T>
      void arithNoVal(const Img<T> &src, Img<T> &dst, int optype) {
        visitROILinesPerChannelWith(src, dst, [optype](const T *s, T *d, int, int w) {
          for(int i = 0; i < w; ++i) {
            switch(optype) {
              case Op::sqrOp:  d[i] = s[i] * s[i]; break;
              case Op::sqrtOp: d[i] = clipped_cast<double,T>(std::sqrt(static_cast<double>(s[i]))); break;
              case Op::lnOp:   d[i] = clipped_cast<double,T>(std::log(static_cast<double>(s[i]))); break;
              case Op::expOp:  d[i] = clipped_cast<double,T>(std::exp(static_cast<double>(s[i]))); break;
              case Op::absOp:  d[i] = absVal(s[i]); break;
              default: break;
            }
          }
        });
      }

      void cpp_arith_with_val(const Image &src, Image &dst, double value, int optype) {
        src.visitWith(dst, [&](const auto &s, auto &d) {
          arithWithVal(s, d, clipped_cast<icl64f, typename std::remove_reference_t<decltype(s)>::type>(value), optype);
        });
      }

      void cpp_arith_no_val(const Image &src, Image &dst, int optype) {
        src.visitWith(dst, [&](const auto &s, auto &d) {
          arithNoVal(s, d, optype);
        });
      }
    } // anon namespace

    // ================================================================
    // Constructor
    // ================================================================

    UnaryArithmeticalOp::UnaryArithmeticalOp(optype t, icl64f val)
      : m_eOpType(t), m_dValue(val)
    {
      initDispatching("UnaryArithmeticalOp");

      auto& withVal = addSelector<ArithValSig>("withVal");
      auto& noVal   = addSelector<ArithNoValSig>("noVal");

      withVal.add(Backend::Cpp, cpp_arith_with_val);
      noVal.add(Backend::Cpp, cpp_arith_no_val);
    }

    // ================================================================
    // apply()
    // ================================================================

    void UnaryArithmeticalOp::apply(const Image &src, Image &dst) {
      if(!prepare(dst, src)) return;

      switch(m_eOpType) {
        case addOp: case subOp: case mulOp: case divOp: {
          auto& sel = getSelector<ArithValSig>("withVal");
          sel.resolve(src)->apply(src, dst, m_dValue, static_cast<int>(m_eOpType));
          break;
        }
        default: {
          auto& sel = getSelector<ArithNoValSig>("noVal");
          sel.resolve(src)->apply(src, dst, static_cast<int>(m_eOpType));
          break;
        }
      }
    }

  } // namespace filter
} // namespace icl
