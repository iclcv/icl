// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke, Andre Justus

#include <ICLFilter/WeightedSumOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
    template <class T, class D>
    void apply_ws(const Img<T> &src, Img<D> &dst, const std::vector<D> &weights) {
      const ImgIterator<T> itSrc = src.beginROI(0);
      const ImgIterator<T> itSrcEnd = src.endROI(0);
      ImgIterator<D> itDst = dst.beginROI(0);

      D w = weights[0];
      for(;itSrc != itSrcEnd; ++itSrc, ++itDst){
        *itDst = *itSrc * w;
      }

      for (int c=src.getChannels()-1; c > 0; c--) {
        w = weights[c];
        const ImgIterator<T> itSrc = src.beginROI(c);
        const ImgIterator<T> itSrcEnd = src.endROI(c);
        ImgIterator<D> itDst = dst.beginROI(0);
        for(;itSrc!=itSrcEnd; ++itSrc, ++itDst){
          *itDst += *itSrc * w;
        }
      }
    }

    void WeightedSumOp::apply(const Image &src, Image &dst) {
      ICLASSERT_RETURN( static_cast<int>(m_vecWeights.size()) == src.getChannels() );

      depth dstDepth = src.getDepth() == depth64f ? depth64f : depth32f;
      if(!prepare(dst, dstDepth,
                  getClipToROI() ? src.getROISize() : src.getSize(),
                  formatMatrix, 1,
                  getClipToROI() ? Rect(Point::null, src.getROISize()) : src.getROI(),
                  src.getTime())) return;

      if(dstDepth == depth64f){
        Img64f &d = dst.as64f();
        src.visit([&](const auto &s) {
          apply_ws(s, d, m_vecWeights);
        });
      }else{
        std::vector<icl32f> v(m_vecWeights.size());
        for(unsigned int i=0;i<m_vecWeights.size();++i){
          v[i] = static_cast<float>(m_vecWeights[i]);
        }
        Img32f &d = dst.as32f();
        src.visit([&](const auto &s) {
          apply_ws(s, d, v);
        });
      }
    }

  } // namespace icl::filter