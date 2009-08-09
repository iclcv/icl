#include <iclSampledLine.h>

namespace icl{

  Point SampledLine::buf[MAX_LINE_LENGTH];

  template<bool steep, bool steep2, int ystep>
  inline void SampledLine::bresenham_templ(int x0, int x1, int y0, int y1, int minX, int maxX, int minY, int maxY, Point *p){
    Point *buf = p;
    int deltax = x1 - x0;
    int deltay = std::abs(y1 - y0);
    int error = 0;
    
    if(steep2) p += MAX_LINE_LENGTH-1;
    
    for(int x=x0,y=y0;x<=x1;x++){
      if(steep){
        if( x>=minY && x<maxY && y>=minX && y<=maxX){
          if(steep2){
            *p-- = Point(y,x);
          }else{
            *p++ = Point(y,x);
          }
        }
      }else{
        if( x>=minX && x<maxX && y>=minY && y<maxY){
          if(steep2){
            *p-- = Point(x,y);
        }else{
            *p++ = Point(x,y);
          }
        }
      }
      error += deltay;
      if (2*error >= deltax){
        y += ystep;
        error -=deltax;
      }
    }
    
    if(steep2){
      init(p+1,buf+MAX_LINE_LENGTH);
    }else{
      init(buf,p);
    }
  }

  template<bool steep, bool steep2, int ystep>
  inline void SampledLine::bresenham_templ_2(int x0, int x1, int y0, int y1, Point *p){
    Point *buf = p;
    int deltax = x1 - x0;
    int deltay = std::abs(y1 - y0);
    int error = 0;
    
    if(steep2) p += MAX_LINE_LENGTH-1;
    
    for(int x=x0,y=y0;x<=x1;x++){
      if(steep){
        if(steep2){
          *p-- = Point(y,x);
        }else{
          *p++ = Point(y,x);
        }
      }else{
        if(steep2){
          *p-- = Point(x,y);
        }else{
          *p++ = Point(x,y);
        }
      }
      
      error += deltay;
      if (2*error >= deltax){
        y += ystep;
        error -=deltax;
      }
    }
    
    if(steep2){
      init(p+1,buf+MAX_LINE_LENGTH);
    }else{
      init(buf,p);
    }
  }

  
  void SampledLine::bresenham(int x0, int x1, int y0, int y1, int minX, int maxX, int minY, int maxY){
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
    
    if(y0 < y1){
      if(steep){
        if(steep2){
          return bresenham_templ<true,true,1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf);
        }else{
          return bresenham_templ<true,false,1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf);
        }
      }else{
        if(steep2){
          return bresenham_templ<false,true,1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf);
        }else{
          return bresenham_templ<false,false,1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf);
        }
      }
    }else{
      if(steep){
        if(steep2){
          return bresenham_templ<true,true,-1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf);
        }else{
          return bresenham_templ<true,false,-1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf);
        }
      }else{
        if(steep2){
          return bresenham_templ<false,true,-1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf);
        }else{
          return bresenham_templ<false,false,-1>(x0,x1,y0,y1,minX,maxX,minY,maxY,buf);
        }
      }
    }
  }
  
  void SampledLine::bresenham(int x0, int x1, int y0, int y1){
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
    
    if(y0 < y1){
      if(steep){
        if(steep2){
          return bresenham_templ_2<true,true,1>(x0,x1,y0,y1,buf);
        }else{
          return bresenham_templ_2<true,false,1>(x0,x1,y0,y1,buf);
        }
      }else{
        if(steep2){
          return bresenham_templ_2<false,true,1>(x0,x1,y0,y1,buf);
        }else{
          return bresenham_templ_2<false,false,1>(x0,x1,y0,y1,buf);
        }
      }
    }else{
      if(steep){
        if(steep2){
          return bresenham_templ_2<true,true,-1>(x0,x1,y0,y1,buf);
        }else{
          return bresenham_templ_2<true,false,-1>(x0,x1,y0,y1,buf);
        }
      }else{
        if(steep2){
          return bresenham_templ_2<false,true,-1>(x0,x1,y0,y1,buf);
        }else{
          return bresenham_templ_2<false,false,-1>(x0,x1,y0,y1,buf);
        }
      }
    }
  }
}
