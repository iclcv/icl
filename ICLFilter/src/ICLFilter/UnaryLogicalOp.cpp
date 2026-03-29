/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryLogicalOp.cpp             **
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

#include <ICLFilter/UnaryLogicalOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <type_traits>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{
    namespace{

      // --- C++ fallback implementations ---

      void cpp_withval(const Image &src, Image &dst, icl32s val, int optype) {
        src.visitWith(dst, [val, optype](const auto &s, auto &d) {
          using T = typename std::remove_reference_t<decltype(s)>::type;
          if constexpr (std::is_integral_v<T>) {
            T v = clipped_cast<icl32s,T>(val);
            switch(optype) {
              case UnaryLogicalOp::andOp:
                visitROILinesPerChannelWith(s, d, [v](const T *sp, T *dp, int, int w) {
                  for(int i = 0; i < w; ++i) dp[i] = sp[i] & v;
                }); break;
              case UnaryLogicalOp::orOp:
                visitROILinesPerChannelWith(s, d, [v](const T *sp, T *dp, int, int w) {
                  for(int i = 0; i < w; ++i) dp[i] = sp[i] | v;
                }); break;
              case UnaryLogicalOp::xorOp:
                visitROILinesPerChannelWith(s, d, [v](const T *sp, T *dp, int, int w) {
                  for(int i = 0; i < w; ++i) dp[i] = sp[i] ^ v;
                }); break;
              default: break;
            }
          }
        });
      }

      void cpp_noval(const Image &src, Image &dst) {
        src.visitWith(dst, [](const auto &s, auto &d) {
          using T = typename std::remove_reference_t<decltype(s)>::type;
          if constexpr (std::is_integral_v<T>) {
            visitROILinesPerChannelWith(s, d, [](const T *sp, T *dp, int, int w) {
              for(int i = 0; i < w; ++i) dp[i] = ~sp[i];
            });
          }
        });
      }

    } // anonymous namespace

    UnaryLogicalOp::UnaryLogicalOp(optype t, icl32s val)
      : m_eOpType(t), m_dValue(val)
    {
      initDispatching("UnaryLogicalOp");

      auto& wv = addSelector<WithValSig>("withVal");
      auto& nv = addSelector<NoValSig>("noVal");

      wv.add(Backend::Cpp, cpp_withval);
      nv.add(Backend::Cpp, cpp_noval);
    }

    void UnaryLogicalOp::apply(const Image &src, Image &dst) {
      ICLASSERT_RETURN(src.getDepth() == depth8u || src.getDepth() == depth16s || src.getDepth() == depth32s);
      if(!prepare(dst, src)) return;

      if(m_eOpType == notOp) {
        getSelector<NoValSig>("noVal").resolve(src)->apply(src, dst);
      } else {
        getSelector<WithValSig>("withVal").resolve(src)->apply(src, dst, m_dValue, (int)m_eOpType);
      }
    }

  } // namespace filter
}
