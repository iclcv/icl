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

struct Canvas : public AbstractCanvas{
  typedef void (*point_func)(const Point32f&, void **, int, const AbstractCanvas::Color&);
  typedef void (*line_func)(LineSampler &ls, const Point32f&, const Point32f&, 
                            void **, int, const AbstractCanvas::Color&);
  LineSampler ls;
  void *data[4];
  Size size;
  depth d;
  int c;
  static const float ALPHA_SCALE = 0.00392156862745098039; // 1./255
  
  point_func f_point_no_alpha;
  point_func f_point_with_alpha;
  line_func f_line_no_alpha;
  line_func f_line_with_alpha;
  
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

  template<class T, int CHAN>
  void link_pointers(){
    f_point_no_alpha = &point_template<CHAN,T,false>;                 
    f_point_with_alpha = &point_template<CHAN,T,true>;       
    f_line_no_alpha = &line_template<CHAN,T,false>;          
    f_line_with_alpha = &line_template<CHAN,T,true>;         
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
    this->f_point_no_alpha = 0;
    this->f_point_with_alpha = 0;

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
  
  virtual void draw_point_internal(const utils::Point32f &p){
    if(clip(p)){
      if(state.linecolor[3] == 255){
        f_point_no_alpha(p,data,size.width,state.linecolor);
      }else{
        f_point_with_alpha(p,data,size.width,state.linecolor);
      }
    }
  }

  virtual void draw_line_internal(const utils::Point32f &a, 
                                  const utils::Point32f &b){
    ls.setBoundingRect(getClipRect());
    SHOW(getClipRect());
    if(state.linecolor[3] == 255){
      f_line_no_alpha(ls, a, b, data,size.width,state.linecolor);
    }else{
      f_line_with_alpha(ls, a, b, data,size.width,state.linecolor);
    }
  }
  virtual void fill_triangle_internal(const utils::Point32f &a, 
                                      const utils::Point32f &b,
                                      const utils::Point32f &c){
  
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


int main(){
  ImgQ image(Size(1000,1000),3);
  Canvas c(&image);


  Time t = Time::now();
  for(int i=0;i<360;++i){
    c.push();
    c.rotate(i/180.*M_PI);
    c.translate(500,500);
    c.line(-400,0,400,0);
    c.pop();
  }
  t.showAge("time for drawing 360 feathered lines (lines)");


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
  show(image);
  
  return 0;
}
