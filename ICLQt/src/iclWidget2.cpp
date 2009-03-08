#include <iclWidget2.h>
#include <iclImg.h>
#include <iclGLTextureMapBaseImage.h>
#include <iclGLPaintEngine.h>
#include <iclFileWriter.h>
#include <iclOSD.h>
#include <string>
#include <vector>
#include <iclTime.h>
#include <iclQImageConverter.h>
#include <QImage>
#include <iclQtPaintEngine.h>

#include <QIcon>
#include <QPixmap>
#include <iclWindowIcon.h>

#include <iclFileWriter.h>
#include <QInputDialog>
#include <iclThread.h>

#include <QMutexLocker>
#include <QPushButton>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSlider>
#include <QSizePolicy>

#include <iclGUI.h>
#include <iclTabHandle.h>
#include <iclBoxHandle.h>
#include <iclComboHandle.h>
#include <iclSliderHandle.h>


#include <iclRect32f.h>
#include <iclRange.h>


using namespace std;
namespace icl{
  class ICLWidget2::OutputBufferCapturer{
  public:
    OutputBufferCapturer(ICLWidget2 *parent):
      // {{{ open

      capturingMode(capturingStopped),parent(parent),filename(""),filewriter(0),mutex(QMutex::Recursive){}

    // }}}

    ~OutputBufferCapturer(){
      // {{{ open

      ICL_DELETE(filewriter);
    }

    // }}}

    std::string getNewFilePattern(const std::string &lastFileName){
      // {{{ open

      bool ok = false;
      QString name = QInputDialog::getText(parent,
                                           "Specify output file pattern",
                                           "Please specify destination file pattern\n"
                                           "(e.g. images/captured_frame_########.ppm",
                                           QLineEdit::Normal,lastFileName.c_str(),&ok);
      if(ok){
        return name.toLatin1().data();
      }else{
        return "";
      }

    }

    // }}}

    void capture(){
      ICLASSERT_RETURN(filewriter);
      QImage qim = parent->grabFrameBuffer();
      converter.setQImage(&qim);
      try{
        filewriter->write(converter.getImg<icl8u>());
      }catch(ICLException &ex){
        ERROR_LOG("error capturing frame buffer: " << ex.what());
      }
    }
    
    void captureIfOn(){
      // {{{ open

      QMutexLocker l(&mutex);
      if(capturingMode == capturingStarted){
        if(frameskip){
          if(frameindex == frameskip){
            capture();
            frameindex = 0;
          }else{
            frameindex++;
          }
        }else{
          capture();
        }
      }
    }

    // }}}
    void setCapturing(ICLWidgetCaptureMode newMode){
      // {{{ open
      QMutexLocker l(&mutex);
      ICLWidgetCaptureMode &oldMode = this->capturingMode;
      if(oldMode == newMode) return;
      switch(newMode){
        case capturingStarted:
          if(this->capturingMode == capturingStopped){
            filename = getNewFilePattern(filename);
            std::string dirname = File(filename).getDir();
            if(!File(dirname).exists()){
              std::string command = string("if [ ! -d ")+dirname+" ] ; then mkdir "+dirname+" ; fi";
              system(command.c_str());
              while(!File(dirname).exists()){
                Thread::msleep(10);
              }
            }
            ICL_DELETE(filewriter);
            try{
              filewriter = new FileWriter(filename);
            }catch(ICLException &ex){
              ERROR_LOG("invalid file pattern:" << filename << std::endl << "Exception:\"" << ex.what() << "\"");
              filewriter = 0;
              filename = "";
              return;
            }            
          }
          this->capturingMode = newMode;
          // else mode becomes paused (noting more)
          break;
        case capturingStopped:
          ICL_DELETE(filewriter);
          this->capturingMode = newMode;
          break;
        case capturingPaused:
          if(oldMode == capturingStarted){
            this->capturingMode = newMode;
          }
          break;
      }
    }

    // }}}

    ICLWidgetCaptureMode getCaptureMode() const {
      // {{{ open

      QMutexLocker l(&mutex);
      return capturingMode;
    }

    // }}}

    std::string getNextCapturingFileName() const{
      // {{{ open

      QMutexLocker l(&mutex);
      if(filewriter){
        return filewriter->getFilenameGenerator().showNext();
      }else{
        return "currently not recording...";
      }
    }

    // }}}

    void setFrameSkip(unsigned int frameskip){
      QMutexLocker l(&mutex);
      this->frameskip = frameskip;
      this->frameindex = 0;
    }
    unsigned int getFrameSkip() const {
      QMutexLocker l(&mutex);
      return frameskip;
    }

    ICLWidgetCaptureMode capturingMode;
    ICLWidget2 *parent;
    string filename;
    FileWriter *filewriter;
    QImageConverter converter;
    mutable QMutex mutex;
    unsigned int frameskip;
    unsigned int frameindex;
  };

  class ZoomAdjustmentWidgetParent;
  
  struct ICLWidget2::Data{
    Data(ICLWidget2 *parent):
      channelSelBuf(0),image(new GLTextureMapBaseImage(0,false)),
      qimageConv(0),qimage(0),mutex(QMutex::Recursive),fm(fmHoldAR),
      rm(rmOff),mouse(-1,-1),selChannel(-1),showNoImageWarnings(true),
      outputCap(new ICLWidget2::OutputBufferCapturer(parent)),menuOn(true),
      menuptr(0),showMenuButton(0),embedMenuButton(0),zoomAdjuster(0)
    {
      for(int i=0;i<3;++i){
        bci[i] = 0;
        downMask[i] = 0;
      }
    }  
    ~Data(){
      ICL_DELETE(channelSelBuf);
      ICL_DELETE(image);
      ICL_DELETE(qimageConv);
      ICL_DELETE(qimage);
      ICL_DELETE(outputCap);
    }

