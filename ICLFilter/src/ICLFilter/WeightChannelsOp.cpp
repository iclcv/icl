/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/WeightChannelsOp.cpp           **
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

#include <ICLFilter/WeightChannelsOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    void WeightChannelsOp::apply(const Image &src, Image &dst) {
      ICLASSERT_RETURN( static_cast<int>(m_vecWeights.size()) == src.getChannels() );
      if(!prepare(dst, src)) return;
      src.visitWith(dst, [this](const auto &s, auto &d) {
        using T = typename std::remove_reference_t<decltype(s)>::type;
        visitROILinesPerChannelWith(s, d, [this](const T *sp, T *dp, int ch, int w) {
          icl64f wt = m_vecWeights[ch];
          for(int i = 0; i < w; ++i) {
            dp[i] = clipped_cast<icl64f, T>(static_cast<icl64f>(sp[i]) * wt);
          }
        });
      });
    }

  } // namespace filter
} // namespace icl
