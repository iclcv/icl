/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/AbstractCanvas.cpp                 **
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

#include <ICLCore/AbstractCanvas.h>
#include <ICLUtils/StringUtils.h>

namespace icl{
  namespace core{



    void AbstractCanvas::point(float x, float y){
      if(state.pointsize == 1){
        draw_point_internal(transform(x,y));
      }else{
        push();
        state.fillcolor = state.linecolor;
        state.linecolor = AbstractCanvas::Color(0,0,0,0);
        circle(x,y,state.pointsize);
        pop();
      }
    }

    void AbstractCanvas::line(float x0, float y0, float x1, float y1){
      draw_line_internal(transform(x0,y0),transform(x1,y1));
    }
    void AbstractCanvas::triangle(float x0, float y0, float x1, float y1, float x2, float y2){
      const Point32f a = transform(x0,y0);
      const Point32f b = transform(x1,y1);
      const Point32f c = transform(x2,y2);
      fill_triangle_internal(a,b,c);
      draw_line_internal(a,b);
      draw_line_internal(b,c);
      draw_line_internal(c,a);
    }

    void AbstractCanvas::sym(char c, float x, float y) {
      const float s = state.symsize;
      switch(c){
        case 'x':
          line(x-s,y-s,x+s,y+s);
          line(x-s,y+s,x+s,y-s);
          break;
        case '+':
          line(x,y-s,x,y+s);
          line(x-s,y,x+s,y);
          break;
        case '*':
          sym('x',x,y);
          sym('+',x,y);
          break;
        case 'o':
          // TODO: temporarily deactivate fill
          push();
          fillcolor(0,0,0,0);
          circle(x,y,s);
          pop();
          break;
        case '.':
          point(x,y);
          break;
        case 't':
          triangle(x,y-s, x+s, y+s, x-s, y+s);
          break;
        case 's':
          rect(x-s,y-s,2*s,2*s);
          break;
        default:
          throw ICLException("undefined symbol type: " + str(c));
      }
    }

    void AbstractCanvas::linestrip(int n, const float *xs, const float *ys,
                                   int xStride, int yStride, bool loop){
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

    void AbstractCanvas::rect(float x, float y, float w, float h){
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

    void AbstractCanvas::circle(float cx, float cy, float r){
      ellipse(cx-r,cy-r,2*r,2*r);
    }

    void AbstractCanvas::ellipse(float cx, float cy, float rx, float ry){
      const Point32f c = transform(cx,cy);
      const Point32f h = transform(cx+rx,cy) - c;
      const Point32f v = transform(cx,cy+ry) - c;
      draw_ellipse_internal(c,h,v);
    }

    void AbstractCanvas::image(const ImgBase *image, float xanchor, float yanchor, float alpha,
                               scalemode sm, bool centered) {
      ICLASSERT_THROW(image,ICLException("AbstractCanvas::image: input image is null"));

      if(centered){
        const float x = xanchor, y=yanchor, w=image->getWidth()/2., h=image->getHeight()/2.;
        const Point32f ul = transform(x-w,y-h);
        const Point32f ur = transform(x+w,y-h);
        const Point32f lr = transform(x+w,y+h);
        const Point32f ll = transform(x-w,y+h);
        draw_image_internal(ul,ur,lr,ll,alpha,sm);

      }else {
        const float x = xanchor, y=yanchor, w=image->getWidth(), h=image->getHeight();
        const Point32f ul = transform(x,y);
        const Point32f ur = transform(x+w,y);
        const Point32f lr = transform(x+w,y+h);
        const Point32f ll = transform(x,y+h);
        draw_image_internal(ul,ur,lr,ll,alpha,sm);
      }
    }

    void AbstractCanvas::text(const std::string &t, float x, float y, bool centered){
      ERROR_LOG("AbstractCanvas::text: text rendering is not implemented yet");
      //        draw_text_internal(t, transform(x,y),
      // fallback: use image and a hardcoded font (including hardcoded size)
      // we need some kind of a font metrics ...
    }



  }
}