    ImgBase *channelSelBuf;
    GLTextureMapBaseImage *image;
    QImageConverter *qimageConv;
    QImage *qimage;
    QMutex mutex;
    fitmode fm;
    rangemode rm;
    int bci[3];
    bool downMask[3];
    Point mouse;
    int selChannel;
    MouseInteractionInfo mouseInfo;
    bool showNoImageWarnings;
    ICLWidget2::OutputBufferCapturer *outputCap;
    bool menuOn;

    Mutex menuMutex;    
    GUI menu;
    QWidget *menuptr;
    QPushButton *showMenuButton;
    QPushButton *embedMenuButton;
    Rect32f zoomRect;
    ZoomAdjustmentWidgetParent *zoomAdjuster;

    
    void adaptMenuSize(const QSize &parentSize){
      static const int MARGIN = 5;
      static const int TOP_MARGIN = 20;
      int w = parentSize.width()-2*MARGIN;
      int h = parentSize.height()-(MARGIN+TOP_MARGIN);
      
      w = iclMax(menuptr->minimumWidth(),w);
      h = iclMax(menuptr->minimumHeight(),h);
      menuptr->setGeometry(QRect(MARGIN,TOP_MARGIN,w,h));
    }
  };



#define LOCK_SECTION QMutexLocker SECTION_LOCKER(&m_data->mutex)

  namespace{
    
    Rect computeRect(const Size &imageSize, const Size &widgetSize, ICLWidget2::fitmode mode,const Rect32f &relZoomRect=Rect32f::null){
      // {{{ open

      int iImageW = imageSize.width;
      int iImageH = imageSize.height;
      int iW = widgetSize.width;
      int iH = widgetSize.height;
      
      switch(mode){
        case ICLWidget2::fmNoScale:
          // set up the image rect to be centeed
          return Rect((iW -iImageW)/2,(iH -iImageH)/2,iImageW,iImageH); 
          break;
        case ICLWidget2::fmHoldAR:{
          // check if the image is more "widescreen" as the widget or not
          // and adapt image-rect
          float fWidgetAR = (float)iW/(float)iH;
          float fImageAR = (float)iImageW/(float)iImageH;
          if(fImageAR >= fWidgetAR){ //Image is more "widescreen" then the widget
            float fScaleFactor = (float)iW/(float)iImageW;
            return Rect(0,(iH-(int)floor(iImageH*fScaleFactor))/2,
                        (int)floor(iImageW*fScaleFactor),(int)floor(iImageH*fScaleFactor));
          }else{
            float fScaleFactor = (float)iH/(float)iImageH;
            return Rect((iW-(int)floor(iImageW*fScaleFactor))/2,0,
                        (int)floor(iImageW*fScaleFactor),(int)floor(iImageH*fScaleFactor));
          }
          break;
        }
        case ICLWidget2::fmFit:
          // the image is force to fit into the widget
          return Rect(0,0,iW,iH);
          break;
        case ICLWidget2::fmZoom:{
          const Rect32f &rel = relZoomRect;
          float x = (rel.x/rel.width)*widgetSize.width;
          float y = (rel.y/rel.height)*widgetSize.height;
          
          float relRestX = 1.0-rel.right();
          float relRestY = 1.0-rel.bottom();

          float restX = (relRestX/rel.width)*widgetSize.width; 
          float restY = (relRestY/rel.height)*widgetSize.height;
          
          return Rect(round(-x),round(-y),round(x+widgetSize.width+restX),round(y+widgetSize.height+restY));
          break;
        }
      }
      return Rect(0,0,iW,iH);
    }

    // }}}
  }


  struct ZoomAdjustmentWidget : public QWidget{
    Rect32f &r;
    bool downMask[3];
    
    enum Edge { LEFT=0, TOP=1, RIGHT=2, BOTTOM=3 };
    enum LeftDownMode { DRAG, CREATE, NONE };
    bool edgesHovered[4];
    bool edgesDragged[4];
    LeftDownMode mode;
    bool dragAll;

    Point32f dragAllOffs;
    Point32f currPos;
    
    ZoomAdjustmentWidget(QWidget *parent, Rect32f &r):QWidget(parent),r(r){
      downMask[0]=downMask[1]=downMask[2]=false;
      //      r = Rect32f(0,0,width(),height());
      for(int i=0;i<4;++i){
        edgesHovered[i] = edgesDragged[i] = false;
      }
      mode = NONE;
      setMouseTracking(true);
      dragAll = false;
    }

    float xr2a(float rel){
      return rel*width();
    }
    float yr2a(float rel){
      return rel*height();
    }
    float xa2r(float abs){
      return abs/width();
    }
    float ya2r(float abs){
      return abs/height();
    }
    QPointF r2a(const QPointF &rel){
      return QPointF(xr2a(rel.x()),yr2a(rel.y()));
    }
    QPointF a2r(const QPointF &abs){
      return QPointF(xa2r(abs.x()),ya2r(abs.y()));
    }

