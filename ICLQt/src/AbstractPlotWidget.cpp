/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/AbstractPlotWidget.cpp                       **
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

#include <ICLUtils/SmartPtr.h>
#include <ICLUtils/FixedMatrix.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/LinearTransform1D.h>

#include <ICLQt/AbstractPlotWidget.h>
#include <ICLQt/GUIWidget.h>
#include <ICLQt/GUI.h>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <QtGui/QPalette>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QMessageBox>

namespace icl{

  

  struct AbstractPlotWidget::Data{
    struct Annotation{
      Annotation(char type,const float *data, int num, 
                 const QPen &pen, const QBrush &brush,
                 const std::string &text, const std::string &delim):
        type(type), num(num), pen(pen), brush(brush){
        if(text.size()){
          texts = tok(text,delim);
          hasText = true;
        }else{
          hasText = false;
        }
        if(type == 't' && !hasText){
          throw ICLException("AbstractPlotWidget::addAnnotation(..): type is text ('t'), but no text was given");
        }
        const int dim =  ((type=='c') ? 3 :
                          (type=='r' || type == 'l') ? 4 :
                          (type=='t' || type == 'L') ? 2 : 
                          (type=='g') ? (data[0]*data[1]*2+2) : 0);
        if(!dim) throw ICLException("AbstractPlotWidget::addAnnotation(..): invalid type");
        this->data.assign(data,data+num*dim);
      }
      char type;
      int num;
      std::vector<float> data;
      QPen pen;
      QBrush brush;
      std::vector<std::string> texts;
      bool hasText;
    };
    std::vector<Annotation*> annotations;
    
    QMutex mutex;
    QWidget *parentBeforeFullScreen;

    Data():mutex(QMutex::Recursive),parentBeforeFullScreen(0){
      mousePos = QPoint(-1,-1);
      track_mouse = true;
    }

    QBrush bgBrush;
    QPainter *currPainter;
    std::vector<float> constants;
    std::vector<QPen> pens;
    inline bool setPen(PenType t) const{
      const QPen p = pens[t];
      if(p != Qt::NoPen){
        currPainter->setPen(p);
        return true;
      }
      return false;
    }
    inline bool hasPen(PenType t) const{
      return pens[t] != Qt::NoPen;
    }

    bool menuCreated;
    GUI menu;
    
    QPoint mousePressCurr;
    QPoint mousePressStart;
    bool disableUpdate;
    Range32f lastXRange, lastYRange;
    Rect32f lastWindowRect;
    
    std::vector<Rect32f> viewPortStack;
    Rect32f viewPort;
    
    QPoint mousePos;
    bool track_mouse;
    
    QRect zoomIndicatorRect;
    bool showZoomIndicator;
  };
  
  bool AbstractPlotWidget::isZoomed() const{
    return data->viewPortStack.size();
  }
  
  void AbstractPlotWidget::addAnnotations(const char type,const float *data, int num, 
                                          const QPen &linePen,
                                          const QBrush &brush,
                                          const std::string &text, 
                                          const std::string &textDelim){
    Locker lock(this);
    this->data->annotations.push_back(new Data::Annotation(type, data, num, linePen, brush, text, textDelim));
  }

  void AbstractPlotWidget::clearAnnotations(){
    Locker lock(this);
    for(unsigned int i=0;i<data->annotations.size();++i){
      delete data->annotations[i];
    }
    data->annotations.clear();
  }

  void AbstractPlotWidget::lock() const{
    data->mutex.lock();
  }

  void AbstractPlotWidget::unlock() const{
    data->mutex.unlock();
  }

  Rect32f AbstractPlotWidget::getDataViewPort() const{
    return data->viewPort;
  }
  
  void AbstractPlotWidget::setDataViewPort(const Rect32f &viewPort){
    data->viewPort = viewPort;
    if(data->viewPort.width < 0){
      data->viewPort.x += data->viewPort.width;
      data->viewPort.width *= -1;
    }
    if(data->viewPort.height < 0){
      data->viewPort.y += data->viewPort.height;
      data->viewPort.height *= -1;
    }
  }
  
  void AbstractPlotWidget::setDataViewPort(const Range32f &xrange, const Range32f &yrange){
    const Range32f &rx =  xrange, &ry = yrange;
    setDataViewPort(Rect32f(rx.minVal, ry.minVal, rx.getLength(), ry.getLength()));
  }



