/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/DitheringOp.cpp                **
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

#include <ICLFilter/DitheringOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>

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