    bool hit(Edge e,const QPointF &p){
      static const float D = 0.02;
      switch(e){
        case LEFT: 
          if(!Range32f(r.x-D,r.x+D).contains(p.x())) return false;
          if(!Range32f(r.y-D,r.bottom()+D).contains(p.y())) return false;
          return true;
        case RIGHT:
          if(!Range32f(r.right()-D,r.right()+D).contains(p.x())) return false;
          if(!Range32f(r.y-D,r.bottom()+D).contains(p.y())) return false;
          return true;
        case TOP: 
          if(!Range32f(r.y-D,r.y+D).contains(p.y())) return false;
          if(!Range32f(r.x-D,r.right()+D).contains(p.x())) return false;
          return true;
        case BOTTOM: 
          if(!Range32f(r.bottom()-D,r.bottom()+D).contains(p.y())) return false;
          if(!Range32f(r.x-D,r.right()+D).contains(p.x())) return false;
          return true;
      }
      return false;
    }

    bool hitAny(const QPointF &p){
      return hit(LEFT,p) || hit(TOP,p) || hit(RIGHT,p) || hit(BOTTOM,p);
    }
    bool dragAny(){
      return dragAll || edgesDragged[LEFT] || edgesDragged[TOP] || edgesDragged[RIGHT] || edgesDragged[BOTTOM];
    }

    void normalize(){
      r = r.normalized();
      if(r.x < 0) r.x = 0;
      if(r.y < 0) r.y = 0;
      if(r.right() > 1) r.width = 1.0-r.x;
      if(r.bottom() > 1) r.height = 1.0-r.y;
    }

    QPen pen(Edge e){
      return QPen((dragAll||edgesDragged[e])?QColor(255,0,0):QColor(50,50,50),edgesHovered[e] ? 2.5 : 0.5);
    }
    
    virtual void paintEvent(QPaintEvent *e){
      QWidget::paintEvent(e);
      QPainter p(this);
      if(dragAny() || r.contains(currPos.x,currPos.y)){
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(255,0,0),Qt::BDiagPattern));
        p.drawRect(QRectF(width()*r.x,height()*r.y,width()*r.width,height()*r.height));
      }
      
      p.setPen(QColor(50,50,50));
      p.setBrush(Qt::NoBrush);
      p.setRenderHint(QPainter::Antialiasing);
      p.drawRect(QRectF(0,0,width(),height()));
      
      float sx = width();
      float sy = height();
      p.setPen(QColor(50,50,50));

      p.setPen(pen(LEFT));
      p.drawLine(QPointF(sx*r.x,sy*r.y),QPointF(sx*r.x,sy*r.bottom()));

      p.setPen(pen(TOP));
      p.drawLine(QPointF(sx*r.x,sy*r.y),QPointF(sx*r.right(),sy*r.y));

      p.setPen(pen(RIGHT));
      p.drawLine(QPointF(sx*r.right(),sy*r.y),QPointF(sx*r.right(),sy*r.bottom()));

