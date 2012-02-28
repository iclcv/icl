/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/PlotWidget.h                             **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_PLOT_WIDGET_H
#define ICL_PLOT_WIDGET_H

#include <ICLQt/LowLevelPlotWidget.h>
#include <ICLUtils/FixedMatrix.h>

namespace icl{

  class PlotWidget : public LowLevelPlotWidget{
    struct Data;
    Data *m_data;
    
    public:
    PlotWidget(QWidget *parent=0);
    ~PlotWidget();
    
    inline void lock() { LowLevelPlotWidget::lock(); }
    inline void unlock() { LowLevelPlotWidget::unlock(); }


    virtual void clear();
 
    inline void reset() { clear(); }
    
    void label(const std::string &primitiveLabel);
    inline void nolabel() { label(""); }
    
    void color(int r, int g, int b, int a=255);
    void pen(const QPen &pen);

    void fill(int r, int g, int b, int a=255);
    void brush(const QBrush &brush);
    
    template<class VectorType>
    inline void color(const VectorType &c){
      color(c[0],c[1],c[2],c[3]);
    }

    template<class VectorType>
    inline void fill(const VectorType &c){
      fill(c[0],c[1],c[2],c[3]);
    }


    void sym(char s);
    inline void sym(char s, int symsize){
      sym(s);
      this->symsize(symsize);
    }
    
    inline void nocolor(){ color(0,0,0,0); }
    inline void nofill() { fill(0,0,0,0); }
    inline void nosym() { sym(' '); }
    
    void linewidth(float width);
    void symsize(float size);
    
    template<class T>
    void scatter(const T *xs, const T *ys, int num, int xStride = 1, int yStride=1, bool connect=false);

    inline void scatter(const std::vector<Point32f> &ps, bool connect=false){
      scatter(&ps[0].x, &ps[0].y, ps.size(), 2, 2, connect);
    }
    
    inline void scatter(const std::vector<Point> &ps, bool connect=false){
      scatter(&ps[0].x, &ps[0].y, ps.size(), 2, 2, connect);
    }
    
    template<class T, int WIDTH>
    inline void scatter(const FixedMatrix<T,WIDTH,3-WIDTH> *ps, int num, bool connect=false){
      scatter(&ps[0][0],&ps[0][1], num, 2, 2, connect);
    }

    
    template<class T>
    void series(const T *data, int num, int stride=1);
    
    template<class T>
    inline void series(const std::vector<T> &data){
      series(data.data(), data.size(), 1);
    }
    

    /// what about bar labels ?
    template<class T>
    void bars(const T *data, int num, int stride=1);
    
    template<class T>
    inline void bars(const std::vector<T> &data){
      bars(data.data(), data.size(), 1);
    }
    
    /// draws a line using the current color
    void line(const Point32f &a, const Point32f &b);
    
    inline void line(float x1, float y1, float x2, float y2){
      line(Point32f(x1,y1),Point32f(x2,y2));
    }

    void linestrip(const std::vector<Point32f> &ps, bool closedLoop=true);
    void linestrip(const std::vector<Point> &ps, bool closedLoop=true);
    void linestrip(const Point32f *ps, int num, bool closedLoop=true);
    void linestrip(const Point *ps, int num, bool closedLoop=true);
    void linestrip(const float *xs, const float *ys, int num, bool closedLoop=true);

    void rect(const Point32f &ul, const Point32f &lr);
    void rect(const Rect &r);
    void rect(const Rect32f &r);
    void rect(float x, float y, float w, float h);

    void circle(const Point32f &c, float r);
    void circle(float cx, float cy, float r);
      
    void text(float x, float y, const std::string &text);
    void text(const Point32f &p, const std::string &text);
  };

};

#endif
