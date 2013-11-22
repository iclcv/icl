/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/demos/canvas/canvas.cpp                        **
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

#include <ICLQt/Common.h>
#include <ICLCore/AbstractCanvas.h>
#include <ICLCore/LineSampler.h>
#include <ICLUtils/Random.h>
#include <ICLMath/LeastSquareModelFitting2D.h>

struct Canvas : public AbstractCanvas{
  typedef void (*point_func)(const Point32f&, void **, int, const AbstractCanvas::Color&);
  typedef void (*line_func)(LineSampler &ls, const Point32f&, const Point32f&, 
                            void **, int, const AbstractCanvas::Color&);
  typedef void (*triangle_func)(const Point32f&, const Point32f &, 
                                const Point32f&,void **, const int, 
                                const AbstractCanvas::Color &,
                                const AbstractCanvas::ClipRect&);
  LineSampler ls;
  void *data[4];
  Size size;
  depth d;
  int c;
  static const float ALPHA_SCALE = 0.00392156862745098039; // 1./255
  
  struct Functions{
    point_func f_point;
    line_func f_line;
    triangle_func f_triangle;
  } funcs[2]; // 0=no alpha, 1 = with alpha
  
  template<class T>
  static inline void set_color_alpha(T &t, const float c, const float a){
    t = clipped_cast<float,T>(c*a + t*(1-a));
  }
  template<class T>
  static inline void set_color(T &t, const float c){
    t = clipped_cast<float,T>(c);
  }
  static inline int get_idx(float x, float y, int w){
    return (int)(round(x)+round(y)*w);
  }
  static inline int get_idx(int x, int y, int w){
    return x+w*y;
  }
  static inline int get_idx(const Point32f &p, int w){
    return get_idx(p.x,p.y,w);
  }
  static inline int get_idx(const Point &p, int w){
    return get_idx(p.x,p.y,w);
  }
  
  template<class T, int CHAN, bool WITH_ALPHA> 
  static inline void set_color_gen(void **data, int o, const AbstractCanvas::Color &c, float aScaled=0){
    if(WITH_ALPHA){
      if(CHAN > 0) set_color_alpha((((T**)data)[0])[o],c[0],aScaled);
      if(CHAN > 1) set_color_alpha((((T**)data)[1])[o],c[1],aScaled);
      if(CHAN > 2) set_color_alpha((((T**)data)[2])[o],c[2],aScaled);
      if(CHAN > 3) set_color_alpha((((T**)data)[3])[o],c[3],aScaled);
    }else{
      if(CHAN > 0) set_color((((T**)data)[0])[o],c[0]);
      if(CHAN > 1) set_color((((T**)data)[1])[o],c[1]);
      if(CHAN > 2) set_color((((T**)data)[2])[o],c[2]);
      if(CHAN > 3) set_color((((T**)data)[3])[o],c[3]);
    }
  }

  
  template<int CHAN, class T, bool WITH_ALPHA>
  static void point_template(const Point32f &p, void **data, const int w, 
                             const AbstractCanvas::Color &c){
    // we hope that c[3]*A is optimized out if not needed
    set_color_gen<T,CHAN,WITH_ALPHA>(data, get_idx(p,w), c, c[3]*ALPHA_SCALE); 
  }
  
  template<int CHAN, class T, bool WITH_ALPHA>
  static void line_template(LineSampler &ls,
                            const Point32f &a, const Point32f &b, 
                            void **data, const int w, 
                            const AbstractCanvas::Color &c){
    LineSampler::Result r = ls.sample(a,b);
    const float aScaled = c[3]*ALPHA_SCALE;      
    for(int i=0;i<r.n;++i){
      set_color_gen<T,CHAN,WITH_ALPHA>(data, get_idx(r[i],w), c, aScaled);
    }
  }

  static inline bool less_pt_y(const Point &a, const Point &b){
    return a.y<b.y;
  }

