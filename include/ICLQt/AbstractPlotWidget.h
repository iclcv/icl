/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/AbstractPlotWidget.h                     **
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

#ifndef ICL_ABSTRACT_PLOT_WIDGET_H
#define ICL_ABSTRACT_PLOT_WIDGET_H

#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Configurable.h>

#include <ICLQt/ThreadedUpdatableWidget.h>

#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtGui/QPainter>

namespace icl{
  
  
  /// The PlotWidget is an abstract base class for 2D plotting components
  /** \section _NONENCLATURE_ Nomenclature 

      - <b>pen</b> like in Qt a pen is used to draw primitives. We use
        QPens directly to provide everything, that Qt provides here such
        as line color, line width and also dash-patterns
      
      \section _WHERE_ where is the data
      Since the PlotWidget is just an abstract interface, it does not define
      how the actual data is shaped. Use the derived classes LowLevelPlotWidget
      or PlotWidget to visualize your data. We strongly recommend to use
      the (high level) PlotWidget class, whose interface is designed in ICL
      manner.

      \section _FEATURES_ Features
      The Abstract PlotWidget provides setting, visualizing and navigating
      within a data viewport. The data viewport can be set explicitly --
      in this case, data that is outside the viewport is clipped -- or not.
      If not data viewport is given, it is estimated before every drawing 
      step dynamically.
      The created Widged shows a coordinate frame, tics, a grid, and tic
      labels. All of these features can be adapted and deactivated
      dynamically. Furthermore, the uses can use the left mouse button to 
      zoom the viewport to a specific sub-rect of the current viewport. 
      The subrect is intuitively defined by holding the left-mouse-button.
      Pressing the left mousebutton zooms back to the former viewport. 

      \section _ANNOTATIONS_ Annotations
      In addition to the abstract data visualization interface, the AbstractPlotWidget
      provides the AbstractPlotWidget::addAnnotations method. This can be used
      to add annotations (such as rectangles, circles, lines or texts) to the
      data viewport.
      The annotaiton iterface is hold very low-level. Perhaps extending classes
      provide simpler interfaces.
  */
  class AbstractPlotWidget : public ThreadedUpdatableWidget, public Configurable{
    struct Data; //!< pimpl
    Data *data;  //!< pimpl pointer

    /// internally used
    void property_changed(const Property &);

    public:
    
    /// different pen types
    enum PenType{
      X_AXIS_PEN,    //!< pen used to draw the x-axis
      Y_AXIS_PEN,    //!< pen used to draw the y-axis
      X_TIC_PEN,     //!< pen used to draw the x-tics
      Y_TIC_PEN,     //!< pen used to draw the y-tics
      X_LABEL_PEN,   //!< pen used to draw the x-labels
      Y_LABEL_PEN,   //!< pen used to draw the y-labels
      X_GRID_PEN,    //!< pen used to draw the x-grid lines
      Y_GRID_PEN,    //!< pen used to draw the y-grid lines
      AXIS_NAME_PEN, //!< pen used to draw the axis labels
      NUM_PEN_TYPES
    };

    
    /// Base constructor
    AbstractPlotWidget(QWidget *parent=0);

    /// destructor
    ~AbstractPlotWidget();
    
    /// custom drawing 
    virtual void paintEvent(QPaintEvent *evt);

    /// listens for F11 which enables the fullscreen mode
    virtual void keyPressEvent(QKeyEvent *event);
    
    /// renders the whole content using the given qpainter
    void renderTo(QPainter &p);

    /// sets the background brush
    void setBackground(const QBrush &bgBrush);

    /// sets the pen for one of the widget compoments
    void setPen(PenType p, const QPen &pen);  
    
