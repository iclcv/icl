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

namespace icl{

  class PlotWidget : public LowLevelPlotWidget{
    struct Data;
    Data *m_data;
    
    public:
    PlotWidget(QWidget *parent=0);
    ~PlotWidget();
    
    inline void lock() { LowLevelPlotWidget::lock(); }
    inline void unlock() { LowLevelPlotWidget::unlock(); }

    inline void reset() { clear(); }
    
    void color(int r, int g, int b, int a=255);
    void fill(int r, int g, int b, int a=255);
    
    template<class VectorType>
    inline void color(const VectorType &c){
      color(c[0],c[1],c[2],c[3]);
    }

    template<class VectorType>
    inline void fill(const VectorType &c){
      fill(c[0],c[1],c[2],c[3]);
    }


    void sym(char s);
    
    inline void nocolor(){ color(0,0,0,0); }
    inline void nofill() { fill(0,0,0,0); }
    inline void nosym() { sym(' '); }
    
    void linewidth(float width);
    void symsize(float size);
    void pointsize(float size);
    
    
    template<class T>
    void scatter(const T *xs, const T *ys, int num, int xStride = 1, int yStride=1);

    inline void scatter(const std::vector<Point32f> &ps){
      scatter(&ps[0].x, &ps[0].y, ps.size(), 2, 2);
    }
    
    inline void scatter(const std::vector<Point> &ps){
      scatter(&ps[0].x, &ps[0].y, ps.size(), 2, 2);
    }
    
    template<class T, int WIDTH>
    inline void scatter(const FixedMatrix<T,WIDTH,3-WIDTH> *ps, int num){
      scatter(&ps[0][0],&ps[0][1], num, 2, 2);
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

    /// annotation stuff
    point();
    points();
    line();
    lines();
    linestrip();
    triangle();
    rect();
    text();
    fontsize();
    ellipse();
    circle();
    arrow();
    
    
    
    

  };

};

#endif
