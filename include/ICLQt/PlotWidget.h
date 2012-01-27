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

#include <ICLQt/AbstractPlotWidget.h>

namespace icl{

  
  /// Default AbstractPlotWidget implementation for plotting different kinds of data
  /** \section _TYPES_ Data types
      So far, the PlotWidget can be used as scatter-plot and as series-plot (graph-plot)
      The both types can also be used in parallel for overlayed visualizations
      
      \section _SCATTER_ scatter plots
      The scatter plot engine uses a set of data sets, that can be added successively.
      Each dataset is basically defined by a set of N (x,y) points, where the x- and y-
      coordinates are both taken from a data pointer that is read using a stride value.
      To add scatter data, the method  PlotWidget::addScatterData is provided.
      Scatter data is usually visualized by scattered colored symbols, and it
      can be visualized with- and without connecting lines between the points
      and if the shape that is described by the points is convex, it can also be
      filled automatically.   
      
      \section _SERIES_ function data (or series data)
      In order to draw function graphs, the X-viewport is used in slightly adapted way .
      Basically, a single function data-set is defined by a 'strided' float array of N
      elements. If a data-viewport is given to the inherited AbstractPlotWidget, the
      data X-values (which are not given) are assumed to be equally distributed withing
      the data-viewport's x-range. If no data viewport is provided, the X-range is
      automatically estimated to [0, N-1], which is the data index range.

      <b>Please Note:</b> If both, scatter- and function data is given, but not viewport
      information is available, it is not clear, whether to use the scatter-data X-range
      or the function data index-range. Therefore, visualizing scatter and function 
      data at once is only supported if the data viewport is given.
      
      \section _PERFORMANCE_ Performance and bechmarks
      The data interfaces are hold very low-level. The provide the option copy
      given data shallowly and deeply. However drawing the data needs some 
      viewport calculations. Note, in the widget's right-button context menu,
      you can directly see the rendering time!

      Here are some examples, to get an idea of the performance
      (using a 4-Core Intel(R) Xeon(R) CPU E5530 running at 2.40GHz and an optimized build)
      
      -# scatter data point rendering
         point rendering is internally accellerated since it turned out that the 
         QPainter's drawPoint method works very slow for single points. Instead,
         the points are pre-buffered internally in order to be able to use the 
         QPainter's drawPoints method, which is almost 10 times faster!
         - rendering 100.000 points (almost all visible) -> 12ms
         - zooming into the center of the data -> 7ms
         - zooming to somewhere where no datapoints are -> 5ms
           (please note, that the clipping mechanism, that avoid to draw invisible 
           point works quite well)
         - using a dynamic viewport for 100.000 points (here, all are visible) -> 17ms

      -# scatter plot symbol rendering (here, size 5 crosses)
         - rendering 100.000 crosses (all visible) 30ms
         - zooming into the center of the data -> 12ms
         - zooming to somewhere where no datapoints are -> 5ms
         - additionally connecting all successive points with lines the lines -> 50ms

      -# drawing function/series data
         Here, x-clipping is implemented much more efficient: Due to the linear character
         of the x-coordinates, the visible X-interval of the function can be estimated
         directly. Therefore, only the visible part of the function data is processed 
         and drawn at all.
         - rendering a sine-function with 100.000 entries (usually this does not
           make sense since about 100 Entries become a single point on the screen)
           - lines only -> 17ms 
           - symbols only (dots) -> 17ms
           - lines and dots (redundant) -> 26ms
           - fill only with alpha value -> 65ms (rather slow, but actually much less samples 
             are needed. Using less samples can easily be implemented by increasing the
             data stride)
           - using '+'-symbols instead of dots, lines off -> 26ms
         - using the same function, but a stride value of 100 (optically, there is no difference).
           In this case: with and without symbols, the rendering time is 1msec
           - lines only or in combination with any symbol: 1ms
           - fill only (with alpha value) -> 9ms
           - fill and lines -> 15ms
  */
  class PlotWidget : public virtual AbstractPlotWidget{
    class Data; //!< pimpl
    Data *data; //!< pimpl pointer
    
    class DrawState; //!< internally used
    protected:

    /// draws the ledgend
    virtual void drawLegend(QPainter &p,const Rect &where, bool horizontal);

    /// draws the series data
    virtual bool drawSeriesData(QPainter &p, const DrawState &state); 

    /// draws the sctter data
    virtual bool drawScatterData(QPainter &p, const DrawState &state); 

    /// draws all the data
    virtual bool drawData(QPainter &p); 

    /// estimates the data xrange (for automatic viewport adaption)
    virtual Range32f estimateDataXRange() const;
    
    /// estimates the data yrange (for automatic viewport adaption)
    virtual Range32f estimateDataYRange() const;
    
    public:

    /// constructor
    PlotWidget(QWidget *parent=0);

    /// destructor
    ~PlotWidget();
    
    /// returns the data viewport 
    /** If the data viewport is not given using AbstractPlotWidget::setDataViewPort */
    virtual Rect32f getDataViewPort() const;
    
    /// adds series data
    void addSeriesData(const float *data, int len, 
                       const AbstractPlotWidget::PenPtr &style = new AbstractPlotWidget::Pen(QColor(255,0,0)),
                       const std::string &name="", int stride=1, bool deepCopy=true, bool passOwnerShip=false);

      /// adds a list of symbols 
    /** @param sym symbol type 
        @param xs x-coordinate pointer (using xStride as stride)
        @param ys y-coordinate pointer (using yStride as stride)
        @param num number of symbols (i.e. xs and ys have xStride * num elements
        @param r symbol red component
        @param g symbol green component
        @param b symbol blue component
        @param size symbol size (in screen pixels)
        @param filled defines whether the symbol must be filled (for triangle, circle, rect and diamond only)
        @param xStride stride for xs-data
        @param yStride stride for ys-data
        @param connectingLine draw a line that connects successive symbols
        @param deepCopyData if set to true, xs, and ys are copied deeply
               otherwise, they are just linked (via pointer)
        @param passOwnerShip if set to true, the plot widget will take the ownership
               of the given data
        
        @see AbstractPlotWidget::Pen::Pen for allowed symbols 
    */
    void addScatterData(char symbol, const float *xs, const float *ys, int num, 
                        const std::string &name = "",
                        int r=255, int g=0, int b=0, int size=5, bool filled=false,
                        int xStride=1, int yStride=1, bool connectingLine = false,
                        bool deepCopyData=true, bool passDataOwnerShip=false);
    
    /// clears the symbol draw list
    void clearScatterData();
    
    /// clears the symbol draw list
    void clearSeriesData();
    
    /// clears all contained data
    void clear();
  
  

                 
  };
}

#endif
