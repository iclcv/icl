/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/LineSampler.cpp                    **
** Module : ICLCore                                                **
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

#include <ICLCore/LineSampler.h>

namespace icl{
  
  using namespace utils;
  
  namespace core{
    
    namespace{

      template<bool useBounds>
      bool bounds_check(int x, int y,const int bounds[4]){
        return x>=bounds[0] && y>=bounds[1] && x<=bounds[2] && y<=bounds[3];
      }
      template<> bool bounds_check<false>(int,int,const int[4]) { return true; }

      /** UnaryPointFunc has functions:
          void init(bool invertOrder);
          void push(const Point &p);
          void inc(int direction);
        */
      
      template<class UnaryPointFunc, int ystep, bool steep2, bool steep, bool usebr>
      inline void bresenham_internal_generic_level_4(int x0, int y0, int x1, int y1,const int bounds[4], UnaryPointFunc f){
        const int deltax = x1 - x0;
        const int deltay = std::abs(y1 - y0);
        int error = 0;
        const int dp = steep2 ? -1 : 1;
        f.init(steep2); 
        
        for(int x=x0,y=y0;x<=x1;x++){
          if( bounds_check<usebr>(steep?y:x,steep?x:y,bounds) ){
            f.push(Point(steep?y:x,steep?x:y));
            f.inc(dp);
          }
          error += deltay;
          if (2*error >= deltax){
            y += ystep;
            error -=deltax;
          }
        }
      }
      
      template<class UnaryPointFunc, int ystep, bool steep2, bool steep>
      inline void bresenham_internal_generic_level_3(int x0, int y0, int x1, int y1,const int br[4],UnaryPointFunc f){
        if(br){
          bresenham_internal_generic_level_4<UnaryPointFunc,ystep,steep2,steep,true>(x0,y0,x1,y1,br,f);
        }else{
          bresenham_internal_generic_level_4<UnaryPointFunc,ystep,steep2,steep,false>(x0,y0,x1,y1,br,f);
        }
      }
      

      template<class UnaryPointFunc, int ystep, bool steep2>
      inline void bresenham_internal_generic_level_2(int x0, int y0, int x1, int y1,const int br[4], bool steep, UnaryPointFunc f){
        if(steep){
          bresenham_internal_generic_level_3<UnaryPointFunc,ystep,steep2,true>(x0,y0,x1,y1,br,f);
        }else{
          bresenham_internal_generic_level_3<UnaryPointFunc,ystep,steep2,false>(x0,y0,x1,y1,br,f);
        }
      }
      
      
      template<class UnaryPointFunc, int ystep>
      inline void bresenham_internal_generic_level_1(int x0, int y0, int x1, int y1,const int br[4], 
                                                     bool steep, bool steep2, UnaryPointFunc f){
        if(steep2){
          bresenham_internal_generic_level_2<UnaryPointFunc,ystep,true>(x0,y0,x1,y1,br,steep,f);
        }else{
          bresenham_internal_generic_level_2<UnaryPointFunc,ystep,false>(x0,y0,x1,y1,br,steep,f);
        }
      }
      
      template<class UnaryPointFunc>
      inline void bresenham_internal_generic_level_0(int x0, int y0, int x1, int y1,const int br[4], UnaryPointFunc f){
        bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
        if(steep){
          std::swap(x0, y0);
          std::swap(x1, y1);
        }
        bool steep2 = x0 > x1;
        if(steep2){
          std::swap(x0, x1);
          std::swap(y0, y1);
        }
        
        // new optimization: if start and end point are IN the bounding rect,
        // bounds dont need to be check for every point!
        const int *useBR = br;
        if(br && bounds_check<true>(x0, y0, br) && bounds_check<true>(x1, y1, br) ){
          useBR =  0;
        }

        if(y0 < y1){
          bresenham_internal_generic_level_1<UnaryPointFunc,1>(x0,y0,x1,y1,useBR,steep,steep2,f);
        }else{
          bresenham_internal_generic_level_1<UnaryPointFunc,-1>(x0,y0,x1,y1,useBR,steep,steep2,f);
        } 
      } 
    }


    LineSampler::LineSampler(int maxLen):
      m_buf(maxLen){}
    
    LineSampler::LineSampler(const Rect &br):
      m_buf(iclMax(br.width,br.height)){
      setBoundingRect(br);
    }
    
    
    void LineSampler::setBoundingRect(const Rect &bb){
      m_br.resize(4);
      m_br[0] = bb.x;
      m_br[1] = bb.y;
      m_br[2] = bb.right();
      m_br[3] = bb.bottom();
    }
    
