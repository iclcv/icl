/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/FoldMap.cpp                  **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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
#include <ICLPhysics/FoldMap.h>
#include <ICLCore/LineSampler.h>


namespace icl{
  namespace physics{
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
        if((unsigned)p.x < w && (unsigned)p.y < h){
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
        if((unsigned)p.x >= w || (unsigned)p.y >= h){
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
}
