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
#include <ICLUtils/Rect32f.h>


namespace icl{

  using namespace math;
  using namespace utils;

  namespace core{
    

    
    class AbstractCanvas : public utils::Uncopyable{
      public:
      typedef math::FixedMatrix<float,3,3> Transform;
      typedef core::Color4D32f Color;
      struct ClipRect{
        ClipRect(float minx=0, float maxx=0, float miny=0, float maxy=0): 
          minx(minx),maxx(maxx),miny(miny),maxy(maxy){}
        float minx;
        float maxx;
        float miny;
        float maxy;
        bool in(float x, float y) const{
          return ((x >= minx) && (x<= maxx) 
                  && (y >= miny) && (y <= maxy));
        }
        bool in(const Point32f &p) const{
          return in(p.x,p.y);
        }
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
          fontsize(10),fontname("arial"){}
      };
      
      State state;
      std::vector<State> stack;
  
      inline utils::Point32f transform(float x, float y) const{
        return Point32f(state.T(0,0)*x + state.T(1,0)*y + state.T(2,0),
                        state.T(0,1)*x + state.T(1,1)*y + state.T(2,1));
      }
      inline bool clip(float x, float y) const{
        return state.clip.in(x,y);
      }
      inline bool clip(const Point32f &p) const{
        return state.clip.in(p);
      }
                        
      
      public:
      
      AbstractCanvas(){}
      virtual ~AbstractCanvas(){}
      
      virtual void draw_point_internal(const utils::Point32f &p) = 0;
      virtual void draw_line_internal(const utils::Point32f &a, 
                                      const utils::Point32f &b) = 0;
      virtual void fill_triangle_internal(const utils::Point32f &a, 
                                          const utils::Point32f &b,
                                          const utils::Point32f &c) = 0;
      virtual void draw_ellipse_internal(const utils::Point32f &c,
                                         const utils::Point32f &axis1,
                                         const utils::Point32f &axis2) = 0;
      virtual void draw_image_internal(const utils::Point32f &ul, 
                                       const utils::Point32f &ur, 
                                       const utils::Point32f &lr, 
                                       const utils::Point32f &ll,
                                       float alpha, scalemode sm) = 0;
      
      public:

      virtual const Transform &getTransform() const{
        return state.T;
      }
      virtual void getTransform(float &angle, float &tx, float &ty) const{
        angle = acos(state.T(0,0));
        tx = state.T(2,0);
        ty = state.T(2,1);
      }

      
      virtual Rect32f getClipRect(){
        return Rect32f(state.clip.minx, state.clip.miny,
                       state.clip.maxx-state.clip.minx,
                       state.clip.maxy-state.clip.miny);
      }
      
      virtual void push(){
        stack.push_back(state);
      }
      
      virtual void pop() throw (utils::ICLException){
        ICLASSERT_THROW(stack.size(), utils::ICLException("AbstractCanvas::pop: stack is empty"));
        state = stack.back();
        stack.pop_back();
      }
      
      virtual void point(float x, float y);
      virtual void line(float x0, float y0, float x1, float y1);
      virtual void triangle(float x0, float y0, float x1, float y1, float x2, float y2);
      virtual void sym(char c, float x, float y) throw (utils::ICLException);
      virtual void linestrip(int n, const float *xs, const float *ys, 
                             int xStride=1, int yStride=1, bool loop=false);
      virtual void rect(float x, float y, float w, float h);
      virtual void circle(float cx, float cy, float r);
      virtual void ellipse(float cx, float cy, float rx, float ry);
      virtual void image(const ImgBase *image, float xanchor, 
                         float yanchor, float alpha, 
                         scalemode sm=interpolateLIN, 
                         bool centered=false)  throw (utils::ICLException);
      virtual void text(const std::string &t, float x, float y, bool centered = false);
      
      virtual void linecolor(float r, float g, float b, float a=255){
        state.linecolor = Color(r,g,b,a);
      }

      virtual void fillcolor(float r, float g, float b, float a=255){
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
          const float sa = sin(angle), ca = cos(angle);
          state.T = Transform(ca,sa,tx,
                              -sa,ca,ty,
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