  void AbstractPlotWidget::property_changed(const Property &p){
    if(p.name == "show mouse pos"){
      bool on = getPropertyValue(p.name);
      data->track_mouse = on;
    }
    else if(p.name == "style preset"){
      std::string preset = getPropertyValue("style preset");
      QPen defaultPen;
      QPen gridPen;
      QPen axisLabelPen;
      lock();
      if(preset == "default"){
        defaultPen = QPen(QColor(50,50,50));
        gridPen = QPen(QColor(150,150,150));
        axisLabelPen = QPen(QColor(0,0,0));
        data->bgBrush = QPalette().window();
      }else if(preset == "black"){
        defaultPen = QPen(QColor(150,150,150));
        gridPen = QPen(QColor(50,50,50));
        axisLabelPen = QPen(QColor(200,200,200));
        data->bgBrush = QBrush(QColor(0,0,0));
      }else if(preset == "white"){
        defaultPen = QPen(QColor(100,100,100));
        gridPen = QPen(QColor(200,200,200));
        axisLabelPen = QPen(QColor(50,50,50));
        data->bgBrush = QBrush(QColor(255,255,255));
      }

      data->pens[X_AXIS_PEN] = defaultPen;
      data->pens[Y_AXIS_PEN] = defaultPen;
      data->pens[X_TIC_PEN] = defaultPen;
      data->pens[Y_TIC_PEN] = defaultPen;
      data->pens[X_LABEL_PEN] = defaultPen;
      data->pens[Y_LABEL_PEN] = defaultPen;
      data->pens[X_GRID_PEN] = gridPen;
      data->pens[Y_GRID_PEN] = gridPen;
      data->pens[AXIS_NAME_PEN] = axisLabelPen;
      unlock();
    }
    else if(p.name == "fullscreen"){
      if(isFullScreen()){
        setWindowState(windowState() & !Qt::WindowFullScreen);
        setParent(data->parentBeforeFullScreen);
        if(data->parentBeforeFullScreen && data->parentBeforeFullScreen->layout()){
          data->parentBeforeFullScreen->layout()->addWidget(this);
        }
      }else{
        data->parentBeforeFullScreen = (QWidget*)parent();
        setParent(0);
        setWindowState(windowState() ^ Qt::WindowFullScreen);
      }
      show();
    }

    if(!data->disableUpdate){
      updateFromOtherThread();
    }
    
    /* basically, this works, but the results is useless due to it's poor quality
        if(p.name == "export to SVG"){
        QSvgGenerator svgGen;
        QFile file("./output.svg");
        svgGen.setOutputDevice(&file);
        //svgGen.setViewBox(QRect(0,0,width(),height()));
        QPainter p(&svgGen);
        renderTo(p);
        }
    */
    
  }
  
  AbstractPlotWidget::AbstractPlotWidget(QWidget *parent):
    ThreadedUpdatableWidget(parent), data(new Data){
    
    setMouseTracking(true);

    //setAttribute(Qt::WA_PaintOnScreen,true);
    setAttribute(Qt::WA_OpaquePaintEvent,true);

    data->disableUpdate = false;
    data->menuCreated = false;
    data->bgBrush = QPalette().window();
    
    addProperty("borders.left","range:spinbox","[0,1000]",35,0,"Left distance from widget border to the drawing area");
    addProperty("borders.right","range:spinbox","[0,1000]",5,0,"Right distance from widget border to the drawing area");
    addProperty("borders.top","range:spinbox","[0,1000]",5,0,"Top distance from widget border to the drawing area");
    addProperty("borders.bottom","range:spinbox","[0,1000]",38,0,"Bottom distance from widget border to the drawing area");


    addProperty("antialiasing","flag","",false,0,"Enables Antialiased Rendering (slow)");
    addProperty("dynamic-tic-scaling", "flag","",true,0,"Automatic adaption of tic-distance when zooming in.");
    addProperty("style preset","menu","default,black,white","default",0,"Preset forground and background Styles");

    addProperty("tics.length","range:spinbox","[0,100]",6, 0, "Length of tics in pixels.");
    addProperty("tics.x-distance","float","[1E-37,1E+37]",1, 0, "Distance for tics along the X-axis(in drawing units)");
    addProperty("tics.y-distance","float","[1E-37,1E+37]",10, 0,"Distance for tics along the Y-axis (in drawing units)");
    addProperty("tics.x-grid","flag","",true,0,"Enables the vertical grid");
    addProperty("tics.y-grid","flag","",true,0,"Enables the horizontal grid");

    addProperty("labels.x-precision","range:spinbox","[0,20]",3,0,"Precision for X-axis labels");
    addProperty("labels.y-precision","range:spinbox","[0,20]",3,0,"Precision for Y-axis labels");
    addProperty("labels.text-size","range:spinbox","[1,100]",8,0,"Font size");
    addProperty("labels.x-axis","string","100","",0,"X-axis label");
    addProperty("labels.y-axis","string","100","",0,"Y-axis label");
    addProperty("labels.diagramm","string","100","",0,"Headline label");

    addProperty("enable lines","flag","",true,0,"Enables line rendering (for scatter- and function data)");
    addProperty("enable symbols","flag","",true,0,"Enables symbol rendering (for scatter- and function data)");
    addProperty("enable fill","flag","",true,0,"Enables filled rendering (for scatter- and function data)");
    addProperty("draw legend","flag","",true,0,"Show/Hide the legend");
    addProperty("render symbols as images","flag","",false,0,"Method that is used for symbols");
    addProperty("show zoom indicator","flag","",true,0,"Visualize the current zoom rect in the upper right corner"); 
    addProperty("highlight 0-axis'","flag","",true,0,"Draw the X=0 and Y=0 grid lines with a thicker pen");

    addProperty("drawing time","info","","?",0,"Benchmark for rendering time");

    addProperty("legend.x","range:spinbox","[-10000,10000]",10,0,"Legend X-offset");
    addProperty("legend.y","range:spinbox","[-10000,10000]",-22,0,"Legend Y-offset");
    addProperty("legend.width","range:spinbox","[-10000,10000]",-20,0,"Legend width");
    addProperty("legend.height","range:spinbox","[-10000,10000]",20,0,"Legend height");

    addProperty("legend.orientation","menu","horizontal,vertical","horizontal",0,"Horizontal or vertical legend allignment");
    addProperty("show mouse pos","flag","",true,0,"if true, the mose position is visualized");
    addProperty("fullscreen","flag","",false,0,"activates the fullscreen view (F11)");
    

    // does not work properly
    //    addProperty("export to SVG","command","","");
    
    data->pens.resize(NUM_PEN_TYPES);

    const QPen defaultPen(QColor(50,50,50));
    data->pens[X_AXIS_PEN] = defaultPen;
    data->pens[Y_AXIS_PEN] = defaultPen;
    data->pens[X_TIC_PEN] = defaultPen;
    data->pens[Y_TIC_PEN] = defaultPen;
    data->pens[X_LABEL_PEN] = defaultPen;
    data->pens[Y_LABEL_PEN] = defaultPen;
    data->pens[X_GRID_PEN] = QPen(QColor(150,150,150));
    data->pens[Y_GRID_PEN] = QPen(QColor(150,150,150));
    data->pens[AXIS_NAME_PEN] = QPen(QColor(0,0,0));
    
    setContextMenuPolicy(Qt::CustomContextMenu);
    
    registerCallback(function(this,&AbstractPlotWidget::property_changed));
    
    data->mousePressStart = data->mousePressCurr = QPoint(-1,-1);
    data->lastXRange = data->lastYRange = Range32f(0,0);
    data->lastWindowRect = Rect32f::null;
  }

  
  AbstractPlotWidget::~AbstractPlotWidget(){
    clearAnnotations();
    delete data;
  }

