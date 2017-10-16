/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MarkerGridEvaluater.cpp      **
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

#include <ICLMarkers/MarkerGridEvaluater.h>
#include <ICLMath/FixedMatrix.h>
#include <ICLMath/FixedVector.h>

namespace icl{
  namespace markers{
    using namespace utils;
    using namespace math;

    static float sprod2(const Point32f &a, const FixedColVector<float,2> &b){
      return a.x*b.x + a.y*b.y;
    }

    MarkerGridEvaluater::Line::PCAInfo
    MarkerGridEvaluater::Line::perform_pca(const std::vector<Point32f> &ps){
      PCAInfo pca;
      pca.c[0] = pca.c[1] = 0;
      pca.isNull = true;

      int n = (int)ps.size();
      if(ps.size() <= 2){
        return pca;
      }
      for(int i=0;i<n;++i){
        pca.c.x += ps[i].x;
        pca.c.y += ps[i].y;
      }
      pca.c *= 1./n;

      PCAInfo::M C(0,0,0,0);
      for(int i=0;i<n;++i){
        Point32f p = ps[i] - Point32f(pca.c.x,pca.c.y);
        float xx = p.x*p.x, yy = p.y*p.y, xy = p.x*p.y;
        C(0,0) += xx;
        C(1,1) += yy;
        C(0,1) += xy;
        C(1,0) += xy;
      }
      C *= 1./n;

      try{
        C.eigen(pca.evecs,pca.evals);
      }catch(ICLException &){
        //DEBUG_LOG("eigen failed:" << e.what());
        return pca;
      }
      pca.isNull = false;
      //if(str(pca.getError()) == "-nan" || str(pca.getError()) == "nan"){
      //  std::cout << "#ps:" << ps.size() << std::endl;
      //  SHOW(cat(ps,"  "));
      //}
      return pca;
    }

    MarkerGridEvaluater::Line::Line(const std::vector<Point32f> &ps, float *error){
      isNull = true;

      PCAInfo pca = perform_pca(ps);
      if(pca.isNull) return;

      if(error) *error = pca.getError();
      // find min and max projection
      float min = 1, max = -1;
      Point32f pmin, pmax;
      PCAInfo::V v0 = pca.evecs.col(0);
      for(size_t i=0;i<ps.size();++i){
        Point32f p = ps[i] - Point32f(pca.c.x,pca.c.y);
        float pr = sprod2(p,v0);
        if(pr < min){
          min = pr;
          pmin = ps[i];
        }else if(pr > max){
          max = pr;
          pmax = ps[i];
        }
      }

      this->a.x = pca.c.x + v0.x * max;
      this->a.y = pca.c.y + v0.y * max;
      this->b.x = pca.c.x + v0.x * min;
      this->b.y = pca.c.y + v0.y * min;

      isNull = false;
    }

