/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/AbstractCanvas.h                   **
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

#pragma once

#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLMath/FixedMatrix.h>
#include <ICLCore/Color.h>
#include <ICLCore/Rect32f.h>


namespace icl{
  namespace core{
    
    class AbstractCanvas : public utils::Uncopyable{
      public:
      typedef math::FixedMatrix<float,3,3> Transform;
      typedef core::Color4D32f Color;
      struct ClipRect{
        ClipRect(float minx=0, float maxx=0, float miny=0, float maxy=0): 
          minx(minx),maxy(maxx),miny(miny),maxy(maxy){}
        float minx;
        float miny;
        float maxx;
        float maxy;
      };
      
      protected:
      struct State{
        AbstractCanvas::Transform T;
        AbstractCanvas::Color linecolor;
        AbstractCanvas::Color fillcolor;
        float linewidth;
        float pointsize;
        float symsize;
        float fontsize;
        std::string fontname;
        ClipRect clip;
        
        inline State():
          T(AbstractCanvas::Transform::id()),
          linecolor(255,0,0,255),fillcolor(0,0,0,0),
          linewidth(1),pointsize(1),symsize(10),
          fontsize(10),fontname("arial"),
          antialiasing(false){}
      };
      
      State state;
      std::vector<State> stack;
  
      inline Point32f transform(float x, float y) const{
        return Point32f(state.T(0,0)*x + state.T(1,0)*y + state.T(2,0),
                        state.T(0,1)*x + state.T(1,1)*y + state.T(2,1));
      }
      inline bool clip(float x, float y) const{
        return ((x >= state.clip.minx) && (x<= state.clip.maxx) 
                && (y >= state.clip.miny) && (y <= state.clip.maxy));
      }
                        
      
      public:
      
      AbstractCanvas(){}
      virtual ~AbstractCanvas(){}
      
      virtual void draw_point_internal(const Point32f &p) = 0;
      virtual void draw_line_internal(const Point32f &a, const Point32f &b) = 0;
      virtual void fill_triangle_internal(const Point32f &a, const Point32f &b, const Point32f &c) = 0;
      virtual void draw_ellipse_internal(const Point32f &c,  const Point32f &axis1, const Point32f &axis2);
      
      public:

      virtual void push(){
        stack.push_back(state);
      }
      virtual void pop() throw (utils::ICLException){
        ICLASSERT_THROW(stack.size(), ICLException("AbstractCanvas::pop: stack is empty"));
        state = stack.back();
        stack.pop_back();
      }
      
      virtual void point(float x, float y){
        draw_point_internal(transform(x,y));
      }
      virtual void line(float x0, float y0, float x1, float y1){
        draw_line_internal(transform(x0,y0),transform(x1,y1));
      }
      virtual void triangle(float x0, float y0, float x1, float y1, float x2, float y2){
        const Point32f a = transform(x0,y0);
        const Point32f b = transform(x1,y1);
        const Point32f c = transform(x2,y2);
        fill_triangle_internal(a,b,c);
        draw_line_internal(a,b);
        draw_line_internal(b,c);
        draw_line_internal(c,a);
      }
      
      virtual void sym(char c, float x, float y){
        // draw lines out of symbols (regarding size)
        // symbols are o . x + * t(riangle) s(quare)
      }
      
      virtual void linestrip(int n, const float *xs, const float *ys, int xStride=1, int yStride=1, bool loop=false){
        if(n<2) return;
        Point32f a = transform(xs[0],ys[0]),b;
        for(int i=1;i<n;++i){
          b = transform(xs[i*xStride],ys[i*yStride]);
          draw_line_internal(a,b);
          a = b;
        }
        if(loop){
          draw_line_internal(a,transform(xs[0],ys[0]));          
        }
      }
      
      virtual void rect(float x, float y, float w, float h){
        const Point32f ul = transform(x,y);
        const Point32f ur = transform(x+w,y);
        const Point32f lr = transform(x+w,y+h);
        const Point32f ll = transform(x,y+h);
        fill_triangle_internal(ul,ur,lr);
        fill_triangle_internal(ul,lr,ll);
        draw_line_internal(ul,ur);
        draw_line_internal(ur,lr);
        draw_line_internal(lr,ll);
        draw_line_internal(ll,ul);
      }
      virtual void circle(float cx, float cy, float r){
        // TODO
      }
      virtual void ellipse(float x, float x, float w, float h){
        // TODO: virtual void draw_ellipse_internal(const Point32f &c,  const Point32f &axis1, const Point32f &axis2);
        
      }
      
      virtual void image(const ImgBase *image, float xanchor, float yanchor){
        // estimate the four corners and use rectification to perform sampling
      }

      virtual void text(const std::string &t, float x, float y){
        // fallback: use image and a hardcoded font (including hardcoded size)
      }
            
      
      virtual void linecolor(float r, float g, float b, float a){
        state.linecolor = Color(r,g,b,a);
      }

      virtual void fillcolor(float r, float g, float b, float a){
        state.fillcolor = Color(r,g,b,a);
      }
      
      virtual void linewidth(float width){
        state.linewidth = width;
      }

      virtual void pointsize(float size){
        state.pointsize = size;
      }

      virtual void symsize(float size){
        state.symsize = size;
      }
      
      virtual void fontsize(float size){
        state.fontsize = size;
      }
      
      virtual void fontname(const std::string &fontname){
        state.fontname = fontname;
      }

      virtual void transform(const Transform &T){
        state.T = T*state.T;
      }
      
      virtual void transform(float tx, float ty, float angle){
        if(angle){
          state.T = Transform(sa,ca,tx,
                              -ca,sa,ty,
                              0,0,1)*state.T;
        }else{
          state.T(2,0) += tx;
          state.T(2,1) += ty;
        }
      }
      
      /// resets the transform
      virtual void reset(){
        state.T = Transform::id();
      }
      
      virtual void rotate(float angle){
        transform(0,0,angle);
      }
      virtual void translate(float tx, float ty){
        transform(tx,ty,0);
      }
      virtual void scale(float s){
        scale(s,s);
      }

      virtual void scale(float sx, float sy){
        state.T = Transform(sx,0,0,
                            0,sy,0,
                            0,0,1) * state.T;
      }
    };
    
  }
}