  template<int CHAN, class T, bool WITH_ALPHA>
  static inline void hlinef(void **data, float x1, float x2, float y, int w, 
                            const AbstractCanvas::Color &cFill, float aScaled,
                            const AbstractCanvas::ClipRect &clip){
    if(y < clip.miny || y > clip.maxy || x2 < clip.minx || x1 > clip.maxx) return;
    if(x1 < clip.minx) x1 = clip.minx;
    if(x2 > clip.maxx) x2 = clip.maxx;

    const int oL = (int)round(y)*w;
    const int oS = round(x1) + oL;
    const int oE = round(x2) + oL;
    for(int o=oS;o<oE;++o){
      set_color_gen<T,CHAN,WITH_ALPHA>(data,o,cFill,aScaled);
    }
  }


  template<int CHAN, class T, bool WITH_ALPHA>
  static void triangle_template(const Point32f &p1, const Point32f &p2, const Point32f &p3,
                                void **data, const int w, const AbstractCanvas::Color &cFill,
                                const AbstractCanvas::ClipRect &clip){
    const float aScaled = cFill[3]*ALPHA_SCALE;      

    Point ps[3] = { p1, p2, p3 };
    std::sort((Point*)ps,(Point*)(ps+3),less_pt_y);
    const Point &A=ps[0], &B=ps[1], &C=ps[2];
    float dx1,dx2,dx3;
    if (B.y-A.y > 0){
      dx1=float(B.x-A.x)/(B.y-A.y);
    }else{
      dx1=B.x - A.x;
    }
    if (C.y-A.y > 0){
      dx2=float(C.x-A.x)/(C.y-A.y);
    }else{
      dx2=0;
    }
    if (C.y-B.y > 0){
      dx3=float(C.x-B.x)/(C.y-B.y);
    }else{
      dx3=0;
    }
    
    Point32f S = Point32f(A.x,A.y);
    Point32f E = Point32f(A.x,A.y);
    if(dx1 > dx2) {
      for(;S.y<=B.y;++S.y,++E.y,S.x+=dx2,E.x+=dx1){
        hlinef<CHAN,T,WITH_ALPHA>(data,S.x,E.x,S.y,w,cFill,aScaled,clip);
      }
      E=Point32f(B.x+dx3,B.y+1);
      for(;S.y<=C.y;S.y++,E.y++,S.x+=dx2,E.x+=dx3){
        hlinef<CHAN,T,WITH_ALPHA>(data,S.x,E.x,S.y,w,cFill,aScaled,clip);
        //            hlinef(image,S.x,E.x,S.y,true);
      }
    }else {
      for(;S.y<=B.y;S.y++,E.y++,S.x+=dx1,E.x+=dx2){
        hlinef<CHAN,T,WITH_ALPHA>(data,S.x,E.x,S.y,w,cFill,aScaled,clip);
        //        hlinef(image,S.x,E.x,S.y,true);
      }
      S=Point32f(B.x+dx3,B.y+1);
      for(;S.y<=C.y;S.y++,E.y++,S.x+=dx3,E.x+=dx2){
        hlinef<CHAN,T,WITH_ALPHA>(data,S.x,E.x,S.y,w,cFill,aScaled,clip);
        //hlinef(image,S.x,E.x,S.y,true);
      }
    }
  }

  template<class T, int CHAN>
  void link_pointers(){
    funcs[0].f_point = &point_template<CHAN,T,false>;
    funcs[1].f_point = &point_template<CHAN,T,true>;
    funcs[0].f_line = &line_template<CHAN,T,false>;
    funcs[1].f_line = &line_template<CHAN,T,true>;
    funcs[0].f_triangle = &triangle_template<CHAN,T,false>;
    funcs[1].f_triangle = &triangle_template<CHAN,T,true>;
  }
    
