// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/ColorDistanceOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLCore/VisitorsN.h>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  template<class S>
  void apply_distance(const Img<S> &src, const double ref[3], double threshold, Image &dst) {
    if(threshold >= 0){
      Img8u &d = dst.as8u();
      double t2 = threshold * threshold;
      visitROILinesNWith<3,1>(src, d, [&](const S *r, const S *g, const S *b,
                                          icl8u *out, int w) {
        for(int i = 0; i < w; ++i){
          double dr = ref[0] - r[i], dg = ref[1] - g[i], db = ref[2] - b[i];
          out[i] = (dr*dr + dg*dg + db*db < t2) ? 255 : 0;
        }
      });
    }else{
      dst.visit([&](auto &out) {
        visitROILinesNWith<3,1>(src, out, [&](const S *r, const S *g, const S *b,
                                              auto *d, int w) {
          for(int i = 0; i < w; ++i){
            double dr = ref[0] - r[i], dg = ref[1] - g[i], db = ref[2] - b[i];
            d[i] = std::sqrt(dr*dr + dg*dg + db*db);
          }
        });
      });
    }
  }

  void ColorDistanceOp::apply(const Image &src, Image &dst) {
    ICLASSERT_RETURN(src.getChannels() == 3);
    ICLASSERT_RETURN(m_refColor.size() == 3);

    depth dstDepth = m_threshold >= 0 ? depth8u :
                     (src.getDepth() == depth64f ? depth64f : depth32f);
    if(!prepare(dst, dstDepth,
                getClipToROI() ? src.getROISize() : src.getSize(),
                formatMatrix, 1,
                getClipToROI() ? Rect(Point::null, src.getROISize()) : src.getROI(),
                src.getTime())) return;

    const double ref[3] = { m_refColor[0], m_refColor[1], m_refColor[2] };
    src.visit([&](const auto &s) {
      apply_distance(s, ref, m_threshold, dst);
    });
  }

  } // namespace icl::filter