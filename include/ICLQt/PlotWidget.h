/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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

      \section _GUI_ GUI Embedding
      The plot widget is also a component of ICL's icl::GUI framework.
      The component tag is "plot" its parameters are 
      - X viewport min value
      - X viewport max value
      - Y viewport min value
      - Y viewport max value
      - an optional flag "gl". If this is given, the PlotWidget
        is rendered in OpenGL. This <b>can</b> increase the rendering
        performance, however it can also make it slower
      
      Example:
      \code
      // create some data
      std::vector<float> sinData(100);
      for(float i=0;i<100;++i){
        sinData[i] = sin(i/100 * 2*M_PI);
      }

      GUI gui;
      gui << "plot(0,6.283,-1.1,1.1)[@handle=plot@minsize=20x20]" << "!show";

      // extract the handle
      PlotHandle plot = gui["plot"];

      plot->lock();            // lock the draw queue
      plot->color(255,0,0);    // set graph line color
      plot->fill(255,0,0,100); // set graph fill (semi transparent)
      plot->label("sin(x)");   // set the legend label
      plot->series(sinData);   // add data to visualize
      plot->unlock();          // unlock the draw queue
      plot.update();           // post an update event to Qt
      
      \endcode
      
      \section _EX_ More Examples
      *Soon!*
      
  */
  class PlotWidget : public LowLevelPlotWidget{
    struct Data;  //!< internal data structure
    Data *m_data; //!< internal data (pimpl)
    
    public:
    
    /// Constructor with given parent
    /** Usually, the */
    PlotWidget(QWidget *parent=0);
    
    /// Destrutor
    ~PlotWidget();
    
    /// locks the draw queue
    inline void lock() { LowLevelPlotWidget::lock(); }

    /// unlocks the draw queue
    inline void unlock() { LowLevelPlotWidget::unlock(); }

    /// clears the draw queue
    virtual void clear();
 
    /// synonym for clear()
    inline void reset() { clear(); }
    
    /// sets the legend label for the next scatter-, series- or bar-plot
    void label(const std::string &primitiveLabel);
    
    /// calls label("")
    inline void nolabel() { label(""); }
    
    /// sets the line color
    void color(int r, int g, int b, int a=255);

    /// convenience macro for types that provide and index operator []
    /** Please note, that c needs 4 elements at least.  */
    template<class VectorType>
    inline void color(const VectorType &c){
      color(c[0],c[1],c[2],c[3]);
    }
    
    /// sets a fully transparent line color
    inline void nocolor(){ color(0,0,0,0); }

    /// sets the line pen
    /** This overwrites the current setting for linewidth and color. The
        pen can e.g. be used to draw dashed lines. */
    void pen(const QPen &pen);

    /// sets the fill color for series- and bar-plots
    /** The fill color is also used for rect- and circle annotations */
    void fill(int r, int g, int b, int a=255);


    /// convenience macro for types that provide and index operator []
    /** Please note, that c needs 4 elements at least.  */
    template<class VectorType>
    inline void fill(const VectorType &c){
      fill(c[0],c[1],c[2],c[3]);
    }
    /// sets a fully transparent fill color
    inline void nofill() { fill(0,0,0,0); }
    
    /// sets the fill brush
    /** This is more powerful then just setting the fill color using 
        PlotWidget::fill. The brush overwrites the current fill color
        setting. In contrast to PlotWidget::fill, the brush can also
        be set up to fill areas with a given pattern */
    void brush(const QBrush &brush);
    
    /// sets the current symbol
    /** The symbol is used for series and scatter plots */
    void sym(char s);
    
    /// sets the current symbol and the symbol size
    /** The symbol size is always given in pixels */
    inline void sym(char s, int symsize){
      sym(s);
      this->symsize(symsize);
    }
    
    /// resets the symbol to ' ' which means no symbols are shown
    inline void nosym() { sym(' '); }
    
    /// sets the linewidth
    void linewidth(float width);
    
    /// sets the symbols size
    void symsize(float size);
    
    
    /// adds a scatter plot with given x- and y-data pointer
    /** @param xs source pointer for the x-values 
        @param ys source poitner for the y-value 
        @param num number of points
        @param xStride pointer increment to to iterate through the 
               elements of xs (in sizeof(float) units)
        @param yStride pointer increment to to iterate through the 
               elements of ys (in sizeof(float) units)
        @param connect if set to true, successive points are connected
               using a line
    */
    template<class T>
    void scatter(const T *xs, const T *ys, int num, int xStride = 1, int yStride=1, bool connect=false);

    /// adds a scatter plot from given vector of points
    inline void scatter(const std::vector<Point32f> &ps, bool connect=false){
      scatter(&ps[0].x, &ps[0].y, ps.size(), 2, 2, connect);
    }
    
    /// adds a scatter plot from given vector of points
    inline void scatter(const std::vector<Point> &ps, bool connect=false){
      scatter(&ps[0].x, &ps[0].y, ps.size(), 2, 2, connect);
    }

    /// adds a scatter plot from given set of of fixed vectors
    template<class T, int WIDTH>
    inline void scatter(const FixedMatrix<T,WIDTH,3-WIDTH> *ps, int num, bool connect=false){
      scatter(&ps[0][0],&ps[0][1], num, 2, 2, connect);
    }

    /// adds series data 
    /** @param data data pointer
        @param num number of elements
        @param stride data increment to iterate through the data elements
               stride is given in sizeof(float) units */
    template<class T>
    void series(const T *data, int num, int stride=1);
    
    /// adds series data from given std::vector
    template<class T>
    inline void series(const std::vector<T> &data){
      series(data.data(), data.size(), 1);
    }
    

    /// adds bar plot data
    /** @param data data pointer
        @param num number of elements
        @param stride data increment to iterate through the data elements
               stride is given in sizeof(float) units */
    template<class T>
    void bars(const T *data, int num, int stride=1);
    
    /// adds bar plot data from given std::vector
    template<class T>
    inline void bars(const std::vector<T> &data){
      bars(data.data(), data.size(), 1);
    }
    
    /// draws a line annotation using the current color
    void line(const Point32f &a, const Point32f &b);
    
    /// draws a line  annotation given 4 coordinates
    inline void line(float x1, float y1, float x2, float y2){
      line(Point32f(x1,y1),Point32f(x2,y2));
    }

    /// draws a line strip annotation
    void linestrip(const std::vector<Point32f> &ps, bool closedLoop=true);

    /// draws a line strip annotation
    void linestrip(const std::vector<Point> &ps, bool closedLoop=true);

    /// draws a line strip annotation
    void linestrip(const Point32f *ps, int num, bool closedLoop=true);

    /// draws a line strip annotation
    void linestrip(const Point *ps, int num, bool closedLoop=true);

    /// draws a line strip annotation
    void linestrip(const float *xs, const float *ys, int num, bool closedLoop=true, int stride = 1);

    /// draws a rectangle annotation with given upper left and lower right coordinate
    void rect(const Point32f &ul, const Point32f &lr);

    /// draws a rectangle annotation
    void rect(const Rect &r);

    /// draws a rectangle annotation
    void rect(const Rect32f &r);

    /// draws a rectangle annotation
    void rect(float x, float y, float w, float h);

    /// draws a circle annotation
    void circle(const Point32f &c, float r);

    /// draws a circle annotation
    void circle(float cx, float cy, float r);
      
    /// draws a text annotation
    /** Note.: the text anchor is always centered) */
    void text(float x, float y, const std::string &text);

    /// draws a text annotation
    /** Note.: the text anchor is always centered) */
    void text(const Point32f &p, const std::string &text);

    /// draws a grid-annoation
    void grid(int nX, int nY, const float *xs, const float *ys, int stride=1);

    /// draws a grid-annoation
    void grid(const Array2D<Point> &data);
    
    /// draws a grid-annoation
    inline void grid(const Array2D<Point32f> &data){
      grid(data.getWidth(), data.getHeight(), &data(0,0).x, &data(0,0).y, 2);
    }

    /// draws a grid-annoation
    inline void grid(int nX, int nY, const float *xys){
      grid(nX,nY,xys, xys+1,2);
    }

    /// draws a grid-annoation
    inline void grid(int nX, int nY, const Point *ps){
      grid(Array2D<Point>(nX,nY,const_cast<Point*>(ps),false));
    }

    /// draws a grid-annoation
    inline void grid(int nX, int nY, const Point32f *ps){
      grid(nX, nY, &ps[0].x, &ps[0].y, 2);
    }

    /// draws a grid-annoation
    inline void grid(int nX, int nY, const std::vector<Point32f> &ps){
      grid(nX, nY, &ps[0].x, &ps[0].y, 2);
    }

    /// draws a grid-annoation
    inline void grid(int nX, int nY, const std::vector<Point> &ps){
      grid(Array2D<Point>(nX,nY, const_cast<Point*>(ps.data()), false));    
    }

    /// draws a grid-annoation
    inline void grid(int nX, int nY, const std::vector<float> &xys){
      grid(nX, nY, xys.data(), xys.data()+1,2);
    }
    
    /// draws a grid-annoation
    inline void grid(int nX, int nY, const std::vector<float> &xs, const std::vector<float> &ys){
      grid(nX, nY, xs.data(), ys.data());
    }

    /// sets the diagram title
    /** Please note, that usually the property 
        "borders.top" needs to be set to something around 18.
        Otherwise, the tiltle is placed within the drawing area
    */
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
