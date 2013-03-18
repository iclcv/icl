/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImageRenderer.cpp                  **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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
#include <ICLCore/ImageRenderer.h>
//#include <ICLCore/SampledLine.h>

using namespace icl::utils;
using namespace icl::math;

namespace icl{
  namespace core{
    
    template<class T>
    void hline(const int color[4], const Img<T> &dst, int y, int x1, int x2){
      const int rgb[] = { color[0], color[1], color[2] }, alpha = color[3], beta = 255 - alpha;
      const int rgb_premultiplied[] = { rgb[0]*alpha, rgb[1]*alpha, rgb[2]*alpha }; 
      
      if(beta){
        for(int c=0;c<dst->getChannels() && c<3;++c){
          Channel<T> ch = dst[c];
          for(int x=x1; x<=x2;++x){
            ch(x,y) = (ch(x,y) * beta + rgb_premultiplied[c])/T(255);
          }
        }
      }else{
        for(int c=0;c<dst->getChannels() && c<3;++c){
          T *begin = &dst(x1,y,c), end = begin + (x2-x1+1);
          std::fill(begin,end,T(rgb[c]));
        }
      }
    }

    template<class T>
    void vline(const int color[4], const Img<T> &dst, int x, int y1, int y2){
      const int rgb[] = { color[0], color[1], color[2] }, alpha = color[3], beta = 255 - alpha;
      const int rgb_premultiplied[] = { rgb[0]*alpha, rgb[1]*alpha, rgb[2]*alpha }; 
      
      if(beta){
        for(int c=0;c<dst->getChannels() && c<3;++c){
          Channel<T> ch = dst[c];
          for(int y=y1; y<=y2;++y){
            ch(x,y) = (ch(x,y) * beta + rgb_premultiplied[c])/T(255);
          }
        }
      }else{
        for(int c=0;c<dst->getChannels() && c<3;++c){
          Channel<T> ch = dst[c];
          for(int y=y1; y<=y2;++y){
            ch(x,y) = rgb[c];
          }
        }
      }
    }

    
    template<class T>
    void line(const int color[4], Img<T> &dst, int x1, int y1, int x2, int y2){
      if(x1 == x2) { vline(color,dst,x1,y1,y2); return; }
      if(y1 == y2) { hline(color,dst,y1,x1,x2); return; }
      
      const int rgb[] = { color[0], color[1], color[2] }, alpha = color[3], beta = 255 - alpha;
      const int rgb_premultiplied[] = { rgb[0]*alpha, rgb[1]*alpha, rgb[2]*alpha }; 
      /*
      SampledLine l(x1,y1,x2,y2,0,0,dst->getWidth(),dst->getHeight());
      
      if(beta){
        for(int c=0;c<dst->getChannels() && c<3;++c){
          Channel<T> ch = dst[c];
          while(l.hasNext()){
            const Point &p = l.next();
            ch(p.x,p.y) = (ch(p.x,p.y) * beta + rgb_premultiplied[c])/T(255);
          }
        }
      }else{
        for(int c=0;c<dst->getChannels() && c<3;++c){
          Channel<T> ch = dst[c];
          while(l.hasNext()){
            const Point &p = l.next();
            ch(p.x,p.y) = rgb[c];
          }
        }
      }
     */
    } 

    template<class T>
    struct set_pix_with_alpha{
      int prem,beta;
      set_pix_with_alpha(int prem, int beta):prem(prem),beta(beta){}
      void operator()(T &t) const{ t = (beta * t + prem)/T(255); }
    };
      
      
    
    template<class T>
    void rect(const int color[4], Img<T> &dst, int x, int y, int w, int h){
      const Rect origROI = dst.getROI();

      const int rgb[] = { color[0], color[1], color[2] }, alpha = color[3], beta = 255 - alpha;
      const int rgb_premultiplied[] = { rgb[0]*alpha, rgb[1]*alpha, rgb[2]*alpha }; 

      const Rect r = Rect(x,y,w,h) & dst.getImageRect();
      dst.setROI(x,y,w,h);
      
      for(int i=0;i<3 && i < dst.getChannels();++i){
        if(beta){
          dst.forEach_C(set_pix_with_alpha<T>(rgb_premultiplied[i],beta),i);
        }else{
          dst.fillChannelROI(i,rgb[i]);
        }
      }

      dst.setROI(origROI);
    }