  Canvas(ImgBase *image){
    ICLASSERT_THROW(image,ICLException("Canvas::Canvas: image was null"));
    ICLASSERT_THROW(image->getDim(), ICLException("Canvas::Canvas: image size was 0x0"));
    ICLASSERT_THROW(image->getChannels()>0 && image->getChannels()<=4, 
                    ICLException("Canvas::Canvas: image must have 1,2,3 or 4 channels"));
    std::fill(data,data+4,(void*)0);
    for(int i=0;i<image->getChannels() && i<4;++i){
      data[i] = image->getDataPtr(i);
    }
    this->size = image->getSize();
    this->d = image->getDepth();
    this->c = image->getChannels();

    state.clip = AbstractCanvas::ClipRect(0,size.width-1,0,size.height-1);

    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                       \
      case depth##D:                                   \
        switch(this->c){                               \
          case 1:  link_pointers<icl##D,1>(); break;   \
          case 2:  link_pointers<icl##D,2>(); break;   \
          case 3:  link_pointers<icl##D,3>(); break;   \
          case 4:  link_pointers<icl##D,4>(); break;   \
          default: break;                              \
        }                                              \
        break;
  ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
       default: ICL_INVALID_DEPTH; break;
     }                
  }
  
  inline bool hasLineAlpha() const{
    return state.linecolor[3] != 255;
  }

  inline bool hasFillAlpha() const{
    return state.fillcolor[3] != 255;
  }
  
  
  virtual void draw_point_internal(const utils::Point32f &p){
    if(clip(p)){
      funcs[hasLineAlpha()].f_point(p,data,size.width,state.linecolor);
    }
  }

  virtual void draw_line_internal(const utils::Point32f &a, 
                                  const utils::Point32f &b){
    ls.setBoundingRect(getClipRect());
    funcs[hasLineAlpha()].f_line(ls, a, b, data,size.width,state.linecolor);
  }
  virtual void fill_triangle_internal(const utils::Point32f &a, 
                                      const utils::Point32f &b,
                                      const utils::Point32f &c){
    funcs[hasFillAlpha()].f_triangle(a,b,c, data,size.width, state.fillcolor, state.clip);
  }
  virtual void draw_ellipse_internal(const utils::Point32f &c,
                                     const utils::Point32f &axis1,
                                     const utils::Point32f &axis2){
  
  }
  virtual void draw_image_internal(const utils::Point32f &ul, 
                                   const utils::Point32f &ur, 
                                   const utils::Point32f &lr, 
                                   const utils::Point32f &ll,
                                   float alpha, scalemode sm){
  
  }
  
};


struct Ellipse{
  float p[6];
  Ellipse(const std::vector<double> &ps){
    std::copy(ps.begin(),ps.end(),p);
    SHOW(p[0]);
    SHOW(p[1]);
    SHOW(p[2]);
    SHOW(p[3]);
    SHOW(p[4]);
    SHOW(p[5]);
  }
  inline float f(float x, float y) const{
    return p[0] *sqr(x) + p[1]*x*y + p[2] * sqr(y) + p[3] * x + p[4] *y + p[5];
  }
  
  bool operator()(float x, float y) const{
    return f(x,y) > 0;
  }
};

void fill_ellipse_test_2(Channel32f C, AbstractCanvas::Transform T, Rect32f r){
  typedef FixedColVector<float,2> Vec2;
  typedef FixedColVector<float,3> Vec3;
  typedef LeastSquareModelFitting2D Fit;
  Fit fit(6,Fit::ellipse_gen);
  const Point32f c = r.center();
  const Vec3 ps[3] = { 
    T * Vec3(r.right(), c.y, 1),
    T * Vec3(r.left(), c.y, 1),
    T * Vec3(c.x,r.top(), 1)
    //T * Vec3(c.x,r.bottom(), 1)
  };
  std::vector<Point32f> psv(3);
  for(int i=0;i<3;++i){
    psv[i] = Point32f(ps[i].x,ps[i].y);
    std::cout << "psv[" << i << "]: "<< psv[i] << std::endl;
  }

  std::vector<double> params = fit.fit(psv);
  for(int i=0;i<(int)params.size();++i){
    std::cout << "params[" << i << "]: "<< params[i] << std::endl;
  }
  Ellipse e(params);
  

  for(float y=0;y<1000;++y){
    for(float x=0;x<1000;++x){
      C(x,y) = e.f(x,y);
      if(e(x,y)) C(x,y) = 255;
    }
  }
}

