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
#include <ICLUtils/Array2D.h>

namespace icl{
  
  /// Easy to use widget component for 2D data visualization
  /** In contrast to it's parent class icl::LowLevelPlotWidget, the
      icl::PlotWidget interface is allinged with ICL's 2D image
      annotation framework (see icl::ICLDrawWidget).

      \section TYP Visualization Types
      The PlotWidget has two types of content: 
      -# <b>Plotting Data</b> (visualized as scatter-plot: 
         PlotWidget::scatter, or series/function plot:
         PlotWidget::series, or bar-plot: PlotWidget::bars)
      -# <b>Annotation data</b> these are extra primitives
         that are used to show/highligh additional information
         in the data plot. Annotations are always drawn in the
         data coordinate space. 
      
      Only plotting data is used for automatic view-port estimation.
      
      \section GEN General Usage
      The PlotWidget internally uses a synchronized queue of darw
      commands. Draw commands can only added in a thread-safe manner
      if the PlotWidget is locked (see PlotWidget::lock() and 
      PlotWidget::unlock() ). The PlotWidget does not support
      to reference to be visualized data using a shallow/pointer
      copy. Instead, the given data is always copied deeply into
      an internal buffer. By theses means, given data must not
      be kept in memory or synchronized mannually. While
      locked, the following operations for data visualization are
      available:
      - PlotWidget::clear() clears the former draw command queue
      - PlotWidget::label and PlotWidget::nolabel set/unset the
        legend label for the next scatter, series or bar plot
      - PlotWidget::color sets the drawing color. This is usually
        used for symbols as well as for any lines for drawing data
        as well as for annotations.
      - PlotWidget::pen also affects the drawing color, but it
        can be used to a custom pen for line drawing. The pen
        can e.g. be set up to draw dashed lines.
      - PlotWidget::linewidth does also affect the pen, so
        using PlotWidget::pen, will also overwrite the last
        linewidth value.
      - PlotWidget::fill is used to the the color for filled
        areas, such as the bars in a bar-plot, or area between
        the function-graph and the y=0 axis in series plots.
        It also affects the fill color of rect- and ellipse
        annotations.
      - PlotWidget::brush is similar to fill as pen to color. By
        setting a QBrush, special color patterns can be used for
        filling.
      - PlotWidget::sym sets the symbol, that is used for 
        scatter and series plots. Supported symbols are listed
        in the documentation of the AbstractPlotWidget::Pen::Pen
        constructor.
      - PlotWidget::scatter, PlotWidget::series and PlotWidget::bars
        are used to pass Plotting data that is then visualized. For
        each of these functions, that are several convenience methods
        and even template that allow for passing differently shaped
        data directly.
      
      For annotation data, the following basic methods are provided.
      Again, for each of these, several convience methods are also
      available.
      - PlotWidget::line draws a line (using the current color)
      - PlotWidget::rect draws a rect (using the current color and fill)
      - PlotWidget::circle draws a circle (using the current color and fill)
      - PlotWidget::text draws text into the drawing area (using
        the currnet color)
      - PlotWidget::linestrip draws several lines each starting
        where the last line ended. In this case, only N-1 points 
        have to be passed for N lines
      - PlotWidget::grid allows to draw a 2D grid annotation
      
      The methods, PlotWidget::title, PlotWidget::xlabel and 
      PlotWidget::ylabel are just convenience function that use
      the derived Configurable's Configurable::setPropertyValue method
      in a direct manner.
      
      \section _EX_ Examples
      
  */
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
    void linestrip(const float *xs, const float *ys, int num, bool closedLoop=true, int stride = 1);

    void rect(const Point32f &ul, const Point32f &lr);
    void rect(const Rect &r);
    void rect(const Rect32f &r);
    void rect(float x, float y, float w, float h);

    void circle(const Point32f &c, float r);
    void circle(float cx, float cy, float r);
      
    void text(float x, float y, const std::string &text);
    void text(const Point32f &p, const std::string &text);

    void grid(int nX, int nY, const float *xs, const float *ys, int stride=1);
    void grid(const Array2D<Point> &data);
    
    inline void grid(const Array2D<Point32f> &data){
      grid(data.getWidth(), data.getHeight(), &data(0,0).x, &data(0,0).y, 2);
    }
    inline void grid(int nX, int nY, const float *xys){
      grid(nX,nY,xys, xys+1,2);
    }
    inline void grid(int nX, int nY, const Point *ps){
      grid(Array2D<Point>(nX,nY,const_cast<Point*>(ps),false));
    }
    inline void grid(int nX, int nY, const Point32f *ps){
      grid(nX, nY, &ps[0].x, &ps[0].y, 2);
    }
    inline void grid(int nX, int nY, const std::vector<Point32f> &ps){
      grid(nX, nY, &ps[0].x, &ps[0].y, 2);
    }
    inline void grid(int nX, int nY, const std::vector<Point> &ps){
      grid(Array2D<Point>(nX,nY, const_cast<Point*>(ps.data()), false));    
    }
    inline void grid(int nX, int nY, const std::vector<float> &xys){
      grid(nX, nY, xys.data(), xys.data()+1,2);
    }
    inline void grid(int nX, int nY, const std::vector<float> &xs, const std::vector<float> &ys){
      grid(nX, nY, xs.data(), ys.data());
    }

    /// sets the plot title
    void title(const std::string &title){
      setPropertyValue("labels.diagramm",title);
    }
    
    /// sets the x-axis label
    void xlabel(const std::string &xlabel){
      setPropertyValue("labels.x-axis",xlabel);
    }

    /// sets the y-axis label
    void ylabel(const std::string &ylabel){
      setPropertyValue("labels.y-axis",ylabel);
    }
    
  };

};

#endif
