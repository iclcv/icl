#include <iclHoughLine.h>
#include <iclPoint32f.h>
#include <iclRange.h>
#include <iclFixedMatrix.h>
#include <cmath>
#include <iclImgChannel.h>

namespace icl{

  /// create dedicated file for this class
  HoughLine::HoughLine():m_distance(0),m_angle(0){}
  
  HoughLine::HoughLine(const Point32f &offs, const Point32f &dir):
    m_offset(offs),m_direction(dir){
    
    m_angle=::atan2(m_direction.y,m_direction.x);
    m_distance=m_offset.norm();
    
  }
  HoughLine::HoughLine(icl32f distance, icl32f angle):
    m_distance(distance),m_angle(angle){
    
    float c = ::cos(angle);
    float s = ::sin(angle);
    m_offset = Point32f(c,s)*distance;
    m_direction = Point32f(-s,c);
  }

  namespace{
    
    template<class T, int C, bool withAlpha>
    void taintPixel(ImgChannel<T> cs[C],int x, int y, const icl32f col[C]){
      if(withAlpha){
        const float a = col[3]/255;
        for(int c=0;c<C;++c){
          T &p = cs[c](x,y);
          p = clipped_cast<icl32f,T>(a*col[c]+(1.0-a)*p);
        }
      }else{
        for(int c=0;c<C;++c){
          cs[c](x,y)=clipped_cast<icl32f,T>(col[c]);        
        }
      }
    }


    template<class T, int C, bool withAlpha>
    void sample_line_t_c_a(Img<T> &image,const HoughLine &p, const icl32f col[C]){
      ImgChannel<T> cs[C];
      pickChannels(&image,cs);
      
      // sampling y(x) = mx+b
      float m = -1.0/::tan(p.theta());
      float b = p.rho()/sin(p.theta());
      
      for(int x=image.getWidth()-1;x>=0;--x){
        int y = ::round(m*x+b);
        if(y>=0 && y<image.getHeight()){
          taintPixel<T,C,withAlpha>(cs,x,y,col);
        }
      }
      
      m = -::tan(p.theta());
      b = p.rho()/cos(p.theta());
      
      for(int y=image.getHeight()-1;y>=0;--y){
        int x = ::round(m*y+b);
        if(x >= 0 && x < image.getWidth()){
          taintPixel<T,C,withAlpha>(cs,x,y,col);
        }
      }
    }
    
    template<class T>
    void sample_line_c(Img<T> &image,const HoughLine &p, const icl32f *col){
      if(col[3] == 255){
        switch(image.getChannels()){
          case 1: sample_line_t_c_a<T,1,false>(image,p,col); break;
          case 2: sample_line_t_c_a<T,2,false>(image,p,col); break;
          case 3: sample_line_t_c_a<T,3,false>(image,p,col); break;
          default:
            ERROR_LOG("sampling lines is only supported for 1,2,3 and channel images");
        }
      }else{
        switch(image.getChannels()){
          case 1: sample_line_t_c_a<T,1,true>(image,p,col); break;
          case 2: sample_line_t_c_a<T,2,true>(image,p,col); break;
          case 3: sample_line_t_c_a<T,3,true>(image,p,col); break;
          default:
            ERROR_LOG("sampling lines is only supported for 1,2,3 and channel images");
        }
      }
    }
  }
  
  void HoughLine::sample(ImgBase *image, icl32f r, icl32f g, icl32f b, icl32f alpha) const{
    const icl32f color[4] = {r,g,b,alpha};
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                          \
      case depth##D:                                      \
      sample_line_c(*image->asImg<icl##D>(),*this,color);     \
      break;                                                        
      
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
      }
  }

  
  Point32f HoughLine::getIntersection(const HoughLine &a, const HoughLine &b){
    
    float sa = ::sin(a.m_angle);
    float sb = ::sin(b.m_angle);
    float ca = ::cos(a.m_angle);
    float cb = ::cos(b.m_angle);
    const float &r = a.m_distance;
    const float &s = b.m_distance;
    
    FixedMatrix<float,2,2> M(-sa, sb,
                             ca, -cb);
    FixedMatrix<float,1,2> B(-r*ca + s*cb,
                             -r*sa + s*sb);
    
    try{ // Maybe M cannot be inverted if lines are colinear
      FixedMatrix<float,1,2> x = M.inv()*B; 
      // insert x[0] = lamda1 into line-equation a
      return a.m_offset + (a.m_direction*x[0]);
    }catch(SingularMatrixException&){}
    return NO_INTERSECTION;
  }

  
  std::vector<Point32f> HoughLine::getPairwiseIntersections(const std::vector<HoughLine> &lines){
    std::vector<Point32f> v;
    for(unsigned int i=0;i<lines.size();++i){
      for(unsigned int j=i+1;j<lines.size();++j){
        Point32f p = getIntersection(lines[i],lines[j]);
        if(p != NO_INTERSECTION){
          v.push_back(p);
        }
      }
    }
    return v;

  }
  
  const Point32f HoughLine::NO_INTERSECTION = Point32f(Range32f::limits().maxVal,Range32f::limits().maxVal);
  
}