  void AbstractPlotWidget::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_F11){
      const bool on = getPropertyValue("fullscreen");
      setPropertyValue("fullscreen",!on);
    }
  }
  
  Rect32f AbstractPlotWidget::getDynamicDataViewPort() const{
    if(data->viewPortStack.size()){
      return data->viewPortStack.back();
    }else{
      return getDataViewPort();
    }
      
  }
  
  void  AbstractPlotWidget::setBackground(const QBrush &bgBrush){
    data->bgBrush = bgBrush;
  }

  
  void AbstractPlotWidget::paintEvent(QPaintEvent *evt){
    QPainter p(this);
    renderTo(p);
  }
  void AbstractPlotWidget::renderTo(QPainter &p){
    Locker lock(this);
    
    Time t = Time::now();

    data->currPainter = &p;
    
    if(getPropertyValue("antialiasing")){
      p.setRenderHint(QPainter::Antialiasing);
    }
    const float w = width(), h = height();
    p.fillRect(QRect(0,0,w,h),data->bgBrush);
    
    const float b_left = getPropertyValue("borders.left");
    const float b_right = getPropertyValue("borders.right");
    const float b_top = getPropertyValue("borders.top");
    const float b_bottom = getPropertyValue("borders.bottom");

    const bool showZoomIndicator = getPropertyValue("show zoom indicator");
    const bool highlightZeroAxis = getPropertyValue("highlight 0-axis'");

    data->lastWindowRect = Rect32f(b_left, b_top, w-(b_right+b_left), h-(b_bottom+b_top));
    data->showZoomIndicator = showZoomIndicator;
    // --------------------------------------------------
    // -- setup view transform --------------------------
    // --------------------------------------------------
    Rect32f v = getDynamicDataViewPort();
    Rect32f vd = getDataViewPort();
    

    const bool zoomed = v != vd;

    Range32f rx(v.x,v.right());
    Range32f ry(v.y,v.bottom());

    Range32f rxd(vd.x,vd.right());
    Range32f ryd(vd.y,vd.bottom());

    if(!rx.getLength()){
      WARNING_LOG("data x-range length is 0 -> using new range [0,1]");
      rx = Range32f(0,1);
    }
    if(ry== Range32f(0,0)){
      ry = Range32f(0,1);
      WARNING_LOG("data y-range length is 0 -> using new range [0,1]");
    }
    data->lastXRange = rx;
    data->lastYRange = ry;
    
    const float mx = (w-1-(b_left+b_right))/ rx.getLength();
    const float my = (b_top-(h-1-b_bottom))/ ry.getLength();
    const float bx = -mx * rx.minVal + b_left;
    const float by = -my * ry.minVal + (h-1-b_bottom);


    /*
    p.setPen(QColor(255,0,0));
    p.drawLine(QPoint(0,height()-b_bottom), QPoint(width(), height()-b_bottom));

    p.setPen(QColor(0,255,0));
    p.drawLine(QPoint(0,drawToWinY(ry.minVal)), QPoint(width(),drawToWinY(ry.minVal))); 
    */

    // set up view transform to data range
    p.translate(bx,by);
    p.scale(mx,my);
    
    const int f = getPropertyValue("labels.text-size");
    QFont font = p.font();
    font.setPointSizeF(f);
    p.setFont(font);

    const int prx = getPropertyValue("labels.x-precision");
    const int pry = getPropertyValue("labels.y-precision");

    

    // --------------------------------------------------
    // -- X-Axis Tics and labels ------------------------
    // --------------------------------------------------
    if(data->hasPen(X_TIC_PEN) || data->hasPen(X_LABEL_PEN) || data->hasPen(X_GRID_PEN) || data->hasPen(AXIS_NAME_PEN)){
      const bool xgrid = getPropertyValue("tics.x-grid");
      float dx = getPropertyValue("tics.x-distance");
      
      if(v.width && vd.width && getPropertyValue("dynamic-tic-scaling").as<bool>()){
        float f = 1;
        while(2 * v.width * f < vd.width) f*=2;
        dx /= f;
      }
      
      const float tl = getPropertyValue("tics.length").as<float>()/(2*my) ;
      const float y1 = winToDrawY(h-b_bottom) - tl, y2 =  winToDrawY(h-b_bottom) + tl;

      const float gridT = iclMax(ryd.minVal,winToDrawY(b_top));
      const float gridB = iclMin(ryd.maxVal,winToDrawY(height()-b_bottom));
      
      float firstVisibleTic = winToDrawX(b_left);
      int dxUnits = ceil(firstVisibleTic/dx);
      firstVisibleTic = dx * dxUnits;
      
      float lastVisibleTic = winToDrawX(width()-b_right);
      dxUnits = ceil(lastVisibleTic/dx);
      lastVisibleTic = dx * dxUnits;
      
      for(float x=firstVisibleTic; x <= lastVisibleTic; x+=dx){

        if(xgrid && data->setPen(X_GRID_PEN)){
          p.drawLine(QPointF(x,gridT),QPointF(x,gridB));
        }
        if(data->setPen(X_TIC_PEN)){
          p.drawLine(QPointF(x,y1),QPointF(x,y2));
        }
        if(data->setPen(X_LABEL_PEN)){
          p.save();
          p.resetTransform();
          //          p.drawText(QRectF(mx*x+bx,my*(y1)+by+f/2+2,0,0), Qt::AlignHCenter |  Qt::TextDontClip, QString::number(x,'f',prx));
          p.drawText(QRectF(mx*x+bx,h-b_bottom +f/2,0,0), Qt::AlignHCenter |  Qt::TextDontClip, QString::number(x,'f',prx));

          if(x == firstVisibleTic && data->setPen(AXIS_NAME_PEN)){
            const std::string label = getPropertyValue("labels.x-axis");
            if(label.length()){
              p.drawText(QRect(b_left + (width()-(b_left+b_right))/2, h-b_bottom + 2*f, 0,0),
                         Qt::AlignHCenter | Qt::TextDontClip, label.c_str());
            }
          }
          p.restore();
        }
      }
    }

    // --------------------------------------------------
    // -- Y-Axis Tics and labels ------------------------
    // --------------------------------------------------

   
    if(data->hasPen(Y_TIC_PEN) || data->hasPen(Y_LABEL_PEN) || data->hasPen(Y_GRID_PEN) || data->hasPen(AXIS_NAME_PEN)){
      const bool ygrid = getPropertyValue("tics.y-grid");
      float dy = getPropertyValue("tics.y-distance");

      if(v.height && vd.height && getPropertyValue("dynamic-tic-scaling").as<bool>()){
        float f = 1;
        while(v.height * f < vd.height) f*=2;
        dy /= f;
      }

      const float tl = getPropertyValue("tics.length").as<float>()/(2*mx) ;
      const float x1 = rx.minVal - tl, x2 = rx.minVal + tl;
      // const float numTics = ryd.getLength()/dy;
      //      const float lastTic = ryd.maxVal + ryd.getLength()/(numTics*100);
      
      const float gridL = iclMax(rxd.minVal,winToDrawX(b_left));
      const float gridR = iclMin(rxd.maxVal,winToDrawX(width()-b_right));
      
      float firstVisibleTic = winToDrawY(height()-b_bottom);
      int dyUnits = ceil(firstVisibleTic/dy);
      firstVisibleTic = dy * dyUnits;
      
      float lastVisibleTic = winToDrawY(b_top);
      dyUnits = floor(lastVisibleTic/dy);
      lastVisibleTic = dy * dyUnits;
      
      for(float y=firstVisibleTic; y <= lastVisibleTic; y+=dy){
        
        if(ygrid && data->setPen(Y_GRID_PEN)){
          p.drawLine(QPointF(gridL,y),QPointF(gridR,y));
        }
        if(data->setPen(Y_TIC_PEN)){
            p.drawLine(QPointF(x1,y),QPointF(x2,y));
        }
        if(data->setPen(Y_LABEL_PEN)){
          p.save();
          p.resetTransform();
          p.drawText(QRectF(mx*x1+bx-f/2, my*y+by,0,0), Qt::AlignVCenter | Qt::AlignRight | Qt::TextDontClip, 
                     QString::number(y,'f',pry));
          if(y == firstVisibleTic && data->setPen(AXIS_NAME_PEN)){
            const std::string label = getPropertyValue("labels.y-axis");
            if(label.length()){
              p.translate(2,b_top + (height()-(b_top+b_bottom))/2);
              p.rotate(-90);
              p.drawText(QRect(0,0,0,0),Qt::AlignHCenter | Qt::TextDontClip, label.c_str());
            }
          }
          p.restore();
        }
      }
      
    }

    p.save();
    bool success = drawData(p);
    p.restore();
    
    // x-axis--------------------------------------------
    if(data->setPen(X_AXIS_PEN)){
      p.drawLine(QPointF(rx.minVal, ry.minVal), QPointF(rx.maxVal, ry.minVal));
      if(highlightZeroAxis){
        if(Range32s(b_left, width()-b_right).contains(drawToWinX(0))){
          p.drawLine( QPointF(0, ry.minVal), QPointF(0, ry.maxVal));
        }
      }
    }

    // y-axis
    if(data->setPen(Y_AXIS_PEN)){
      p.drawLine(QPointF(rx.minVal, ry.minVal), QPointF(rx.minVal, ry.maxVal));
      if(highlightZeroAxis){
        if(Range32s(b_top, height()-b_bottom).contains(drawToWinY(0))){
          p.drawLine( QPointF(rx.minVal, 0), QPointF(rx.maxVal, 0));
        }
      }
    }

    p.resetTransform();
    if(!success){
      p.setPen(QColor(50,50,50));
      p.drawRect(QRectF(0,0,w-1,h-1));
      p.drawText(QRectF(0,0,w,h),Qt::AlignCenter,"no data given");
      return;
    }else if(getPropertyValue("draw legend")){
      const int lx = getPropertyValue("legend.x");
      const int ly = getPropertyValue("legend.y");
      const int lw = getPropertyValue("legend.width");
      const int lh = getPropertyValue("legend.height");
      
      p.setClipping(false);
      drawLegend(p,Rect((lx<0) ? (w+lx) : lx,
                        (ly<0) ? (h+ly) : ly,
                        (lw<0) ? (w+lw) : lw,
                        (lh<0) ? (h+lh) : lh), getPropertyValue("legend.orientation")[0] == 'h');
    }

    std::string title = getPropertyValue("labels.diagramm");
    if(title.length() && data->setPen(X_AXIS_PEN)){
      p.drawText(QRectF(w/2,10, 0, 0), Qt::AlignCenter | Qt::TextDontClip, title.c_str());
    }
    
    p.resetTransform();
    if(data->mousePressCurr != QPoint(-1,-1)){
      p.setPen(QColor(0,100,255));
      p.setBrush(QColor(0,100,255,50));
      const QPoint &q = data->mousePressStart;
      const QPoint &q2 = data->mousePressCurr;
      
      p.drawRect(QRect(q,q2).normalized());

      Point32f pd = winToDraw(Point(q.x(), q.y()));

      p.drawText(q-QPoint(-10,5), QString("(%1,%2)").arg(pd.x).arg(pd.y));
      p.drawLine(q-QPoint(0,3), q-QPoint(0,9));

      p.drawLine(q, q-QPoint(3,3));
      p.drawLine(q, q-QPoint(-3,3));
      p.drawLine(q-QPoint(-3,3), q-QPoint(3,3));

      p.drawLine(q - QPoint(0,9), q - QPoint(-9,9));

      
      if(fabs(data->mousePressStart.x() - q2.x()) > 100
         || fabs(data->mousePressStart.y() - q2.y()) > 10){

        Point32f pd2 = winToDraw(Point(q2.x(), q2.y()));
        QString t = QString("(%1,%2)").arg(pd2.x).arg(pd2.y);
        QRect r = p.boundingRect(0,0,400,400,0,t);
        p.drawText(q2-QPoint(r.width()+10,-15), t);
        
        p.drawLine(q2+QPoint(0,3), q2+QPoint(0,9));
        p.drawLine(q2+QPoint(0,9), q2+QPoint(-9,9));

        p.drawLine(q2, q2+QPoint(3,3));
        p.drawLine(q2, q2+QPoint(-3,3));
        p.drawLine(q2 + QPoint(3,3), q2+QPoint(-3,3));
      }
    }

    
    if(zoomed && showZoomIndicator){
      p.resetTransform();
      p.setBrush(Qt::NoBrush);
      p.setPen(QColor(255,0,0,100));
      static const float W = 50;
      const float h = W * height() / iclMax(1,width());
      QRect r(width()-b_right-W, b_top, W, h);
      
      const LinearTransform1D A(Range32f(vd.left(), vd.right()),Range32f(0,1));
      const LinearTransform1D B(Range32f(vd.top(), vd.bottom()),Range32f(1,0));
      const float lFrac = A(v.left()), rFrac = A(v.right());
      const float tFrac = B(v.top()), bFrac = B(v.bottom());
      QRectF rZoom ( QPointF(r.x() + lFrac * r.width(),
                             r.y() + tFrac * r.height()),
                     QPointF(r.x() + rFrac * r.width(),
                             r.y() + bFrac * r.height() ));

      p.drawRect(r);
      data->zoomIndicatorRect = r;

      p.setBrush(QColor(255,0,0,100));
      p.drawRect(rZoom);
      
      p.drawText(QRectF(r.x(), r.bottom(), r.width(), 10), Qt::AlignCenter | Qt::TextDontClip, 
                 QString("zoom (#")+QString::number(data->viewPortStack.size())+ ")");

    }

    if(data->track_mouse && (data->mousePos.x() > 0)){
      p.setPen(QColor(0,100,255));
      p.setBrush(QColor(0,100,255,50));
      const int x = clip(data->mousePos.x(),(int)b_left,(int)(w-b_right));
      const int y = clip(data->mousePos.y(),(int)b_top,(int)(h-b_bottom));
      const int hb = h-b_bottom;
      
      const QPoint p1[3] = { QPoint(b_left,y), QPoint(b_left-5, y-5), QPoint(b_left-5, y+5)};
      p.drawConvexPolygon (p1,3);

      const QPoint p2[3] = { QPoint(x, hb), QPoint(x-5, hb +5), QPoint(x+5, hb + 5) };
      p.drawConvexPolygon (p2,3);

      QString text = QString("(%1,%2").arg(winToDrawX(x)).arg(winToDrawY(y));
      QRect br = p.boundingRect(0,0,500,500, Qt::TextDontClip, text);
      p.setPen(QColor(255,255,255));
      p.drawText( QRect(x - br.width()-2 + 1, y-12  +1 ,0,0), Qt::TextDontClip, text);
      p.setPen(QColor(0,100,255));
      p.drawText( QRect(x - br.width()-2, y-12,0,0), Qt::TextDontClip, text);
      
      QPen pen(QColor(0,100,255),1, Qt::DashLine);
      QVector<qreal> dashpattern;
      dashpattern << 5 << 5;
      pen.setDashPattern(dashpattern);
      p.setPen(pen);
      p.drawLine(QPoint(b_left, y),QPointF(w-b_right,y));
      p.drawLine(QPoint(x,hb -5), QPoint(x, b_top));
    }
    
    /// draw the annotations
    if(data->annotations.size()){
      p.resetTransform();
      p.setClipping(true);
      p.setClipRect(QRect(QPoint(b_left, b_top), QPoint(width()-b_right-2, height()-b_bottom-2)));
      LinearTransform1D tx(data->lastXRange, Range32f(data->lastWindowRect.x, data->lastWindowRect.right()));
      LinearTransform1D ty(data->lastYRange, Range32f(data->lastWindowRect.bottom(),data->lastWindowRect.y));
      for(unsigned int i=0;i<data->annotations.size();++i){
        const Data::Annotation &a = *data->annotations[i];
        p.setPen(a.pen);
        p.setBrush(a.brush);
        const float *data = a.data.data();
        switch(a.type){
          case 'L':{ // line strip
            QPoint last(tx(data[0]), ty(data[1]));
            for(int i=1;i<a.num;++i){
              QPoint curr(tx(data[2*i]),ty(data[2*i+1]));
              p.drawLine(last,curr);
              last = curr;
            }
            break;
          }
          case 'g':{ // grid
            const int W = data[0];
            const int H = data[1];
            const Point32f *ps = reinterpret_cast<const Point32f*>(data+2);
            QPoint *currLine = new QPoint[W], *lastLine = new QPoint[W];
            for(int i=0;i<W;++i){
              lastLine[i] = QPoint(tx(ps[i].x),ty(ps[i].y));
              if(i) p.drawLine(lastLine[i-1], lastLine[i]);
            }
            for(int y=1;y<H;++y){
              ps += W;
              for(int i=0;i<W;++i){
                currLine[i] = QPoint(tx(ps[i].x),ty(ps[i].y));
              }              
              p.drawLine(currLine[0], lastLine[0]);
              for(int x=1;x<W;++x){
                p.drawLine(currLine[x-1], currLine[x]);
                p.drawLine(lastLine[x], currLine[x]);
              }
              
              std::swap(currLine,lastLine);
            }
            delete [] currLine;
            delete [] lastLine;
            break;
          }
          case 'r':{
            for(int i=0;i<a.num;++i, data+=4){
              const float x = tx(data[0]), y = ty(data[1]),
                          m = tx(data[0]+data[2]), n = ty(data[1]+data[3]); 
              QRect r(x,y, m-x, n-y);
              p.drawRect(r);
              if(a.hasText){
                p.drawText(r, Qt::TextDontClip  | Qt::AlignCenter, a.texts[i].c_str());
              }
            }
            break; 
          }
          case 'c':{
            for(int i=0;i<a.num;++i, data+=3){
              const float x = tx(data[0]-data[2]);
              const float y = ty(data[1]-data[2]);
              const float m = tx(data[0]+data[2]);
              const float n = ty(data[1]+data[2]);
                               
              QRect r(x,y,m-x, n-y);
              p.drawEllipse(r);
              if(a.hasText){
                p.drawText(r, Qt::TextDontClip  | Qt::AlignCenter, a.texts[i].c_str());
              }
            }
            break;
          }
          case 'l':{
            for(int i=0;i<a.num;++i, data+=4){
              QPoint p1(tx(data[0]),ty(data[1]));
              QPoint p2(tx(data[2]),ty(data[3]));
              p.drawLine(p1,p2);
              if(a.hasText){
                p.drawText(QRectF((p1+p2)/2, (p1+p2)/2), Qt::TextDontClip  | Qt::AlignCenter, a.texts[i].c_str());
              }
            }
             break;
          }
          case 't':{
            for(int i=0;i<a.num;++i, data+=2){
              p.drawText(QRectF(tx(data[0]),ty(data[1]),0,0), Qt::TextDontClip  | Qt::AlignCenter, a.texts[i].c_str());
            }
          }
        }
      }
    }


    data->disableUpdate = true;
    const int dt = t.age().toMilliSeconds();
    setPropertyValue("drawing time",str(dt) + "ms");
    data->disableUpdate = false;
  }


  void AbstractPlotWidget::mouseDoubleClickEvent(QMouseEvent *event){
  
  }

  void AbstractPlotWidget::mouseMoveEvent(QMouseEvent *event){
    data->mousePos = event->pos();
    bool doUpdate = data->track_mouse;
    if(data->mousePressCurr != QPoint(-1,-1)){
      data->mousePressCurr = event->pos();
      doUpdate = true;
    }
    if(doUpdate) update();
  }

  void AbstractPlotWidget::mousePressEvent(QMouseEvent *event){
    setFocus();
    switch(event->button()){
      case Qt::LeftButton:
        if(data->showZoomIndicator && data->viewPortStack.size() && 
           data->zoomIndicatorRect.contains(event->pos())){
          data->viewPortStack.clear();
        }else{
          data->mousePressStart = data->mousePressCurr = event->pos();
        }
        update();

        break;
      case Qt::RightButton:
        if(!data->menuCreated){
          data->menuCreated = true;
          std::string origID = getConfigurableID();
          static int confiurableIdx = 0;
          std::string id = "_ICL_AbstractPlotWidget-"+str(confiurableIdx++);
          setConfigurableID(id);
          data->menu = Prop(id);
          data->menu.create();
          setConfigurableID(origID);
          data->menu.getRootWidget()->setWindowFlags(Qt::Popup);
        }
        data->menu.getRootWidget()->move(event->globalX(), event->globalY());
        data->menu.show();
        break;
      case Qt::MidButton: break;
      default: break;
    }
  }

  void AbstractPlotWidget::mouseReleaseEvent(QMouseEvent *event){
    if(data->mousePressCurr != QPoint(-1,-1)){
      QPoint d = data->mousePressCurr - data->mousePressStart;
      if(abs(d.x()) > 2 && abs(d.y()) > 2){
        QPoint a = data->mousePressCurr, b = data->mousePressStart;
        int le = iclMin(a.x(), b.x()), ri = iclMax(a.x(), b.x());
        int to = iclMax(a.y(), b.y()), bo = iclMin(a.y(), b.y());
        
        Point32f ul = winToDraw(Point(le,to)), lr = winToDraw(Point(ri,bo));
        Rect32f dv = getDataViewPort();
        
        // avoid zoom outside data ...
        ul.x = clip<float>(ul.x, dv.x, dv.right());
        lr.x = clip<float>(lr.x, dv.x, dv.right());
        ul.y = clip<float>(ul.y, dv.y, dv.bottom());
        lr.y = clip<float>(lr.y, dv.y, dv.bottom());
        
        if( (lr.x - ul.x)/dv.width < 1.E-6 ||
            (lr.y - ul.y)/dv.height < 1.E-6){
          QMessageBox::information(this,"invalid zoom factor",
                                   "This zoom factor is too high!\n"
                                   "To avoid numerical instabilities,\n"
                                   "the dynamic zoom factor can never\n"
                                   "be heigher than 10^6");
        }else{        
          data->viewPortStack.push_back(Rect32f(ul, Size32f(lr.x - ul.x, lr.y - ul.y)));
        }
        
      }else{
        if(data->viewPortStack.size()){
          data->viewPortStack.pop_back();
        }
      }
      data->mousePressStart = data->mousePressCurr = QPoint(-1,-1);
      update();
    }
  }
  void AbstractPlotWidget::enterEvent(QEvent *event){
  }
  void AbstractPlotWidget::leaveEvent(QEvent *event){
    data->mousePos = QPoint(-1,-1);
  }
  void AbstractPlotWidget::drawLegend(QPainter &p,const Rect &where, bool horizontal){
  }
  
  void AbstractPlotWidget::drawDefaultLedgend(QPainter &p,const Rect &where, bool horizontal, 
                                      const std::vector<std::string> &rowNames,
                                      const std::vector<PenPtr> &pens){
    const int N = rowNames.size();
    ICLASSERT_RETURN(rowNames.size() == pens.size());
    
    const int X = where.x;
    const int Y = where.y;
    const int W = where.width;
    const int H = where.height;
    static const int GAP_X = 4;
    static const int GAP_Y = 2;
    const int MAX_LEGEND_ROWS = H/(10+GAP_Y); // 10+2gap
    const int MAX_ENTRIES_PER_ROW = ceil(float(N)/MAX_LEGEND_ROWS);
    const int USED_LEGEND_ROWS = iclMin(MAX_LEGEND_ROWS,N);
    const int ENTRIES_IN_LAST_ROW = N-(USED_LEGEND_ROWS*MAX_ENTRIES_PER_ROW);
    const int ENTRY_WIDTH = (W-(MAX_ENTRIES_PER_ROW-1)*GAP_X)/MAX_ENTRIES_PER_ROW;
    const int ENTRY_HEIGHT = (H-(USED_LEGEND_ROWS-1)*GAP_Y)/USED_LEGEND_ROWS;
    
    int next = 0;
    const bool LINES_ON = getPropertyValue("enable lines");
    const bool SYMBOLS_ON = getPropertyValue("enable symbols");
    const bool FILL_ON = getPropertyValue("enable fill");
        
    for(int r=0;r<USED_LEGEND_ROWS;++r){
      const int ENTRIES_IN_THIS_ROW = (r == USED_LEGEND_ROWS-1) ? (ENTRIES_IN_LAST_ROW ? ENTRIES_IN_LAST_ROW : MAX_ENTRIES_PER_ROW) : MAX_ENTRIES_PER_ROW;
      for(int n=0;n<ENTRIES_IN_THIS_ROW;++n,++next){
        const Pen &s = *pens[next];
        
        const int x = X+n*(ENTRY_WIDTH+GAP_X);
        const int y = Y+r*(ENTRY_HEIGHT+GAP_Y);
        //        const int w = ENTRY_WIDTH;
        // const int h = ENTRY_HEIGHT;
        p.setPen(QColor(50,50,50));
        p.setBrush(FILL_ON ? s.fillBrush : Qt::NoBrush);
        static const int RECT_DIM=10;

        if(FILL_ON && s.fillBrush != Qt::NoBrush){
          if(LINES_ON){
            p.setPen(s.linePen);
          }else{
            p.setPen(Qt::NoPen);
          }
          p.drawRect(QRect(x+6,y+6,6,6));
        }else{
          if(LINES_ON){
            p.setPen(s.linePen);
            p.drawLine(x,y+(ENTRY_HEIGHT-RECT_DIM)/2+RECT_DIM/2,x+RECT_DIM,y+(ENTRY_HEIGHT-RECT_DIM)/2+RECT_DIM/2);
          }
        }
        
        p.setBrush(Qt::NoBrush);
        p.setPen(QColor(0,0,0));
        p.drawText(QRectF(x+14,y,ENTRY_WIDTH-10,ENTRY_HEIGHT),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   rowNames[next].c_str());
        if(SYMBOLS_ON){
          p.setPen(s.symbolPen);
          if( 'A' <= s.symbol &&  s.symbol <= 'Z'){
            p.setBrush(s.symbolPen.color());
          }else{
            p.setBrush(Qt::NoBrush);
          }
          const float symX = x+RECT_DIM/2;
          const float symY = y+(ENTRY_HEIGHT-RECT_DIM)/2+RECT_DIM/2;
          const int symSize = s.symbolSize;
#define PLOT_WIDGET_CASE_X(X)                     \
          case X:                                 \
            draw_symbol<X>(p,symSize,symX,symY);  \
            break
#define PLOT_WIDGET_CASE_XY(X,Y)          \
          case Y:                         \
              PLOT_WIDGET_CASE_X(X)
          
          switch(s.symbol){
            PLOT_WIDGET_CASE_X('.');
            PLOT_WIDGET_CASE_X('-');
            PLOT_WIDGET_CASE_X('x');
            PLOT_WIDGET_CASE_X('*');
            PLOT_WIDGET_CASE_X('+');
            PLOT_WIDGET_CASE_XY('s','S');
            PLOT_WIDGET_CASE_XY('t','T');
            PLOT_WIDGET_CASE_XY('o','O');
            PLOT_WIDGET_CASE_XY('d','D');
            case ' ': break;
            default:
              ERROR_LOG("unabled to draw symbol of unknown type " + str("[") + s.symbol + "]");
          }
#undef PLOT_WIDGET_CASE_X
#undef PLOT_WIDGET_CASE_XY
          
        }
        // draws a rect around each line/symbol/fill
        // p.setBrush(Qt::NoBrush);
        // p.setPen(QColor(100,100,100));
        // p.drawRect(x,y+(ENTRY_HEIGHT-RECT_DIM)/2,RECT_DIM,RECT_DIM); 
      }
    }
    p.setPen(QColor(100,100,100));
    p.setBrush(Qt::NoBrush);//BDiagPattern);
    p.drawRect(QRectF(where.x,where.y,where.width,where.height));
    
    

  }
  


  float AbstractPlotWidget::winToDrawX(int winX) const{
    LinearTransform1D t(Range32f(data->lastWindowRect.x, data->lastWindowRect.right()),data->lastXRange);
    return t(winX);
  }
  float AbstractPlotWidget::winToDrawY(int winY) const{
    LinearTransform1D t(Range32f(data->lastWindowRect.bottom(),data->lastWindowRect.y),data->lastYRange);
    return t(winY);
  }
  Point32f AbstractPlotWidget::winToDraw(const Point &p) const{
    return Point32f( winToDrawX(p.x), winToDrawY(p.y));
  }

  int AbstractPlotWidget::drawToWinX(float drawX) const{
    LinearTransform1D t(data->lastXRange, Range32f(data->lastWindowRect.x, data->lastWindowRect.right()));
    return t(drawX);
  }
  int AbstractPlotWidget::drawToWinY(float drawY) const{
    LinearTransform1D t(data->lastYRange, Range32f(data->lastWindowRect.bottom(),data->lastWindowRect.y));
    return t(drawY);
  }
  Point AbstractPlotWidget::drawToWin(const Point32f &p) const{
    return Point(drawToWinX(p.x), drawToWinY(p.y));
  }

  struct AbstractPlotWidget_VIRTUAL : public AbstractPlotWidget{
    virtual bool drawData(QPainter&){ return false; }
  };
  
  static Configurable *create_AbstractPlotWidget_VIRTUAL(){
    if(!dynamic_cast<QApplication*>(QApplication::instance())){
      static const char *args[] = {"app",0};
      static int n = 1;
      static QApplication __static_app(n,(char**)args);
    }
    return new AbstractPlotWidget_VIRTUAL;
  }
  
  REGISTER_CONFIGURABLE(AbstractPlotWidget_VIRTUAL, return create_AbstractPlotWidget_VIRTUAL());

}
