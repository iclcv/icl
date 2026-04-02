// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/WeightChannelsOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
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

  } // namespace icl::filter