void fill_ellipse_test(Channel32f C, AbstractCanvas::Transform Tglobal, Rect32f r){
  float cx = r.x + r.width/2;
  float cy = r.y + r.height/2;
  
  typedef FixedColVector<float,2> Vec2;
  typedef FixedMatrix<float,2,2> Mat2;

  //  Tglobal = Tglobal.inv();
  Mat2 R = Tglobal.part<0,0,2,2>();
  Vec2 t = Tglobal.part<2,0,1,2>();

  //  Mat2 T(r.width/2,0,
  //       0,r.height/2);
  //T = T*T;
  //T = T.inv();

  // opt ??
  Mat2 T(1./sqr(r.width/2),0,
         0, 1./sqr(r.height/2));

  Vec2 c = Vec2(cx,cy) + t;

  for(float y=0;y<1000;++y){
    for(float x=0;x<1000;++x){
      Vec2 v = R*Vec2(x,y)-c;
      v = v + t;
      float val = v.transp() * T * v;
      if(val < 1) C(x,y) = 255;
    }
  }

#if 0
  
  float a = T(0,0), b = T(1,0), tx=T(2,0);
  float c = T(0,1), d = T(1,1), ty=T(2,1);
  
  
  
  for(int y=0;y<1000;++y){
    float p = (y*(b+c)+tx)/a;
    float q = (d*sqr(y)+y*ty+1)/a;
    float pql = -p/2;
    float pqr = sqr(p)/4 - q;
    if(pqr < 0) continue;
    SHOW(y);
    pqr = ::sqrt( pqr );
    float x1 = pql + pqr;
    float x2 = pql - pqr;
    if(x1 > x2) std::swap(x1,x2);
    std::fill(&C(x1,y),&C(x2,y),255.0f);
  }
#endif
}

int main(){
  URand rc(0,255);
  URandI r(999);
  ImgQ image(Size(1000,1000),3);
  Canvas c(&image);
  c.translate(-500,-500);
  c.rotate(M_PI/4);
  c.translate(500,500);

  fill_ellipse_test_2(image[0],c.getTransform(), Rect(350,450,300,100) );
  c.linecolor(0,255,0,255);
  c.sym('+',500,500);
  c.sym('+',650,500);
  c.sym('+',500,550);

#if 0  
  c.fillcolor(255,0,0,255);
  c.linecolor(0,255,0,100);
  c.triangle(100,500,400,502,900,500);
  c.triangle(500,100,502,400,500,900);
#endif

  
#if 0
  Time t = Time::now();
  for(int i=0;i<100;++i){
    c.fillcolor(rc,rc,rc,255);
    c.linecolor(rc,rc,rc,255);
    c.triangle(r,r,r,r,r,r);
  }
  t.showAge("time for rendering 100 random triangles");
#endif
#if 0
  Time t = Time::now();
  for(int i=0;i<360;++i){
    c.push();
    c.rotate(i/180.*M_PI);
    c.translate(500,500);
    c.line(-400,0,400,0);
    c.pop();
  }
  t.showAge("time for drawing 360 feathered lines (lines)");
#endif

#if 0
  Time t = Time::now();
  for(int i=0;i<360;++i){
    c.push();
    c.rotate(i/180.*M_PI);
    c.translate(500,500);
    for(int x=-400;x<=400;++x){
      c.point(x,0);
    }
    c.pop();
  }
  t.showAge("time for drawing 360 feathered lines (points)");
#endif


#if 0
  // pixel fill rate ~500 MPIX/sec  
  Time t = Time::now();
  c.linecolor(255,0,0);
  for(int y=0;y<image.getWidth();++y){
    for(int x=0;x<image.getHeight();++x){
      c.point(x,y);
    }
  }
  t.showAge("time for filling a " + str(image.getSize()) + " image");
#endif
  
  SHOW(image.getMinMax());
  show(norm(image));
  
  return 0;
}
