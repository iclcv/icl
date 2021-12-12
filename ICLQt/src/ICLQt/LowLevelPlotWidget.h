/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/LowLevelPlotWidget.h                   **
** Module : ICLQt                                                  **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/AbstractPlotWidget.h>

namespace icl{
  namespace qt{


    /// Default AbstractPlotWidget implementation for plotting different kinds of data
    /** \section WHY Why Low-Level ?
        ICL's plotting framework does also provide a derived class called icl::PlotWidget.
        We strongly recommend to use the PlotWidget instead of the LowLevelPlotWidget
        be cause the PlotWidget provides a simpler interface and it's usage is
        more similar to the usage of ICL's image annotation framework (see ICLDrawWidget
        and ICLDrawWidget3D). However, in some special situations, the LowLevelPlotWidget
        provides a better performance, since it can e.g. visualize data that is just linked
        into it using a shallow copy. In contrast to the LowLevelPlotWidget, the PlotWidget
        does also provide template methods, that can be used to draw data of all POD types
        directly. In short:
        <b> The LowLevelPlotWidget provides a low level interface for
        data visualization. It's functions are tuned for speed. Usually, the
        derived class icl::PlotWidget is simpler to use. </b>


        \section _TYPES_ Data types
        So far, the LowLevelPlotWidget can be used as scatter plot, bar-plot and as
        series-plot (graph-plot). All types can be used in parallel for overlayed
        visualization.
        \image html plot_widget_h.png

        \section _SCATTER_ scatter plots
        The scatter plot engine uses a set of data sets, that can be added successively.
        Each dataset is basically defined by a set of N (x,y) points, where the x- and y-
        coordinates are both taken from a data pointer that is read using a stride value.
        To add scatter data, the method  LowLevelPlotWidget::addScatterData is provided.
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

        \section _BAR_PLOTS_
        Basically, bar plots are quite similar to function data/series plots. The viewport
        adaptions is managed indentically here. If several bar plots are used in one graph,
        the bars of each bin become smaller and the different bins are shown in an interleaved
        manner. As for function data, the longest bar plot row is used to determine
        the data x-range if no x-range is provided explicitly.

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

        \section _EX_ Examples
        Please note, that the example application can be found at ICLQt/examples/plot-component-demo.cpp
        the binary name is icl-plot-component-demo.


        \subsection _EX_1_ Series Data 1
        \code
        LowLevelPlotWidget pw;
        static std::vector<float> sinData(100); // fill it
        pw.setPropertyValue("tics.y-distance",0.25);
        pw.setPropertyValue("enable fill",true);
        pw.addSeriesData(sinData.data(), sinData.size(),
                         new AbstractPlotWidget::Pen(QColor(255,0,0),Qt::NoPen,' ',5, QColor(255,0,0,100)),
                         "sin(x)");
        \endcode
        \image html plot_widget_a.png


        \subsection _EX_2_ Series Data 2 (no fill)
        \code
        static std::vector<float> tanData(100); // fill it!
        pw.setPropertyValue("tics.y-distance",0.25);
        pw.addSeriesData(tanData.data(), tanData.size(),
                         new AbstractPlotWidget::Pen(QColor(0,100,255),Qt::NoPen,' ',2, QColor(0,100,255,100)),
                         "tan(x)");
        \endcode
        \image html plot_widget_b.png


        \subsection _EX_3_ Series Data 3 (symbols)
        \code
        static std::vector<float> cosData(100); // fill it!
        pw.setPropertyValue("tics.y-distance",0.25);
        // we use symbols of radius 2, the 'o' selects circles
        pw.addSeriesData(cosData.data(), cosData.size(),
                         new AbstractPlotWidget::Pen(QColor(0,255,0),QColor(0,255,0),'o',2, QColor(0,255,0,100)),
                         "cos(x)");
        \endcode
        \image html plot_widget_e.png


        \subsection _EX_4_ Series Data 4 (multiple functions)
        \code
        // use sin, cos and tan data from above
        pw.setPropertyValue("tics.y-distance",0.25);
        pw.addSeriesData(sinData.data(), sinData.size(),
                         new AbstractPlotWidget::Pen(QColor(255,0,0)),
                         "sin(x)");
        pw.addSeriesData(cosData.data(), cosData.size(),
                         new AbstractPlotWidget::Pen(QColor(0,255,0)),
                         "cos(x)");
        pw.addSeriesData(tanData.data(), tanData.size(),
                         new AbstractPlotWidget::Pen(QColor(0,100,255)),
                         "tan(x)");
        \endcode
        \image html plot_widget_f.png


        \subsection _EX_5_ Scatter Data (one or two data sets)
        \code
        /// interleaved data
        static std::vector<utils::Point32f> scatterData1(10000); // fill it!

        /// planar data order
        static std::vector<float> scatterData2(20000); // fill it!

        pw.setPropertyValue("tics.x-distance",3);
        pw.setPropertyValue("tics.y-distance",3);
        /// for interleaved data, the x- and y-stride is 2
        /// the data is static, so we use shallow copy instead of deep copy
        pw.addScatterData('.',&scatterData1[0].x,&scatterData1[0].y,scatterData1.size(),
                             "some noise", 255, 0, 0, 2, false, 2, 2, false, false, false);

        /// for the planar data, the x- and y-stride is 1
        pw.addScatterData('.',&scatterData2[0],&scatterData2[0]+10000,scatterData2.size()/2,
                             "some other noise", 0, 100, 255, 2, false, 1, 1, false, false, false);

        \endcode
        \image html plot_widget_c.png



        \subsection _EX_6_ Connected Scatter Data and annotations
        \code
        static std::vector<utils::Point32f> scatterData3(1000);
        /// here, we fill it!
        static utils::Time t = utils::Time::now();
        float dtSec = (utils::Time::now()-t).toSecondsDouble();
        for(unsigned int i=0;i<scatterData3.size();++i){
          float rel = float(i)/(scatterData3.size()-1) * 2 * M_PI;
          float r = 2 + 3 * sin(4*rel+dtSec);
          scatterData3[i].x  = r * cos(rel);
          scatterData3[i].y  = r * sin(rel);
        }
        // again, we use stride values of two for the interleaved data
        // this time, connectLines is true
        pw.addScatterData('.',&scatterData3[0].x,&scatterData3[0].y,scatterData3.size(),
                             "some shape", 0, 100, 255, 2, true, 2, 2, true, false, false);

        /// further more we add some annotations
        pw.addAnnotations('r',FixedMatrix<float,1,4>(-.2,-.2,.4,.4).data() ,1,QColor(255,0,0), QColor(255,0,0,100));
        pw.addAnnotations('l',FixedMatrix<float,1,4>(0,0,3,3).data(),1,QColor(255,0,0));
        pw.addAnnotations('t',FixedMatrix<float,1,2>(3.f,3.f).data(),1,QColor(255,0,0),Qt::NoBrush,"the center");

        \endcode
        \image html plot_widget_g.png
    */
    class ICLQt_API LowLevelPlotWidget : public virtual AbstractPlotWidget{
      class Data; //!< pimpl
      Data *data; //!< pimpl pointer