    void LineSampler::removeBoundingBox(int bufferSize){
      m_buf.resize(bufferSize);
      m_br.clear();
    }
    
    
    template<bool vertical, bool forward, bool useBR>
    std::pair<Point*,Point*> sample_simple_line_level_3(int c, int a, int b, const int br[4], Point *buf, int bufSize){
      Point *p = forward ? buf : buf+bufSize-1;
      if(!forward) std::swap(a,b);
      for(int i=a;i<=b;++i){
        Point pt = vertical ? Point(c,i) : Point(i,c);
        if(bounds_check<useBR>(pt.x,pt.y,br)){
          *p = pt;
          if(forward) ++p; else --p;
        }
      }
      
      return forward ? std::make_pair(buf, p) : std::make_pair(p+1, buf+bufSize);
    }

    template<bool vertical, bool forward>
    std::pair<Point*,Point*> sample_simple_line_level_2(int c, int a, int b, const int br[4], 
                                                        Point *buf, int bufSize){
      if(br){
        return sample_simple_line_level_3<vertical,forward,true>(c,a,b,br, buf, bufSize);
      }else{
        return sample_simple_line_level_3<vertical,forward,false>(c,a,b, br, buf, bufSize);
      }
    }
    
    template<bool vertical>
    std::pair<Point*,Point*> sample_simple_line_level_1(int c, int a, int b, const int br[4], 
                                                        Point *buf, int bufSize){
      const int *useBR = br;

      if(vertical){
        if(br && bounds_check<true>(c,a,br) && bounds_check<true>(c,b,br) ) useBR = br;
      }else{
        if(br && bounds_check<true>(a,c,br) && bounds_check<true>(b,c,br) ) useBR = br;
      }
                                    
      if(a<=b){
        return sample_simple_line_level_2<vertical,true>(c,a,b, useBR, buf, bufSize);
      }else{
        return sample_simple_line_level_2<vertical,false>(c,a,b, useBR, buf, bufSize);
      }
    }
    
  
    std::pair<Point*,Point*> sample_simple_line_level_0(bool vertical, int x0, int y0, int x1, int y1, const int br[4], 
                                                        std::vector<Point> &dst){
      if(vertical){
        return sample_simple_line_level_1<true>(x0,y0,y1, br, dst.data(), dst.size());
      }else{
        return sample_simple_line_level_1<false>(y0,x0,x1, br, dst.data(), dst.size());
      }
    }

    struct PushList{
      Point *buf;
      int size;
      Point *&cp;
      bool &inverted;
      inline PushList(Point *buf, int size, Point *&cp, bool &inverted):
        buf(buf),size(size), cp(cp),inverted(inverted){}
      
      inline void init(bool invertOrder){
        if(invertOrder) {
          cp = buf + size-1;
        }else{
          cp = buf;
        }
        inverted = invertOrder;
      }
      
      inline void push(const Point &p){
        *cp = p;
      }
      
      inline void inc(int dir){
        cp += dir;
      }
    };


    
    LineSampler::Result LineSampler::sample(const Point &a, const Point &b){
      int dx = std::abs(a.x - b.x);
      int dy = std::abs(a.y - b.y);
      if(!m_buf.size()) m_buf.resize(iclMax(dx,dy)+1); 
      if(!dx || !dy){
        std::pair<Point*,Point*> res = sample_simple_line_level_0(!dx, a.x, a.y, b.x, b.y, m_br.size() ? m_br.data() : 0, m_buf);
        Result r = { res.first, (int)(res.second - res.first) };
        return r;
      }                     


      bool inverted = false;
      Point *cp = 0;
      PushList pl(m_buf.data(), m_buf.size(), cp, inverted);
      bresenham_internal_generic_level_0<PushList>(a.x,a.y,b.x,b.y,
                                                   m_br.size()?m_br.data():0,pl);
      Point *begin = inverted ? cp+1 : m_buf.data();
      Point *end = inverted ? m_buf.data()+m_buf.size() : cp ;
      Result r = { begin, (int)(end - begin) };
      return r;
    }

   
      /// samples the line into the given destination vector
    void LineSampler::sample(const Point &a, const Point &b, std::vector<Point> &dst){
      int dx = std::abs(a.x - b.x);
      int dy = std::abs(a.y - b.y);
      dst.resize(iclMax(dx,dy)+1); 

      if(!dx || !dy){
        sample_simple_line_level_0(!dx, a.x, a.y, b.x, b.y, m_br.size() ? m_br.data() : 0, dst);
      }     
      
      bool inverted = false;
      Point *cp = 0;
      PushList pl(dst.data(), dst.size(), cp, inverted);
      bresenham_internal_generic_level_0<PushList>(a.x,a.y,b.x,b.y,
                                                   m_br.size()?m_br.data():0,pl);
    }

  } // namespace core
}