    /// Utility structure for styles usend in subclasses
    /** @see \ref _NONENCLATURE_ */
    struct Pen{
      /// creates a row style with given parameters
      /** Use Qt::NoPen or Qt::NoBrush to deactivate specific things
          Supported Symbol types are:
          /// available symbol types
          - ' ' (space) no symbol enum Symbol{
          - '.' point symbol (one pixel point)
          - 'x' cross symbol
          - '+' plus symbol
          - '*' asterisk symbol (like x and +)
          - 'o' circle symbol (upper case O for filled)
          - 's' square symbol (upper case S for filled)
          - 't' triangle symbol (upper case T for filled)
          - 'd' dianond symbol (upper case d for filled)
          
          Note: in the most simple case, QPen and QBrush have just a color
          attribute, which is allo supported by an extra constructor. So
          in order to create red lines, you can just pass a QColor instance,
          where a QPen is expected. E.g. 
          \code
          AbstractPlotWidget::Pen *p = new AbstractPlotWidget::Pen(QColor(255,0,0)), QColor(0,255,0),'x');
          \endcode
          creates a red line Pen, that that draws green 'x'-Symbols          
      */
      Pen(const QPen &linePen=Qt::NoPen, 
          const QPen &symbolPen=Qt::NoPen,
          char symbol= ' ',
          int symbolSize=5,
          const QBrush &fillBrush=Qt::NoBrush):
      linePen(linePen),symbolPen(symbolPen),symbol(symbol),symbolSize(symbolSize),fillBrush(fillBrush){}
      QPen linePen;     //!< pen for line structures
      QPen symbolPen;   //!< pen for symbols
      char symbol;      //!< symbol
      int symbolSize;   //!< symbol size (in pixels, point symbols are always size 1)
      QBrush fillBrush; //!< fill brush (this is e.g. used to fill the area beyond a function graph)
    };
    /// typedef for managed row-style pointers
    typedef SmartPtr<Pen> PenPtr;


    /// updates the screen
    /** a shortcut to ThreadedUpdatableWidget::updateFromOtherThread() */
    inline void render() { updateFromOtherThread(); }

    /// returns the given data vieport
    /** in subclasses, the data viewport can be estimated from the given data dynamically  */
    virtual Rect32f getDataViewPort() const;
    
    /// sets the default dataviewport
    /** if width is 0, the xrange is taken from the data
        if height is 0, the yrange is taken from the data \n
        <b>Important:</b> The viewport needs to be given normalized. I.e. 
        - viewport.x is minX
        - viewport.y is minY
        - viewport.right() is maxX
        - viewport.bottom() is maxY
        */
    void setDataViewPort(const Rect32f &viewPort);

    /// sets the default viewport
    /** @see setDAtaViewPort(const Rect32f&) 
        <b>Important:</b> The range length' must be positive (otherwise, they are swapped)
        */
    void setDataViewPort(const Range32f &xrange, const Range32f &yrange);

    /// locks drawing / data updates in subclasses
    void lock() const;

    /// unlocks drawing / data updates in subclasses
    void unlock() const;

    /// adds an annotation to the data viewport
    /** annotations are used to add some extra information to the data that is already
        displayed. Please note, that annotation points are not used to estimate the 
        data viewport. If you need the data viewport to be adapted automatically to
        show all your annotations, you have to compute and set the data viewport manually.

        @param type annotation primitive type
            - rectangles (type: 'r'): data order [x,y,width,height],...
            - circles (type 'c'): data order [x,y,radius]..
            - lines (type 'l') data order: [x1,y1,x2,y2], ...
            - text (type 't'): data order: [x,y],...
            - linestrip (type 'L') data order [x1,y1], ... 
              for linestrips annotations, "text" is not used
            - grid (type 'g') data order [nXCells, nYCells] [x1, x2], ...
              (point order is row major)
              for grid annotations, "text" is not used
           
            rectangles and circles can optionally be labelled with text labels
            The text labels are displayed centered at the annotations, but the
            text is not scaled. For text primitives, a non-""-text is mandatory.
        @param  data packed data that is used to draw 'num' primitives. I.e.
            for rectangles, data contains num * 4 floats

        This is not the way, data shall be visualized, the method is
        not optimized for speed and rendering and it will always copy
        the given data deeply.
    */
    void addAnnotations(const char type,const float *data, int num=1, 
                        const QPen &linePen = QColor(255,0,0),
                        const QBrush &brush = Qt::NoBrush,
                        const std::string &text="", const std::string &textDelim=",");

    /// removes all existing annotations
    void clearAnnotations();
    
    /// clears everything from the widget
    /** 'Everything' means all data and all annotations */
    virtual void clear() { clearAnnotations(); }

    protected:
    
    /// returns whether the zoom is active
    bool isZoomed() const;
    
    /// internally used
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

    /// internally used
    virtual void mouseMoveEvent(QMouseEvent *event);

    /// internally used
    virtual void mousePressEvent(QMouseEvent *event);

    /// internally used
    virtual void mouseReleaseEvent(QMouseEvent *event);

    /// internally used
    virtual void enterEvent(QEvent *event);

    /// internally used
    virtual void leaveEvent(QEvent *event);