      p.setPen(pen(BOTTOM));
      p.drawLine(QPointF(sx*r.x,sy*r.bottom()),QPointF(sx*r.right(),sy*r.bottom()));
    }

    bool &down(Qt::MouseButton b){
      static bool _ = false;
      switch(b){
        case Qt::LeftButton: return downMask[0];
        case Qt::MidButton: return downMask[1];
        case Qt::RightButton: return downMask[2];
        default: return _;
      }
    }
    
    /// LEFT drag and drop corners / and redraw new rect
    void leftDown(const QPointF &p){
      if(hitAny(p)){
        for(Edge e=LEFT;e<=BOTTOM;((int&)e)++){
          if(hit(e,p)){
            edgesHovered[e] = edgesDragged[e] = true;
          }else{
            edgesHovered[e] = edgesDragged[e] = false;
          }
        }
        mode = DRAG;
      }else{
        r = Rect32f(p.x(),p.y(),0.0001,0.0001);
        mode = CREATE;
      }
    }
    void leftUp(const QPointF &p){
      for(Edge e=LEFT;e<=BOTTOM;((int&)e)++){
        edgesDragged[e] = false;
      }
      mode = NONE;
    }
    
    void leftDrag(const QPointF &p){
      float left=r.x;
      float right=r.right();
      float top = r.y;
      float bottom = r.bottom();
      if(mode == DRAG){
        if(edgesDragged[LEFT])left = p.x();
        else if(edgesDragged[RIGHT])right = p.x();
        if(edgesDragged[TOP])top = p.y();
        else if(edgesDragged[BOTTOM])bottom = p.y();
        r = Rect32f(left,top,right-left,bottom-top);
      }else if(mode == CREATE){
        r = Rect32f(r.x,r.y,p.x()-r.x, p.y()-r.y);
      }else{
        ERROR_LOG("this should not happen!");
      }
      normalize();
    }
    
    void move(const QPointF &p){
      currPos = Point32f(p.x(),p.y());
      for(Edge e=LEFT;e<=BOTTOM;((int&)e)++){
        edgesHovered[e] = hit(e,p);
      }
    }
    
    void rightDown(const QPointF &p){
      if(r.contains(p.x(),p.y())){
        dragAll = true;
        dragAllOffs = r.ul()-Point32f(p.x(),p.y());
      }
    }
    void rightUp(const QPointF &p){
      dragAll = false;
    }
    void rightDrag(const QPointF &p){
      if(dragAll){
        r.x = p.x()+dragAllOffs.x;
        r.y = p.y()+dragAllOffs.y;
      }
    }
    
    
    virtual void mousePressEvent(QMouseEvent *e){
      down(e->button()) = true;
      if(e->button() == Qt::LeftButton){
        leftDown(a2r(e->posF()));
      }else if(e->button() == Qt::RightButton){
        rightDown(a2r(e->posF()));
      }
      update();
    }
    
    virtual void mouseReleaseEvent(QMouseEvent *e){
      down(e->button()) = false;
      if(e->button() == Qt::LeftButton){
        leftUp(a2r(e->posF()));
      }else if(e->button() == Qt::RightButton){
        rightUp(a2r(e->posF()));
      }      
      update();
    }
    
    virtual void mouseMoveEvent(QMouseEvent *e){
      if(downMask[0]){
        leftDrag(a2r(e->posF()));
      }else if(downMask[2]){
        rightDrag(a2r(e->posF()));
      }else{
        move(a2r(e->posF()));
      }
      update();
    }
  };

  struct ZoomAdjustmentWidgetParent : public QWidget{
    Size imageSize;
    ZoomAdjustmentWidget *aw;
    ZoomAdjustmentWidgetParent(const Size &imageSize, QWidget *parent, Rect32f &r):
      QWidget(parent),imageSize(imageSize==Size::null ? Size(100,100): imageSize){
      aw = new ZoomAdjustmentWidget(this,r);
    }
    void updateAWSize(){
      Rect r = computeRect(imageSize,Size(width(),height()), ICLWidget2::fmHoldAR);
      aw->setGeometry(QRect(r.x,r.y,r.width,r.height));
      update();
    }
    
    void resizeEvent(QResizeEvent *e){
      updateAWSize();
    }
    void updateImageSize(const Size &imageSize){
      this->imageSize = imageSize == Size::null ? Size(100,100) : imageSize;
      updateAWSize();
    }    
  };
  

  static void create_menu(ICLWidget2 *widget,ICLWidget2::Data *data){
    Mutex::Locker locker(data->menuMutex);

    // OK, we need to extract default values for all gui elements if gui is already defined!
    data->menu = GUI("tab(bci,scale,channel,capture,info)[@handle=root@minsize=5x7]",widget);

    GUI bciGUI("vbox");

    bciGUI << "combo(off,auto,custom)[@label=bci-mode@handle=bci-mode@out=bci-mode-out]"
           << "slider(-255,255,0)[@handle=brightness@label=brightness@out=_2]"
           << "slider(-255,255,0)[@handle=contrast@label=contrast@out=_1]"
           << "slider(-255,255,0)[@handle=intensity@label=intensity@out=_0]";
    
    GUI scaleGUI("vbox");
    scaleGUI << "combo(hold aspect ratio,force fit,no scale, zoom)[@maxsize=100x2@handle=fit-mode@label=scale mode@out=_3]";
    scaleGUI << "hbox[@handle=scale-widget@minsize=4x3]";

    GUI channelGUI("combo(all,channel #0,channel #1,channel #2, channel #3)[@maxsize=100x2@handle=channel@label=visualized channel@out=_4]");
    
    GUI captureGUI("vsplit");
    
    GUI infoGUI("hsplit");

    data->menu << bciGUI << scaleGUI << channelGUI << captureGUI;
    data->menu.create();

    data->menuptr = data->menu.getRootWidget();
    data->menuptr->setParent(widget);
    
    
    QObject::connect(*data->menu.getValue<ComboHandle>("bci-mode"),SIGNAL(currentIndexChanged(int)),widget,SLOT(bciModeChanged(int)));
    QObject::connect(*data->menu.getValue<ComboHandle>("fit-mode"),SIGNAL(currentIndexChanged(int)),widget,SLOT(scaleModeChanged(int)));
    QObject::connect(*data->menu.getValue<ComboHandle>("channel"),SIGNAL(currentIndexChanged(int)),widget,SLOT(currentChannelChanged(int)));

    QObject::connect(*data->menu.getValue<SliderHandle>("brightness"),SIGNAL(valueChanged(int)),widget,SLOT(brightnessChanged(int)));
    QObject::connect(*data->menu.getValue<SliderHandle>("contrast"),SIGNAL(valueChanged(int)),widget,SLOT(contrastChanged(int)));
    QObject::connect(*data->menu.getValue<SliderHandle>("intensity"),SIGNAL(valueChanged(int)),widget,SLOT(intensityChanged(int)));
    
    data->zoomAdjuster = new ZoomAdjustmentWidgetParent(Size::null,0,data->zoomRect);
    data->menu.getValue<BoxHandle>("scale-widget").add(data->zoomAdjuster);
    /*
        QWidget *scaleBox = *data->menu.getValue<BoxHandle>("scale-widget");
        QWidget *zoomWidget = new ZoomAdjustmentWidget(0);
        zoomWidget->setMinimumSize(QSize(200,200));
        zoomWidget->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum));
        zoomWidget->show();
        //if(scaleBox->layout()){
        //  scaleBox->layout()->addWidget(zoomWidget);
        //}
    */
                     
  }
  
  void update_data(const Size &newImageSize, ICLWidget2::Data *data){
    data->menuMutex.lock();
    if(data->zoomAdjuster->isVisible()){
      data->zoomAdjuster->updateImageSize(newImageSize);
    }
    data->menuMutex.unlock();

  }

  QPushButton *create_top_button(const QString &text,QWidget *parent, int x, int w,bool checkable,bool checked,const char *signal, const char *slot){
    QPushButton *b = new QPushButton(text,parent);
    if(checkable){
      b->setCheckable(true);
      b->setChecked(checked);
    }
    b->setGeometry(QRect(x,-2,w,18));
    b->setAutoFillBackground(false);
    b->setAttribute(Qt::WA_OpaquePaintEvent);
    b->setAttribute(Qt::WA_NoSystemBackground);
    QObject::connect(b,signal,parent,slot);
    return b;
  }

  ICLWidget2::ICLWidget2(QWidget *parent) : 
    m_data(new ICLWidget2::Data(this)){
    
    // TODO (just if mouse interaction receiver is added)
    setMouseTracking(true);
    setWindowIcon(QIcon(QPixmap(ICL_WINDOW_ICON)));
    
    m_data->showMenuButton = create_top_button("menu",this,2,45,false,false,SIGNAL(clicked()),SLOT(showHideMenu()));
    m_data->embedMenuButton = create_top_button("embedded",this,49,65,true,true,SIGNAL(toggled(bool)),SLOT(setMenuEmbedded(bool)));
    create_menu(this,m_data);
  }
  
  // }}}
  ICLWidget2::~ICLWidget2(){
    // {{{ open
    delete m_data; 
  }
  
  // }}}

  
  void ICLWidget2::bciModeChanged(int modeIdx){
    switch(modeIdx){
      case 0: m_data->rm = rmOff; break;
      case 1: m_data->rm = rmAuto; break;
      case 2: m_data->rm = rmOn; break;
      default: ERROR_LOG("invalid range mode index");
    }
  }
  void ICLWidget2::brightnessChanged(int val){
    m_data->bci[0] = val;
  }
  void ICLWidget2::contrastChanged(int val){
    m_data->bci[1] = val;
  }
  void ICLWidget2::intensityChanged(int val){
    m_data->bci[2] = val;
  }
  
  void ICLWidget2::scaleModeChanged(int modeIdx){
    //hold aspect ratio,force fit,no scale, zoom
    switch(modeIdx){
      case 0: m_data->fm = fmHoldAR; break;
      case 1: m_data->fm = fmFit; break;
      case 2: m_data->fm = fmNoScale; break;
      case 3: m_data->fm = fmZoom; break;
      default: ERROR_LOG("invalid scale mode index");
    }
  }
  void ICLWidget2::currentChannelChanged(int modeIdx){
    m_data->selChannel = modeIdx - 1;
  }
  

  void ICLWidget2::showHideMenu(){
    m_data->menuptr->setVisible(!m_data->menuptr->isVisible());
    m_data->adaptMenuSize(size());
  }
  
  void ICLWidget2::setMenuEmbedded(bool embedded){
    bool visible = m_data->menuptr->isVisible();
    if(embedded){
      delete m_data->menuptr;
      create_menu(this,m_data);
      m_data->adaptMenuSize(size());
    }else{
      m_data->menuptr->setParent(0);
      m_data->menuptr->setGeometry(QRect(mapToGlobal(pos())+QPoint(2,2),QSize(width()-4,height()-2)));
    }
    m_data->menuptr->setVisible(visible);
  }


  
  void ICLWidget2::initializeGL(){
    // {{{ open
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glOrtho(0, width(), height(), 0, -999999, 999999);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  // }}}
  void ICLWidget2::resizeGL(int w, int h){
    // {{{ open

    LOCK_SECTION;
    makeCurrent();
    glViewport(0, 0, (GLint)w, (GLint)h);
  }

  // }}}
  void ICLWidget2::paintGL(){
    // {{{ open
    {
      LOCK_SECTION;
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
      if(m_data->image && m_data->image->hasImage()){
        Rect r;
        if(m_data->fm == fmZoom){
          Mutex::Locker locker(m_data->menuMutex);
          r = computeRect(m_data->image->getSize(),getSize(),fmZoom,m_data->zoomAdjuster->aw->r);
        }else{
          r = computeRect(m_data->image->getSize(),getSize(),m_data->fm);
        }
        m_data->image->drawTo(r,getSize());
      }else{
        GLPaintEngine pe(this);
        pe.fill(0,0,0,255);
        Rect fullRect(0,0,width(),height());
        pe.rect(fullRect);
        pe.color(255,255,255,255);
        pe.fill(255,255,255,255);
        
        if(m_data->showNoImageWarnings){
          pe.text(fullRect,"[null]");
        }
      }
    }

    
    GLPaintEngine pe(this);
    {
      LOCK_SECTION;
      /***
          m_oOSDMutex.lock();
          if(m_menuEnabled && m_poCurrOSD){
          float m = iclMin(((float)iclMin(width(),height()))/100,6.0f);
          pe.font("Arial",(int)(1.5*m)+5,PaintEngine::DemiBold);
          m_poCurrOSD->_drawSelf(&pe,m_iMouseX,m_iMouseY,aiDown);
          }
          m_oOSDMutex.unlock();
      **/
      customPaintEvent(&pe);

      /***      
          m_poOutputBufferCapturer->captureIfOn();
          
          
          {
          QMutexLocker l(&m_oFrameBufferCaptureFileNameMutex);
          if(m_sFrameBufferCaptureFileName != ""){
          FileWriter w(m_sFrameBufferCaptureFileName);
          QImage qim = grabFrameBuffer();
          QImageConverter converter(&qim);
          try{
          w.write(converter.getImg<icl8u>());
          }catch(...){
          ERROR_LOG("unable to write framebuffer to file: \"" 
          << m_sFrameBufferCaptureFileName << "\"");
          }
          m_sFrameBufferCaptureFileName = "";
          }
      ***/
    }
    
  }

  // }}}

  // GL case
  void ICLWidget2::paintEvent(QPaintEvent *e){
    // {{{ open

    QGLWidget::paintEvent(e);    
  }

  // }}}

  
  // GL case
  void ICLWidget2::setImage(const ImgBase *image){ 
    // {{{ open

    LOCK_SECTION;
    if(!image){
      m_data->image->updateTextures(0); // in this case [null will be drawn]
      return;
    }

    update_data(image->getSize(),m_data);
    
    if(m_data->rm == rmAuto){
      m_data->image->bci(-1,-1,-1);
    }else if(m_data->rm == rmOn){
      m_data->image->bci(m_data->bci[0],m_data->bci[1],m_data->bci[2]);
    }else{
      m_data->image->bci(0,0,0);
    }

    if(m_data->selChannel >= 0 && m_data->selChannel < image->getChannels()){
      const ImgBase *selectedChannel = image->selectChannel(m_data->selChannel);
      m_data->image->updateTextures(selectedChannel);
      delete selectedChannel;
      if(image != m_data->channelSelBuf){
        image->deepCopy(&m_data->channelSelBuf);
      }
    }else{
      m_data->image->updateTextures(image);
      ICL_DELETE(m_data->channelSelBuf);
    }
  }

  // }}}

  void ICLWidget2::setFitMode(fitmode fm){
    // {{{ open
    m_data->fm = fm;
  }

  // }}}
  void ICLWidget2::setRangeMode(rangemode rm){
    // {{{ open
    m_data->rm = rm;
  }

  // }}}
  void ICLWidget2::setBCI(int brightness, int contrast, int intensity){
    // {{{ open
    m_data->bci[0] = brightness;
    m_data->bci[1] = contrast;
    m_data->bci[2] = intensity;
  }
  // }}}
  void ICLWidget2::customPaintEvent(PaintEngine*){
    // {{{ open
  }
  // }}}

  void ICLWidget2::mousePressEvent(QMouseEvent *e){
    // {{{ open
    switch(e->button()){
      case Qt::LeftButton: m_data->downMask[0]=true; break;
      case Qt::RightButton: m_data->downMask[2]=true; break;
      default: m_data->downMask[1] = true; break;
    }
    
    /*
        m_oOSDMutex.lock();
        if(m_menuEnabled && m_poCurrOSD && m_poCurrOSD->mouseOver(e->x(),e->y())){
        m_iMouseX = e->x();
        m_iMouseY = e->y();
        m_poCurrOSD->_mousePressed(e->x(),e->y(),e->button());
        }else{
        /// emitting signal
        emit mouseEvent(updateMouseInfo(MouseInteractionInfo::pressEvent));
        }
        m_oOSDMutex.unlock();
    */
    
    // TODO only if something really changed!
    // update();
  }
  // }}}

  void ICLWidget2::mouseReleaseEvent(QMouseEvent *e){
    // {{{ open

    switch(e->button()){
      case Qt::LeftButton: m_data->downMask[0]=false; break;
      case Qt::RightButton: m_data->downMask[2]=false; break;
      default: m_data->downMask[1] = false; break;
    }
    /**
        m_oOSDMutex.lock();
        m_iMouseX = e->x();
        m_iMouseY = e->y();
        if(m_menuEnabled && m_poCurrOSD){
        m_poCurrOSD->_mouseReleased(e->x(),e->y(),e->button());
        if(!m_poCurrOSD->mouseOver(e->x(),e->y())){
        /// emitting signal
        emit mouseEvent(updateMouseInfo(MouseInteractionInfo::releaseEvent));
        }
        }
        
        m_oOSDMutex.unlock();
        update();
    **/

  }
  // }}}
  void ICLWidget2::mouseMoveEvent(QMouseEvent *e){
    // {{{ open
    /**
        m_oOSDMutex.lock();
        m_iMouseX = e->x();
        m_iMouseY = e->y();
        if(m_menuEnabled && m_poCurrOSD && m_poCurrOSD->mouseOver(e->x(),e->y())){
        m_poCurrOSD->_mouseMoved(e->x(),e->y(),aiDown);
        }else{
        /// emitting signal
        if(aiDown[0] || aiDown[1] || aiDown[2]){
        emit mouseEvent(updateMouseInfo(MouseInteractionInfo::dragEvent));
        }else{
        emit mouseEvent(updateMouseInfo(MouseInteractionInfo::moveEvent));
        }
        }
        m_oOSDMutex.unlock();
        **/
    // update();
  }
  // }}}
  void ICLWidget2::enterEvent(QEvent *e){
    // {{{ open

    m_data->showMenuButton->show();
    m_data->embedMenuButton->show();
    // TODO show the new osd on/off button here!
    /**
        (void)e;
        m_oOSDMutex.lock();
        m_iMouseX = -1;
        m_iMouseY = -1;
        if(!m_poOSD){
        m_poOSD = new OSD(0,Rect(3,3,width()-10,height()-10),this,0);
        m_poShowOSD = new OSDButton(SHOW_OSD_ID,Rect(width()-80,height()-23,75,18),this,0,"menu");
        
        }
        if(m_poCurrOSD == 0){
        m_poCurrOSD = m_poShowOSD;
        }
        m_oOSDMutex.unlock();
        update();
        emit mouseEvent(updateMouseInfo(MouseInteractionInfo::enterEvent));
    **/
  }
  void ICLWidget2::setVisible(bool visible){
    QGLWidget::setVisible(visible);
    m_data->showMenuButton->setVisible(false);
    m_data->embedMenuButton->setVisible(false);
    m_data->menuptr->setVisible(false);
    m_data->adaptMenuSize(size());
  }

  // }}}
  void ICLWidget2::leaveEvent(QEvent*){
    // {{{ open
    m_data->showMenuButton->hide();
    m_data->embedMenuButton->hide();

    // TODO hide the new OSD here

    /**
        if(m_menuEnabled){
        m_oOSDMutex.lock();
        if(m_poCurrOSD == m_poShowOSD){
        m_poCurrOSD = 0;
        }
        m_oOSDMutex.unlock();
        }
        
        m_iMouseX = -1;
        m_iMouseY = -1;
        
        update();
        emit mouseEvent(updateMouseInfo(MouseInteractionInfo::leaveEvent));
     **/
  }

  // }}}
  void ICLWidget2::resizeEvent(QResizeEvent *e){
    // {{{ open
    resizeGL(e->size().width(),e->size().height());
    m_data->adaptMenuSize(size());

    // Menu should adapt itself using a Qt-Layout manager
    /** 
        if(!m_menuEnabled) return;
        m_oOSDMutex.lock();
        if(m_poOSD && isVisible()){
        QSize s = e->size();
        int iLastOSD = 0; // none
        if(m_poCurrOSD == m_poOSD) iLastOSD = 1; // osd
        else if(m_poCurrOSD == m_poShowOSD) iLastOSD = 2; // show osd
        m_poCurrOSD = 0;
        
        int iLastShownID = ((OSD*)m_poOSD)->getCurrID();
        delete m_poOSD;
        delete m_poShowOSD;
        m_poOSD = new OSD(0,Rect(3,3,s.width()-10,s.height()-10),this,0);
        m_poShowOSD = new OSDButton(SHOW_OSD_ID,Rect(s.width()-80,s.height()-23,75,18),this,0,"menu");
        ((OSD*)m_poOSD)->setCurrID(iLastShownID);
        
        if(iLastOSD == 1) m_poCurrOSD = m_poOSD;
        else if(iLastOSD == 2) m_poCurrOSD = m_poShowOSD;
        }
        m_oOSDMutex.unlock();
     **/
  }
  // }}}

  void ICLWidget2::updateFromOtherThread(){
    // {{{ open

    QApplication::postEvent(this,new QEvent(QEvent::User),Qt::HighEventPriority);
  }

  // }}}

  //  void ICLWidget2::setMenuEnabled(bool enabled){
    // TODO
    /**
        if(!enabled){
        m_oOSDMutex.lock();
        ICL_DELETE(m_poOSD);
        ICL_DELETE(m_poShowOSD);
        m_poCurrOSD = 0;
        m_oOSDMutex.unlock();
        }
        
        m_menuEnabled = enabled;

  }
    **/

  void ICLWidget2::rebufferImageInternal(){
    // {{{ open
    LOCK_SECTION;
    if(m_data->image && m_data->image->hasImage()){
      if(m_data->channelSelBuf){
        setImage(m_data->channelSelBuf);
      }else{
        ImgBase *tmpImage = m_data->image->deepCopy();
        setImage(tmpImage);
        delete tmpImage;
      }
    }
  }

  // }}}

  void ICLWidget2::childChanged(int id, void *val){  
    // {{{ open
    // this is really deprecated now!
    /***
    switch(id){
      case SHOW_OSD_ID:
        m_poCurrOSD = m_poOSD;
        break;
      case OSD::BOTTOM_EXIT_ID:
        m_poCurrOSD = m_poShowOSD;
        break;
      case OSD::FITMODE_NOSCALE_ID:
        m_eFitMode = fmNoScale;
        break;
      case OSD::FITMODE_HOLDAR_ID:
        m_eFitMode = fmHoldAR;
        break;
      case OSD::FITMODE_FIT_ID:
        m_eFitMode = fmFit;
        break;
      case OSD::ADJUST_BRIGHTNESS_SLIDER_ID:
        m_aiBCI[0] = *(int*)val;
        rebufferImageInternal();
        break;
      case OSD::ADJUST_CONTRAST_SLIDER_ID:
        m_aiBCI[1] = *(int*)val;
        rebufferImageInternal();
        break;
      case OSD::ADJUST_INTENSITY_SLIDER_ID:
        m_aiBCI[2] = *(int*)val;
        rebufferImageInternal();
        break;
      case OSD::ADJUST_MODE_NONE_ID:
        m_eRangeMode = rmOff;
        rebufferImageInternal();
        break;
      case OSD::ADJUST_MODE_MANUAL_ID:
        m_eRangeMode = rmOn;
        rebufferImageInternal();
        break;
      case OSD::ADJUST_MODE_AUTO_ID:
        m_eRangeMode = rmAuto;
        rebufferImageInternal();
        break;
      case OSD::CHANNELS_SLIDER_ID:
        m_iCurrSelectedChannel = *(int*)val;
        rebufferImageInternal();
        break;
      case OSD::CAPTURE_BUTTON_ID:{
        // capturing current image:
        m_oMutex.lock();  
        ImgBase *buf = 0;
        if(m_poImage){
          buf = m_poImage->deepCopy();
        }
        m_oMutex.unlock();  
        if(buf){
          string t = Time::now().toString();
          for(unsigned int i=0;i<t.length();i++){
            if(t[i]=='/')t[i]='.';
            if(t[i]==' ')t[i]='_';
          }
          FileWriter(string("./snapshot_[")+t+string("].pnm")).write(buf);
          delete buf;
        }
        break;
      }
      case OSD::CAPTURE_VIDEO_START_BUTTON_ID:
        m_poOutputBufferCapturer->setCapturing(capturingStarted);
        break;
      case OSD::CAPTURE_VIDEO_STOP_BUTTON_ID:
        m_poOutputBufferCapturer->setCapturing(capturingStopped);
        break;
      case OSD::CAPTURE_VIDEO_PAUSE_BUTTON_ID:
        m_poOutputBufferCapturer->setCapturing(capturingPaused);
        break;
      case OSD::CAPTURE_VIDEO_FRAME_SKIP_SLIDER_ID:
        m_poOutputBufferCapturer->setFrameSkip(*reinterpret_cast<int*>(val));
      default:
        break;
    }

    update();
    **/
  }

  // }}}
 
  std::vector<std::string> ICLWidget2::getImageInfo(){
    // {{{ open
    std::vector<string> info;

    // TODO adapt ....
    /**
        GLTextureMapBaseImage* i = m_poImage;
        if(!i || !i->hasImage()){
        info.push_back("Image is NULL");
        return info;
        }
        info.push_back(string("depth:   ")+translateDepth(m_poImage->getDepth()).c_str());
        info.push_back(string("size:    ")+QString::number(i->getSize().width).toLatin1().data()+" x "+
        QString::number(i->getSize().height).toLatin1().data());
        info.push_back(string("channels:")+QString::number(i->getChannels()).toLatin1().data());
        info.push_back(string("format:  ")+translateFormat(i->getFormat()).c_str());
        if(i->getROI() == Rect(Point::null,i->getSize())){
        info.push_back("roi:   full");
        }else{
        char ac[200];
        Rect r = i->getROI();
        sprintf(ac,"roi:   ((%d,%d),(%d x %d))",r.x,r.y,r.width,r.height);
        info.push_back(ac);
        }
        
        std::vector<Range<icl32f> > ranges = i->getMinMax();
        for(int a=0;a<i->getChannels();a++){
        char ac[200];
        Range<icl32f> r = ranges[a];
        sprintf(ac,"channel %d, min:%f, max:%f",a,r.minVal,r.maxVal);
        info.push_back(ac);
        }
        return info;
     **/
  }

  // }}}

  Size ICLWidget2::getImageSize(){
    // {{{ open
    LOCK_SECTION;
    Size s;
    if(m_data->image){
      s = m_data->image->getSize(); 
    }else{
      s = Size(width(),height());
    }
    return s;
  }

  // }}}
  Rect ICLWidget2::getImageRect(){
    // {{{ open

    return computeRect(getImageSize(),Size(width(),height()),m_data->fm);
  }

  // }}}
  MouseInteractionInfo *ICLWidget2::updateMouseInfo(MouseInteractionInfo::Type type){
    // {{{ open
    
    // TODO implement ...
    /**
        if(!m_poImage || !m_poImage->hasImage() ){
        return &m_oMouseInfo;
        }
        m_oMouseInfo.type = type;
        m_oMouseInfo.widgetX = m_iMouseX;
        m_oMouseInfo.widgetY = m_iMouseY;
        
        memcpy(m_oMouseInfo.downmask,aiDown,3*sizeof(int));
        
        m_oMutex.lock();
        Rect r = computeRect(m_poImage->getSize(), Size(width(),height()), m_eFitMode);
        //if(m_poImage && op.on && r.contains(m_iMouseX, m_iMouseY)){
        //    if(r.contains(m_iMouseX, m_iMouseY)){
        float boxX = m_iMouseX - r.x;
        float boxY = m_iMouseY - r.y;
        m_oMouseInfo.imageX = (int) rint((boxX*(m_poImage->getSize().width))/r.width);
        m_oMouseInfo.imageY = (int) rint((boxY*(m_poImage->getSize().height))/r.height);
        if(r.contains(m_iMouseX,m_iMouseY)){
        m_oMouseInfo.color = m_poImage->getColor(m_oMouseInfo.imageX,m_oMouseInfo.imageY);
        }else{
        //      m_oMouseInfo.imageX = -1;
        //m_oMouseInfo.imageY = -1;
        m_oMouseInfo.color.resize(0);
        }
        m_oMouseInfo.relImageX = float(m_oMouseInfo.imageX)/m_poImage->getSize().width;
        m_oMouseInfo.relImageY = float(m_oMouseInfo.imageY)/m_poImage->getSize().height;
        m_oMutex.unlock();
        return &m_oMouseInfo;
    **/
  }

  // }}}

  const ImageStatistics &ICLWidget2::getImageStatistics() {
    // {{{ open

    if(m_data->image){
      return m_data->image->getStatistics();
    }else{
      static ImageStatistics xxx;
      xxx.isNull = true;
      return xxx;
    }
  }



}
