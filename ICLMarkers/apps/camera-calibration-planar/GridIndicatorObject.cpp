/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/apps/camera-calibration-planar/             **
**          GridIndicatorObject.cpp                                **
** Module : ICLMarkers                                             **
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

#include "GridIndicatorObject.h"

namespace icl{
  using namespace utils;
  using namespace math;
  using namespace geom;

  namespace markers{

    struct GridIndicatorObject::MarkerObj : public SceneObject{
      int x, y;
      MarkerObj(const AdvancedMarkerGridDetector::AdvancedGridDefinition &def,
                int x, int y) : x(x), y(y){
        Rect32f b = def.getBounds(x,y);
        static const float H = 2;
        for(float h = 0; h <= H; h+=H){
          addVertex(Vec(b.x, b.y, -h, 1));
          addVertex(Vec(b.right(), b.y, -h, 1));
          addVertex(Vec(b.right(), b.bottom(), -h, 1));
          addVertex(Vec(b.x, b.bottom(), -h, 1));
        }
        for(int h=0;h<2;++h){
          for(int i=0;i<4;++i){
            addLine(4*h+i, 4*h +(i+1) % 4, h ? geom_blue(255) : geom_red(255));
          }
        }
        for(int i=0;i<4;++i){
          addLine(i, i+4, geom_blue(255));
        }
        addQuad(0,1,5,4, geom_blue(100));
        addQuad(1,2,6,5, geom_blue(100));
        addQuad(2,3,7,6, geom_blue(100));
        addQuad(3,0,4,7, geom_blue(100));

        const std::vector<int> &ids = def.getMarkerIDs();
        int id = ids[x + y * def.getSize().width];

        { /// Center text thing!
          float x = b.x, y = b.y, w = b.width, h = b.height;
          static const float bo = 0.05f, ar = 2.0f;
          float ix = x + w*bo, iw = w*(1.-2*bo);
          float ih = (1./ar) * iw, iy = y + (h-ih)*0.5 + bo;
          addVertex(Vec(ix, iy, -H ,1));
          addVertex(Vec(ix+iw, iy, -H, 1));
          addVertex(Vec(ix+iw, iy+ih, -H, 1));
          addVertex(Vec(ix,iy+ih, -H, 1));
        }
        addTextTexture(8,9,10,11, (id < 10 ? "  " : id < 100 ? " " : "") +str(id),
                       geom_blue(255));
      }
    };

    GridIndicatorObject::GridIndicatorObject(const AdvancedMarkerGridDetector::AdvancedGridDefinition &def){
      setLockingEnabled(true);
      for(int y=0;y<def.getSize().height;++y){
        for(int x=0;x<def.getSize().width;++x){
          addChild(new MarkerObj(def,x,y),true);
        }
      }
    }

    GridIndicatorObject::GridIndicatorObject(const Size &cells, const Size32f &bounds){
      float dx = bounds.width/cells.width;
      float dy = bounds.height/cells.height;

      for(int y=0;y<cells.height;++y){
        for(int x=0;x<cells.width;++x){
          addVertex(Vec(x*dx, y*dy, 0, 1));
        }
      }

      for(int y=0;y<cells.height;++y){
        for(int x=0;x<cells.width;++x){
          int idx = x + cells.width * y;
          if(x){
            addLine(idx, idx -1, geom_blue());
          }
          if(y){
            addLine(idx, idx - cells.width, geom_blue());
          }
        }
      }
    }
  }
}