    /// this must be implemented by the specific drawers
    /** if 0 is returned, no data is available */
    virtual bool drawData(QPainter &p) = 0;
    
    /// draws the ledgend
    virtual void drawLegend(QPainter &p,const Rect &where, bool horizontal);


    /// draws a default ledgen into the given quad
    void drawDefaultLedgend(QPainter &p,const Rect &where, bool horizontal, 
                            const std::vector<std::string> &rowNames,
                            const std::vector<PenPtr> &pens);

    /// draws one of the Symbols (filled symbols are drawn by setting the QPainters brush manually)
    /** Please note that this template is not specialized for filled symbols like filledRect or
        filledCircle. These shapes must be drawn by setting up the given QPainters brush
        appropriately before calling the function with the corresponding unfilled shape */
    template<char s> static inline void draw_symbol(QPainter &p,int size, float x, float y){ 
      ERROR_LOG("undefined given symbol ID"); 
    }
    
    /// converts a window x coordinate to logical drawing coordinate
    float winToDrawX(int winX) const;

    /// converts a window y coordinate to logical drawing coordinate
    float winToDrawY(int winY) const;

    /// converts a window coordinates to logical drawing coordinates
    Point32f winToDraw(const Point &p) const;
    
    /// convert logical drawing x coordinate to window coordinate
    int drawToWinX(float drawX) const;

    /// convert logical drawing y coordinate to window coordinate
    int drawToWinY(float drawY) const;

    /// convert logical drawing coordinates to window coordinates
    Point drawToWin(const Point32f &p) const;

    /// returns the current data viewport w.r.t. the current zoom
    Rect32f getDynamicDataViewPort() const;
    
    /// utility class for scoped locking
    struct Locker{
      const AbstractPlotWidget *w; //!< parent AbstractPlotWidget
      /// constructor locks the widget
      inline Locker(const AbstractPlotWidget *w):w(w){ this->w->lock(); }
      /// constructor locks the widget
      inline Locker(const AbstractPlotWidget &w):w(&w){ this->w->lock();}
      /// destructor unlocks the widget
      ~Locker() { w->unlock(); }
    };
  }; 
  
  /** \cond **/
  // (specialized) note, points have not size: use circle for larger round points ...
  template<> inline void AbstractPlotWidget::draw_symbol<'.'>(QPainter &p,int, float x, float y){ 
    //    p.drawPoint(QPoint(x,y));
    p.drawLine(QPoint(x,y),QPoint(x,y));
  }
  
  template<> inline void AbstractPlotWidget::draw_symbol<'x'>(QPainter &p,int size, float x, float y){ 
    p.drawLine(QPoint(x-size,y-size),QPoint(x+size,y+size));
    p.drawLine(QPoint(x-size,y+size),QPoint(x+size,y-size));
  }

  template<> inline void AbstractPlotWidget::draw_symbol<'+'>(QPainter &p,int size, float x, float y){ 
    p.drawLine(QPoint(x,y-size),QPoint(x,y+size));
    p.drawLine(QPoint(x-size,y),QPoint(x+size,y));
  }

  template<> inline void AbstractPlotWidget::draw_symbol<'*'>(QPainter &p,int size, float x, float y){ 
    draw_symbol<'+'>(p,(size*2)/3,x,y);    
    draw_symbol<'x'>(p,size,x,y);
  }

  /// brush needs to be set to the pen before ...
  template<> inline void AbstractPlotWidget::draw_symbol<'o'>(QPainter &p,int size, float x, float y){ 
    p.drawEllipse(QRect(x-size,y-size,2*size,2*size));
  }

  template<> inline void AbstractPlotWidget::draw_symbol<'s'>(QPainter &p,int size, float x, float y){ 
    p.drawRect(QRect(x-size,y-size,2*size,2*size));
  }

  template<> inline void AbstractPlotWidget::draw_symbol<'t'>(QPainter &p,int size, float x, float y){ 
    const QPoint e[3] = { QPoint(x,y-size),QPoint(x+size,y+size),QPoint(x-size,y+size) };
    p.drawConvexPolygon(e,3);
  }

  template<> inline void AbstractPlotWidget::draw_symbol<'d'>(QPainter &p,int size, float x, float y){ 
    const QPoint e[4] = { QPoint(x,y-size),QPoint(x+size,y),QPoint(x,y+size), QPoint(x-size,y) };
    p.drawConvexPolygon(e,4);
  }
  /** \endcond **/
}

#endif