    /*
    inline bool lessPt(const Point &a, const Point &b){
      return a.y<b.y;
    }


    void triangle_intern(Img32f &image,int x1, int y1, int x2, int y2, int x3, int y3 ){
      // {{{ open
  
      // *  the coordinates of vertices are (A.x,A.y), (B.x,B.y), (C.x,C.y); 
      //we assume that A.y<=B.y<=C.y (you should sort them first)
      // * dx1,dx2,dx3 are deltas used in interpolation
      // * horizline draws horizontal segment with coordinates (S.x,Y), (E.x,Y)
      // * S.x, E.x are left and right x-coordinates of the segment we have to draw
      // * S=A means that S.x=A.x; S.y=A.y; 
      if(FILL[3] != 0){
        if(x1==x2 && y1 == y2){ line(image,x1,y1,x3,y3); return; }
        if(x1==x3 && y1 == y3){ line(image,x1,y1,x2,y2); return; }
        if(x2==x3 && y2 == y3){ line(image,x1,y1,x3,y3); return; }
        
        vector<Point> v(3);
        v[0] = Point(x1,y1);
        v[1] = Point(x2,y2);
        v[2] = Point(x3,y3);
        std::sort(v.begin(),v.end(),lessPt);
        
        Point A = v[0];
        Point B = v[1];
        Point C = v[2];
        
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
          for(;S.y<=B.y;S.y++,E.y++,S.x+=dx2,E.x+=dx1){
            hlinef(image,S.x,E.x,S.y,true);
          }
          E=Point32f(B.x,B.y);
          for(;S.y<=C.y;S.y++,E.y++,S.x+=dx2,E.x+=dx3){
            hlinef(image,S.x,E.x,S.y,true);
          }
        }else {
          for(;S.y<=B.y;S.y++,E.y++,S.x+=dx1,E.x+=dx2){
            hlinef(image,S.x,E.x,S.y,true);
          }
          S=Point32f(B.x,B.y);
          for(;S.y<=C.y;S.y++,E.y++,S.x+=dx3,E.x+=dx2){
            hlinef(image,S.x,E.x,S.y,true);
          }
        }
      }
      if(COLOR[3] != 0){
        line(image,x1,y1,x2,y2);
        line(image,x1,y1,x3,y3);
        line(image,x2,y2,x3,y3);
      }
    }
    */



    template<class T> void generic_line(const int color[4], ImgBase &dst, int x1, int y1, int x2, int y2){
      line(color,(Img<T>&)dst,x1,y1,x2,y2);
    }; 

    template<class T> void generic_rect(const int color[4], ImgBase &dst, int x, int y, int w, int h){
      rect(color,(Img<T>&)dst,x,y,w,h);
    }; 
    
    
    // TODO: fill rect, fill triangle, fill ellipse, ellips-border
    
    struct ImageRenderer::Data{
      ImgBase *image;
      int color[4];
      int fill[4];
      int fontsize;
      int symsize;

      typedef void (*line_method)(const int[3], ImgBase &, int,int,int,int);
      typedef void (*rect_method)(const int[3], ImgBase &, int,int,int,int);

      line_method line_methods[5], line;
      line_method rect_methods[5], rect;
      

      Data(){
#define ICL_INSTANTIATE_DEPTH(D) \
        line_methods[depth##D] = &generic_line<depth##D>; \
        rect_methods[depth##D] = &generic_rect<depth##D>; \
        ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
      }
    };
    
    ImageRenderer::ImageRenderer(ImgBase *image):
    m_data(new Data){
      static const int def_color[] = { 255,0,0,255 };
      static const int def_fill[] = { 0,0,0,0 };
      std::copy(def_color,def_color+4,m_data->color);
      std::copy(def_fill,def_fill+4,m_data->fill);      
      m_data->fontsize = 12;
      m_data->symsize = 9;
      setImage(image);
    }
    
    ImageRenderer::~ImageRenderer(){
      delete m_data;
    }
    
    void ImageRenderer::setImage(ImgBase *image){
      m_data->image = image;
      if(image){
        m_data->line = m_data->line_methods[image->getDepth()];
        m_data->rect = m_data->rect_methods[image->getDepth()];
      }
    }
      
    void ImageRenderer::color(int r, int g, int b, int a){
      int color[] = { r,g,b,a };
      std::copy(color,color+4,m_data->color);
    }
    
    void ImageRenderer::fill(int r, int g, int b, int a){
      int fill[] = { r,g,b,a };
      std::copy(fill,fill+4,m_data->fill);
    }
    
    void ImageRenderer::symsize(float size){
      m_data->symsize = size;
    }
    
    void ImageRenderer::fontsize(float size){
      m_data->fontsize = size;
    }
    
    void ImageRenderer::sym(char sym, int x, int y){
      // draw symbol from lines
    }
    
    void ImageRenderer::rect(int x, int y, int w, int h){
      m_data->rect(m_data->color,*m_data->image, x, y, w, h);
    }
     
    void ImageRenderer::triangle(int x1, int y1, int x2, int y2, 
                                 int x3, int y3){
    
    }

    void ImageRenderer::ellipse(int x, int y, int w, int h){
    
    }

    
    void ImageRenderer::line(int x1, int y1, int x2, int y2){
      ICLASSERT_THROW(m_data->image,ICLException("ImageRenderer::line: no target image given!"));
      m_data->line(m_data->color,*m_data->image, x1, y1, x2, y2);
    }

    void ImageRenderer::linestrip(int n, int *xs, int *ys, 
                                  int xStride, int yStride){
      for(int i=1;i<n;++i){
        m_data->line(m_data->color, *m_data->image,
                     xs[xStride*(i-1)], ys[yStride*(i-1)], 
                     xs[xStride*i], ys[yStride*i]);
      }    
    }
    
    void ImageRenderer::pix(int x1, int x2){
      
    }

    void ImageRenderer::pix(int n, int *xs, int *ys, 
                            int xStride, int yStride){
      
    }
    
    void ImageRenderer::circle(int cx, int cy, int r){
      ellipse(cx-r,cy-r,2*r,2*r);
    }
    
    
    void ImageRenderer::text(int x, int y, const std::string &text){
    
    }


  }
}