    template<bool STORE_LINES>
    void MarkerGridEvaluater::updateLines(){
      const int w = grid->getWidth(), h = grid->getHeight();
      this->error = 0;
      std::vector<SmartPtr<std::vector<Point32f> > > ps;

      const size_t oHorz = 2*w, oDiag=2*w+2*h;
      /// vertical lines
      for(int x=0;x<w;++x){
        ps.push_back(new std::vector<Point32f>);
        std::vector<Point32f> &l = *ps.back();
        ps.push_back(new std::vector<Point32f>);
        std::vector<Point32f> &r = *ps.back();

        for(int y=0;y<h;++y){
          const Marker &m = (*grid)(x,y);
          if(!m.wasFound()) continue;
          const Marker::KeyPoints &ip = m.getImagePoints();

          l.push_back(ip.ul);
          l.push_back(ip.ll);
          r.push_back(ip.ur);
          r.push_back(ip.lr);
        }
      }
      /// horizontal lines
      for(int y=0;y<h;++y){
        ps.push_back(new std::vector<Point32f>);
        std::vector<Point32f> &u = *ps.back();
        ps.push_back(new std::vector<Point32f>);
        std::vector<Point32f> &b = *ps.back();
        for(int x=0;x<w;++x){
          const Marker &m = (*grid)(x,y);
          if(!m.wasFound()) continue;
          const Marker::KeyPoints &ip = m.getImagePoints();
          u.push_back(ip.ul);
          u.push_back(ip.ur);
          b.push_back(ip.ll);
          b.push_back(ip.lr);
        }
      }


      // diagonal lines bottom-left to upper-right
      int xs=0, ys=1, xe=1, ye=0;
      while(true){
        ps.push_back(new std::vector<Point32f>);
        std::vector<Point32f> &l = *ps.back();
        ps.push_back(new std::vector<Point32f>);
        std::vector<Point32f> &c = *ps.back();
        ps.push_back(new std::vector<Point32f>);
        std::vector<Point32f> &r = *ps.back();

        ps.push_back(new std::vector<Point32f>);
        std::vector<Point32f> &lu = *ps.back();
        ps.push_back(new std::vector<Point32f>);
        std::vector<Point32f> &cu = *ps.back();
        ps.push_back(new std::vector<Point32f>);
        std::vector<Point32f> &ru = *ps.back();

        for(int x=xs,y=ys; x<=xe;++x, --y){
          const Marker &m = (*grid)(x,y), &mu = (*grid)(x,h-1-y);
          if(m.wasFound()){
            const Marker::KeyPoints &ip = m.getImagePoints();
            l.push_back(ip.ul);
            c.push_back(ip.ll);
            c.push_back(ip.ur);
            r.push_back(ip.lr);
          }
          if(mu.wasFound()){
            const Marker::KeyPoints &ip = mu.getImagePoints();
            lu.push_back(ip.ll);
            cu.push_back(ip.ul);
            cu.push_back(ip.lr);
            ru.push_back(ip.ur);
          }
        }

        if(++ys >= h){
          ys = h-1;
          ++xs;
        }
        if(++xe >= w){
          xe = w-1;
          ++ye;
        }
        if(ye == h-1) break;
      }

      /*

      0 1 2 3 4 5 6
      | | | | | | |
      0- a b c d e f +
      1- g h i j k l *
      2- m n o p q r #
      3- s t u v w x =
      4- 0 1 2 3 4 5 &

      */

      if(STORE_LINES){
        lines.resize(ps.size());
      }
      int nValid = 0;
      float e = 0;
      for(size_t i=0;i<ps.size();++i){
        if(STORE_LINES){
          lines[i] = Line(*ps[i],&e);

          if(lines[i]){
            error += ::sqrt(e);
            ++nValid;
            lines[i].type = ( i < oHorz ? 0 :
                              i < oDiag ? 1 : 2 );
          }
        }else{
          Line::PCAInfo p = Line::perform_pca(*ps[i]);
          if(p) {
            /*if(str(p.getError()) == "nan"){
              SHOW(ps[i]->size());
              std::cout << "points:" << std::endl;
              for(int j=0;j<ps[i]->size();++j){
                std::cout << i << " :" << (*ps[i])[j] << std::endl;
              }

              throw 32;
                }*/
            error += ::sqrt(p.getError());
            ++nValid;
          }
        }
      }
      if(nValid){
        error /= nValid;
      }
    }

    template void MarkerGridEvaluater::updateLines<true>();
    template void MarkerGridEvaluater::updateLines<false>();

    float MarkerGridEvaluater::evalError(bool storeLines) {
      if(storeLines){
        updateLines<true>();
      }else{
        updateLines<false>();
      }
      return error;
    }

    utils::VisualizationDescription MarkerGridEvaluater::vis() const{
      VisualizationDescription vd;

      for(size_t i=0;i<lines.size();++i){
        const Line &l = lines[i];
        if(l.isNull) continue;
        switch(l.type){
          case 0: vd.color(255,0,100,255); break;
          case 1: vd.color(0,255,100,255); break;
          default: vd.color(255,100,0,255); break;
        }
        vd.line(l.a,l.b);
      }
      return vd;
    }

    float MarkerGridEvaluater::compute_error(const MarkerGrid &g){
      MarkerGridEvaluater e(&g);
      return e.evalError(false);
    }
  }
}
