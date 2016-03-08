/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/apps/lens-undistortion-calibration-opencv/     **
**          DisplacementMap.h                                      **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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
#include <ICLCore/Img.h>
#include <ICLCore/PseudoColorConverter.h>
#include <ICLQt/DrawHandle.h>
#include <ICLQt/DrawWidget.h>
#include <ICLCV/RegionDetector.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Macros.h>

namespace icl{
  namespace cv{
    
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
          core::PseudoColorConverter c;
          c.apply(s, pseudo);
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

        for(int y=0;y<h;y+=20){
          for(int x=0;x<w;x+=20){
            float d = ::sqrt(utils::sqr(cx(x,y)-x) + utils::sqr(cy(x,y)-y));
            int cidx = d/maxLen * 255;
            draw->color(lut[0][cidx], lut[1][cidx], lut[2][cidx], 255);
            draw->fill(lut[0][cidx], lut[1][cidx], lut[2][cidx], 255);
            draw->arrow(cx(x,y), cy(x,y), x, y, 3);
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
}
