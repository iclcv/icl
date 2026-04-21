// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/DitheringOp.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>

namespace icl{
  using namespace utils;
  using namespace core;

  namespace filter{
    DitheringOp::DitheringOp (Algorithm a, int l){
      setAlgorithm(a);
      setLevels(l);
    }

    inline void clipped_add(icl8u &v, int x){
      int s = static_cast<int>(v) + x;
      v = s < 0 ? 0 : s > 255 ? 255 : s;
    }

    void DitheringOp::apply(const Image &src, Image &dst) {
      if(!prepare(dst, depth8u, src.getSize(), src.getFormat(), src.getChannels(),
                  getClipToROI() ? src.getROI() : Rect(Point::null, src.getSize()),
                  src.getTime())){
        throw ICLException("DitheringOp::apply: prepare failed");
      }

      // Convert src to dst (depth → 8u)
      Rect roi;
      if(getClipToROI()){
        src.convertROITo(dst);
        roi = dst.getImageRect();
      }else{
        src.convertTo(dst);
        roi = dst.getROI();
      }

      icl8u lut[256] = {0};
      int dl = 256/m_levels, dval=255/(m_levels-1);
      for(int i=0;i<m_levels;++i){
        std::fill(lut+i*dl, lut+(i+1)*dl, i*dval);
      }

      // Floyd-Steinberg dithering in-place on dst
      const int maxx = roi.x + roi.width;
      const int maxy = roi.y + roi.height;
      Img8u &d = dst.as8u();
      for(int c=0;c<dst.getChannels();++c){
        Channel8u img = d[c];
        for(int y=roi.y; y<maxy; y++){
          for(int x=roi.x; x<maxx; x++){
            icl8u o = img(x,y);
            icl8u n = (o <= 0) ? 0 : (o >= 255) ? 255 : lut[o];
            img(x,y) = n;
            int e = o - n;
            bool xin = x+1<maxx, yin = y+1<maxy;
            if(xin) clipped_add(img(x+1,y),(e*7)/16);
            if(yin){
              clipped_add(img(x-1,y+1),(e*3)/16);
              clipped_add(img(x,y+1),(e*3)/16);
              if(xin) clipped_add(img(x+1,y+1),e/16);
            }
          }
        }
      }
    }

  } // namespace filter
}
