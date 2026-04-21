// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/Img.h>
#include <icl/filter/PseudoColorOp.h>
#include <icl/qt/DrawHandle.h>
#include <icl/qt/DrawWidget.h>
#include <icl/cv/RegionDetector.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Macros.h>

namespace icl::cv {
    struct DisplacementMap{
      core::Img8u pseudo;
      const icl8u *lut[3];
      core::Img32s displacement;
      RegionDetector rd;
      void visualizeTo(const core::Img32f &warpMap, qt::DrawHandle &draw){
        if(!pseudo.getDim()){
          core::Img8u s(utils::Size(256,1),1);
          pseudo = core::Img8u(s.getSize(),core::formatRGB);
          for(int i=0;i<pseudo.getWidth();++i){
            s(i,0,0) = i;
          }
          filter::PseudoColorOp c;
          core::Image dst;
          c.apply(core::Image(s), dst);
          pseudo = dst.as8u();
          lut[0] = pseudo.begin(0);
          lut[1] = pseudo.begin(1);
          lut[2] = pseudo.begin(2);
        }

        draw->color(0,100,255,255);
        const core::Channel32f cx = warpMap[0], cy = warpMap[1];
        float maxLen = 0;
        const int w = cx.getWidth(), h = cx.getHeight();

        displacement.setChannels(1);
        displacement.setSize(warpMap.getSize());

        core::Channel32s c = displacement[0];

        for(int y=0;y<h;++y){
          for(int x=0;x<w;++x){
            float d = ::sqrt(utils::sqr(cx(x,y)-x) + utils::sqr(cy(x,y)-y)); // displacement value
            c(x,y) = round(d);
            if(d > maxLen) maxLen = d;
          }
        }
        float factor = 1;
        if(maxLen > 40){
          factor = 1./(1 + maxLen/40);
          for(int i=0;i<w*h;++i){
            c[i] *= factor;
          }
        }


        const std::vector<ImageRegion> &rs = rd.detect(&displacement);
        draw->color(255,255,255,255);
        draw->fill(0,0,0,0);
        if(rs.size() < 1000){
          for(size_t i=0;i<rs.size();++i){
            if(!(rs[i].getVal()%2)){
              draw->linestrip(rs[i].getBoundary());
            }
          }
        }
        if(factor != 1 && factor != 0){
          float f = 1./factor;
          for(int i=0;i<w*h;++i){
            c[i] *= f;
          }
        }

        draw = displacement;

        float arrowDim = (3.0*w)/640;

        for(int y=0;y<h;y+=20){
          for(int x=0;x<w;x+=20){
            float d = ::sqrt(utils::sqr(cx(x,y)-x) + utils::sqr(cy(x,y)-y));
            int cidx = d/maxLen * 255;
            draw->color(lut[0][cidx], lut[1][cidx], lut[2][cidx], 255);
            draw->fill(lut[0][cidx], lut[1][cidx], lut[2][cidx], 255);
            draw->arrow(cx(x,y), cy(x,y), x, y, arrowDim);
          }
        }
        draw->color(0,0,0,255);
        draw->text("max displacement: " + utils::str(maxLen), 5,12);
        draw->color(255,255,255,255);
        draw->text("max displacement: " + utils::str(maxLen), 4,11);
        draw->render();
      }
    };
  }