      class DrawState; //!< internally used
      protected:

      /// draws the ledgend
      virtual void drawLegend(QPainter &p,const utils::Rect &where, bool horizontal);

      /// draws the series data
      virtual bool drawSeriesData(QPainter &p, const DrawState &state);

      /// draws the sctter data
      virtual bool drawScatterData(QPainter &p, const DrawState &state);

      /// draws the bar plot data
      virtual bool drawBarPlotData(QPainter &p, const DrawState &state);

      /// draws all the data
      virtual bool drawData(QPainter &p);

      /// estimates the data xrange (for automatic viewport adaption)
      virtual utils::Range32f estimateDataXRange() const;

      /// estimates the data yrange (for automatic viewport adaption)
      virtual utils::Range32f estimateDataYRange() const;

      public:

      /// constructor
      LowLevelPlotWidget(QWidget *parent=0);

      /// destructor
      ~LowLevelPlotWidget();

      /// returns the data viewport
      /** If the data viewport is not given using AbstractPlotWidget::setDataViewPort */
      virtual utils::Rect32f getDataViewPort() const;

      /// adds series data
      /** @param data data pointer
          @param len number of elements in the data pointer
          @param style draw style
          @param name
	  @param stride data stride (in units of sizeof(float))
          @param deepCopy if true, the data is copied into the drawer, otherwise, it is just
                 linked. If the data is linked, it must remain valid until the seriesData is
                 removed using LowLevelPlotWidget::clearSeriesData or LowLevelPlotWidget::clear.
          @param passOwnerShip This flag is only used if deepCopy is false. If passOwnerShip
                 is true, the LowLevelPlotWidget will delete the data at deletion or when LowLevelPlotWidget::clearSeriesData or LowLevelPlotWidget::clear
                 is called.
          */
      void addSeriesData(const float *data, int len,
                         const AbstractPlotWidget::PenPtr &style = new AbstractPlotWidget::Pen(QColor(255,0,0)),
                         const std::string &name="", int stride=1, bool deepCopy=true, bool passOwnerShip=false);


      /// adds data for a bar plots
      /** Bar plots basically work like function/series data except for the fact, that
          bars are drawn instead of a (filled) function graph line
          @see addSeriesData */
      void addBarPlotData(const float *data, int len,
                          const AbstractPlotWidget::PenPtr &style = new AbstractPlotWidget::Pen(QColor(255,0,0)),
                          const std::string &name="", int stride=1, bool deepCopy=true, bool passOwnerShip=false);

        /// adds a list of symbols
      /** @param symbol symbol type
          @param xs x-coordinate pointer (using xStride as stride)
          @param ys y-coordinate pointer (using yStride as stride)
          @param num number of symbols (i.e. xs and ys have xStride * num elements
          @param name
          @param r symbol red component
          @param g symbol green component
          @param b symbol blue component
          @param size symbol size (in screen pixels)
          @param connectingLine draw a line that connects successive symbols
          @param xStride stride for xs-data (in units of sizeof(float), i.e. if the underlying data is
                 interleaved (x,y)-data, the strides are 2 )
          @param yStride stride for ys-data (in units of sizeof(float))
          @param filled defines whether the symbol must be filled (for triangle, circle, rect and diamond only)
          @param deepCopyData if set to true, xs, and ys are copied deeply
                 otherwise, they are just linked (via pointer)
          @param passDataOwnerShip if set to true, the plot widget will take the ownership
                 of the given data

          @see AbstractPlotWidget::Pen::Pen for allowed symbols
      */
      void addScatterData(char symbol, const float *xs, const float *ys, int num,
                          const std::string &name = "",
                          int r=255, int g=0, int b=0, int size=5, bool connectingLine=false,
                          int xStride=1, int yStride=1, bool filled = false,
                          bool deepCopyData=true, bool passDataOwnerShip=false);

      /// clears the scatter data draw list
      void clearScatterData();

      /// clears series data draw list
      void clearSeriesData();

      /// clears the bar plot draw list
      void clearBarPlotData();

      /// clears all contained data
      void clear();




    };
  } // namespace qt
}
