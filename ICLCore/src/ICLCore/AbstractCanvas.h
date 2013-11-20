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

      protected:
      struct State{
        Transform T;
        Color linecolor;
        Color fillcolor;
        float linewidth;
        float pointsize;
        float symsize;
        float fontsize;
        std::string fontname;
        const Rect32f clip;
      };
      
      State state;
      std::vector<State> stack;
      
      bool antialiasing;
      Rect32f viewport;

      inline Point32f transform(float x, float y){
        return Point32f(stack.T(0,0)*x + stack.T(1,0)*y + stack.T(2,0),
                        stack.T(0,1)*x + stack.T(1,1)*y + stack.T(2,1));
      }
                        
      
      public:
      
      AbstractCanvas():T(Transform::id()),
        linecolor(255,0,0,255),fillcolor(0,0,0,0),
        linewidth(1),pointsize(1),symsize(10),
      antialiasing(false),fontname("arial"){}

      virtual ~AbstractCanvas(){}
      
      public:

      virtual void push(){
        stack.push_back(state);
      }
      virtual void pop() throw (utils::ICLException){
        ICLASSERT_THROW(stack.size(), ICLException("AbstractCanvas::pop: stack is empty"));
        state = stack.back();
        stack.pop_back();
      }
      
      virtual void point(float x, float y) = 0;
      virtual void line(float x0, float y0, float x1, float y1)  = 0;
      virtual void triangle(float x0, float y0, float x1, float y1, float x2, float y2) = 0;
      virtual void sym(char c, float x, float y) = 0;
      virtual void linestrip(int n, const float *xs, const float *ys, int xStride=1, int yStride=1) = 0;
      virtual void rect(float x, float y, float w, float h) = 0;
      virtual void circle(float cx, float cy, float r) = 0;
      virtual void ellipse(float x, float x, float w, float h) = 0;
      virtual void image(const ImgBase *image, float xanchor, float yanchor) = 0;
      virtual void text(const std::string &t, float x, float y) = 0;
            
      
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
