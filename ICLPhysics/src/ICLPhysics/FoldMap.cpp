// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLPhysics/FoldMap.h>
#include <ICLUtils/Point32f.h>
#include <ICLCore/LineSampler.h>


namespace icl::physics {
    using namespace utils;
    using namespace core;

    void FoldMap::clear() {
      m.fill(initialValue);
    }

    FoldMap::FoldMap(const utils::Size &resolution, float initialValue):
      m(resolution,1),initialValue(initialValue){
      clear();
    }

    FoldMap::FoldMap(const Img32f &image, float initialValue):
      m(image.getSize(),1),initialValue(initialValue){
        std::copy(image.begin(0), image.end(0), m.begin(0));
    }

    void FoldMap::draw_fold(const utils::Point32f &a, const utils::Point32f &b, float value){
      LineSampler ls;

      LineSampler::Result r = ls.sample(Point(a.x * (m.getWidth()-1), a.y * (m.getHeight()-1)),
                                        Point(b.x * (m.getWidth()-1), b.y * (m.getHeight()-1)) );

      Channel32f c  = m[0];
      const unsigned int w = c.getWidth()-1, h = c.getHeight()-1;
      for(int i=0;i<r.n;++i){
        const Point &p = r[i];
        if(static_cast<unsigned>(p.x) < w && static_cast<unsigned>(p.y) < h){
          c(p.x,p.y) = value;
          c(p.x+1,p.y) = value;
          c(p.x,p.y+1) = value;
        }
      }
    }

    void FoldMap::addFold(const utils::Point32f &a, const utils::Point32f &b, float value){
      draw_fold(a,b,value);
    }

    void FoldMap::removeFold(const utils::Point32f &a, const utils::Point32f &b){
      draw_fold(a,b,initialValue);
    }


    float FoldMap::getFoldValue(const utils::Point32f &a, const utils::Point32f &b){
      float min = 10e35;
      float maxNeg = -10e35;
      LineSampler ls;
      LineSampler::Result r = ls.sample(Point(a.x * (m.getWidth()-1), a.y * (m.getHeight()-1)),
                                        Point(b.x * (m.getWidth()-1), b.y * (m.getHeight()-1)) );
      const Channel32f c  = m[0];
      const unsigned int w = c.getWidth(), h = c.getHeight();
      bool anyNeg = false;
      for(int i=1;i<r.n-2;++i){
        const Point &p = r[i];
        if(static_cast<unsigned>(p.x) >= w || static_cast<unsigned>(p.y) >= h){
          continue;
        }
        const float cc = c(p.x,p.y);
        if(cc >= 0){
          if(cc < min){
            min = cc;
          }
        }else{
          if(cc > maxNeg){
            anyNeg = true;
            maxNeg = cc;
          }
        }
      }
      if(min>=1){
        if(anyNeg) return maxNeg;
        else return 1;
      }else{
        return min;
      }
    }

  }