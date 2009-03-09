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
#include <iclThread.h>

#include <QInputDialog>
#include <QMutexLocker>
#include <QPushButton>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSlider>
#include <QSizePolicy>
#include <QFileDialog>
#include <QSpinBox>

#include <iclGUI.h>
#include <iclTabHandle.h>
#include <iclBoxHandle.h>
#include <iclButtonHandle.h>
#include <iclComboHandle.h>
#include <iclStringHandle.h>
#include <iclSpinnerHandle.h>
#include <iclSliderHandle.h>
#include <iclStringUtils.h>

#include <iclRect32f.h>
#include <iclRange.h>
#include <iclStackTimer.h>
#include <iclTimer.h>

using namespace std;
namespace icl{

#define LOCK_SECTION QMutexLocker SECTION_LOCKER(&m_data->mutex)

  class ZoomAdjustmentWidgetParent;
  
  struct ICLWidget2::Data{
    // {{{ open

    Data(ICLWidget2 *parent):
      channelSelBuf(0),image(new GLTextureMapBaseImage(0,false)),
      qimageConv(0),qimage(0),mutex(QMutex::Recursive),fm(fmHoldAR),
      rm(rmOff),bciUpdateAuto(false),channelUpdateAuto(false),
      mouse(-1,-1),selChannel(-1),showNoImageWarnings(true),
      outputCap(0),menuOn(true),
      menuptr(0),showMenuButton(0),embedMenuButton(0),zoomAdjuster(0),
      qic(0),menuEnabled(true),infoTab(0)
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
      ICL_DELETE(qic);
      // ICL_DELETE(outputCap); this must be done by the parent widget
    }

    ImgBase *channelSelBuf;
    GLTextureMapBaseImage *image;
    QImageConverter *qimageConv;
    QImage *qimage;
    QMutex mutex;
    fitmode fm;
    rangemode rm;
    int bci[3];
    bool *bciUpdateAuto;
    bool *channelUpdateAuto;
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
    QImageConverter *qic;
    bool menuEnabled;
    QWidget *infoTab;
    
    void adaptMenuSize(const QSize &parentSize){
      if(!menuptr) return;
      static const int MARGIN = 5;
      static const int TOP_MARGIN = 20;
      int w = parentSize.width()-2*MARGIN;
      int h = parentSize.height()-(MARGIN+TOP_MARGIN);
      
      w = iclMax(menuptr->minimumWidth(),w);
      h = iclMax(menuptr->minimumHeight(),h);
      menuptr->setGeometry(QRect(MARGIN,TOP_MARGIN,w,h));
    }
  };

  // }}}

  struct ICLWidget2::OutputBufferCapturer{
    // {{{ open

    ICLWidget2 *parent;
    ICLWidget2::Data *data;
    
    bool recording;
    bool paused;
    enum CaptureTarget { SET_IMAGES, FRAME_BUFFER };
    CaptureTarget target;
    Mutex mutex;
    FileWriter *fileWriter;
    int frameSkip;
    int frameIdx;
  public:
    OutputBufferCapturer(ICLWidget2 *parent, ICLWidget2::Data *data):
      parent(parent),data(data),target(SET_IMAGES),fileWriter(0),
      frameSkip(0),frameIdx(0){}
    
    ~OutputBufferCapturer(){
      ICL_DELETE(fileWriter);
    }

    void ensureDirExists(const QString &dirName){
      if(!dirName.length()){
        throw ICLException("empty file pattern");
      }
      if(dirName[0] == '/'){
        QDir("/").mkpath(dirName);
      }else{
        QDir("./").mkpath(dirName);
      }
    }
    bool startRecording(CaptureTarget t, const std::string &filePattern, int frameskip){
      Mutex::Locker l(mutex);
      target = t;
      ICL_DELETE(fileWriter);
      try{
        fileWriter = new FileWriter(filePattern);
        ensureDirExists(File(filePattern).getDir().c_str());
      }catch(ICLException &ex){
        ERROR_LOG("unable to create filewriter with this pattern: " << filePattern << "\nerror:"<< ex.what());
      }
      recording = true;
    }
    bool setPaused(bool val){
      Mutex::Locker l(mutex);
      paused = val;
    }
    bool stopRecording(){
      Mutex::Locker l(mutex);
      recording = false;
    }
    
    void captureImageHook(){
      Mutex::Locker l(mutex);
      if(!recording || paused || (target != SET_IMAGES) ) return;
      ICLASSERT_RETURN(fileWriter);
      DEBUG_LOG("here!");
      if(frameIdx < frameSkip){
        frameIdx++;
        return;
      }else{
        frameIdx = 0;
      }
      try{
        ImgBase *image = data->image->deepCopy();
        fileWriter->write(image);
        delete image;
      }catch(ICLException &ex){
        ERROR_LOG("unable to capture current image:" << ex.what());
      }
    }

    void captureFrameBufferHook(){
      Mutex::Locker l(mutex);
      if(!recording || paused || (target != FRAME_BUFFER)) return;
      ICLASSERT_RETURN(fileWriter);
      DEBUG_LOG("here!");

      if(frameIdx < frameSkip){
        frameIdx++;
        return;
      }else{
        frameIdx = 0;
      }
      try{
        const Img8u &fb = parent->grabFrameBufferICL();
        fileWriter->write(&fb);
      }catch(ICLException &ex){
        ERROR_LOG("unable to capture frame buffer:" << ex.what());
      }
    }
    std::string getNextFileName(){
      Mutex::Locker l(mutex);
      if(fileWriter){
        return fileWriter->getFilenameGenerator().showNext();
      }else{
        return "currently not recording...";
      }
    }
  };

  // }}}
  
  static Rect computeRect(const Size &imageSize, const Size &widgetSize, ICLWidget2::fitmode mode,const Rect32f &relZoomRect=Rect32f::null){
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

  struct ZoomAdjustmentWidget : public QWidget{
    // {{{ open

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
    
    QWidget *parentICLWidget;
    
    ZoomAdjustmentWidget(QWidget *parent, Rect32f &r, QWidget *parentICLWidget):
      QWidget(parent),r(r),parentICLWidget(parentICLWidget),mode(NONE),dragAll(false){
      downMask[0]=downMask[1]=downMask[2]=false;
      //      r = Rect32f(0,0,width(),height());
      for(int i=0;i<4;++i){
        edgesHovered[i] = edgesDragged[i] = false;
      }
      setMouseTracking(true);
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
        parentICLWidget->update();
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
      parentICLWidget->update();
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
        parentICLWidget->update();
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

  // }}}

  struct ZoomAdjustmentWidgetParent : public QWidget{
    // {{{ open

    Size imageSize;
    ZoomAdjustmentWidget *aw;
    ZoomAdjustmentWidgetParent(const Size &imageSize, QWidget *parent, Rect32f &r, QWidget *parentICLWidget):
      QWidget(parent),imageSize(imageSize==Size::null ? Size(100,100): imageSize){
      aw = new ZoomAdjustmentWidget(this,r,parentICLWidget);
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

  // }}}

  static void create_menu(ICLWidget2 *widget,ICLWidget2::Data *data){
    // {{{ open

    Mutex::Locker locker(data->menuMutex);

    if(data->menuptr){
      QObject::disconnect(*data->menu.getValue<ComboHandle>("bci-mode"),SIGNAL(currentIndexChanged(int)),widget,SLOT(bciModeChanged(int)));
      QObject::disconnect(*data->menu.getValue<ComboHandle>("fit-mode"),SIGNAL(currentIndexChanged(int)),widget,SLOT(scaleModeChanged(int)));
      QObject::disconnect(*data->menu.getValue<ComboHandle>("channel"),SIGNAL(currentIndexChanged(int)),widget,SLOT(currentChannelChanged(int)));
      
      QObject::disconnect(*data->menu.getValue<SliderHandle>("brightness"),SIGNAL(valueChanged(int)),widget,SLOT(brightnessChanged(int)));
      QObject::disconnect(*data->menu.getValue<SliderHandle>("contrast"),SIGNAL(valueChanged(int)),widget,SLOT(contrastChanged(int)));
      QObject::disconnect(*data->menu.getValue<SliderHandle>("intensity"),SIGNAL(valueChanged(int)),widget,SLOT(intensityChanged(int)));
      
      
      QObject::disconnect(*data->menu.getValue<ButtonHandle>("cap-image"),SIGNAL(clicked()),widget,SLOT(captureCurrentImage()));
      QObject::disconnect(*data->menu.getValue<ButtonHandle>("cap-fb"),SIGNAL(clicked()),widget,SLOT(captureCurrentFrameBuffer()));
      

      QObject::disconnect(*data->menu.getValue<ButtonHandle>("auto-cap-record"),SIGNAL(toggled(bool)),widget,SLOT(recordButtonToggled(bool)));
      QObject::disconnect(*data->menu.getValue<ButtonHandle>("auto-cap-pause"),SIGNAL(toggled(bool)),widget,SLOT(pauseButtonToggled(bool)));
      QObject::disconnect(*data->menu.getValue<ButtonHandle>("auto-cap-stop"),SIGNAL(clicked()),widget,SLOT(stopButtonClicked()));
      QObject::disconnect(*data->menu.getValue<ComboHandle>("auto-cap-mode"),SIGNAL(currentIndexChanged(int)),widget,SLOT(stopButtonClicked()));
      QObject::disconnect(*data->menu.getValue<SpinnerHandle>("auto-cap-frameskip"),SIGNAL(valueChanged(int)),widget,SLOT(skipFramesChanged(int)));

    }

    // OK, we need to extract default values for all gui elements if gui is already defined!
    data->menu = GUI("tab(bci,scale,channel,capture,info)[@handle=root@minsize=5x7]",widget);

    GUI bciGUI("vbox");

    bciGUI << ( GUI("hbox")
                << "combo(off,auto,custom)[@label=bci-mode@handle=bci-mode@out=bci-mode-out]"
                << "togglebutton(manual,auto)[@label=update mode@out=bci-update-mode]"
              )
           << "slider(-255,255,0)[@handle=brightness@label=brightness@out=_2]"
           << "slider(-255,255,0)[@handle=contrast@label=contrast@out=_1]"
           << "slider(-255,255,0)[@handle=intensity@label=intensity@out=_0]";
    
    GUI scaleGUI("vbox");
    scaleGUI << "combo(hold aspect ratio,force fit,no scale, zoom)[@maxsize=100x2@handle=fit-mode@label=scale mode@out=_3]";
    scaleGUI << "hbox[@handle=scale-widget@minsize=4x3]";

    GUI channelGUI("vbox");
    channelGUI << "combo(all,channel #0,channel #1,channel #2, channel #3)[@maxsize=100x2@handle=channel@label=visualized channel@out=_4]"
               << "togglebutton(manual,auto)[@maxsize=100x2@label=update mode@out=channel-update-mode]";

    GUI captureGUI("vbox");
     
    captureGUI << ( GUI("hbox[@label=single shot]")
                    << "button(image)[@handle=cap-image]"
                    << "button(frame buffer)[@handle=cap-fb]"
                    << "combo(image.pnm,image_TIME.pnm,ask me)[@label=filename@handle=cap-filename@out=_5]"
                   );
    
    GUI autoCapGUI("vbox[@label=automatic]");
    autoCapGUI << ( GUI("hbox")
                    << "combo(image,frame buffer)[@label=mode@handle=auto-cap-mode@out=_6]"
                    << "spinner(0,100,0)[@label=frame skip@handle=auto-cap-frameskip@out=_10]"
                    << "string(captured/image_####.ppm,100)[@label=file pattern@handle=auto-cap-filepattern@out=_7]"
                  )
               << ( GUI("hbox")
                    << "togglebutton(record,record)[@handle=auto-cap-record@out=_8]"
                    << "togglebutton(pause,pause)[@handle=auto-cap-pause@out=_9]"
                    << "button(stop)[@handle=auto-cap-stop]"
                  );
    captureGUI << autoCapGUI;    
    
    GUI infoGUI("vbox[@handle=info-tab]");
    infoGUI << ( GUI("hbox[@minsize=5x2@maxsize=100x2]")
                 << "combo(all ,channel #0, channel #1,channel #2, channel #3)[@handle=histo-channel@out=_15]"
                 << "togglebutton(median,median)[@handle=median@out=_11]"
                 << "togglebutton(log,log)[@handle=log@out=_12]"
                 << "togglebutton(blur,blur)[@handle=blur@out=_13]"
                 << "togglebutton(fill,fill)[@handle=fill@out=_14]"
               )
            <<  ( GUI("hsplit")
                  << "hbox[@label=histogramm@minsize=2x2]"
                  << "label[@label=params@minsize=2x3]"
                );

    data->menu << bciGUI << scaleGUI << channelGUI << captureGUI << infoGUI;

    data->menu.create();

    data->menuptr = data->menu.getRootWidget();
    data->menuptr->setParent(widget);
    
    data->bciUpdateAuto = &data->menu.getValue<bool>("bci-update-mode");
    data->channelUpdateAuto = &data->menu.getValue<bool>("channel-update-mode");

    data->infoTab = *data->menu.getValue<BoxHandle>("info-tab");
    
    QObject::connect(*data->menu.getValue<ComboHandle>("bci-mode"),SIGNAL(currentIndexChanged(int)),widget,SLOT(bciModeChanged(int)));
    QObject::connect(*data->menu.getValue<ComboHandle>("fit-mode"),SIGNAL(currentIndexChanged(int)),widget,SLOT(scaleModeChanged(int)));
    QObject::connect(*data->menu.getValue<ComboHandle>("channel"),SIGNAL(currentIndexChanged(int)),widget,SLOT(currentChannelChanged(int)));

    QObject::connect(*data->menu.getValue<SliderHandle>("brightness"),SIGNAL(valueChanged(int)),widget,SLOT(brightnessChanged(int)));
    QObject::connect(*data->menu.getValue<SliderHandle>("contrast"),SIGNAL(valueChanged(int)),widget,SLOT(contrastChanged(int)));
    QObject::connect(*data->menu.getValue<SliderHandle>("intensity"),SIGNAL(valueChanged(int)),widget,SLOT(intensityChanged(int)));
    
    data->zoomAdjuster = new ZoomAdjustmentWidgetParent(Size::null,0,data->zoomRect,widget);
    data->menu.getValue<BoxHandle>("scale-widget").add(data->zoomAdjuster);

    QObject::connect(*data->menu.getValue<ButtonHandle>("cap-image"),SIGNAL(clicked()),widget,SLOT(captureCurrentImage()));
    QObject::connect(*data->menu.getValue<ButtonHandle>("cap-fb"),SIGNAL(clicked()),widget,SLOT(captureCurrentFrameBuffer()));
    

    QObject::connect(*data->menu.getValue<ButtonHandle>("auto-cap-record"),SIGNAL(toggled(bool)),widget,SLOT(recordButtonToggled(bool)));
    QObject::connect(*data->menu.getValue<ButtonHandle>("auto-cap-pause"),SIGNAL(toggled(bool)),widget,SLOT(pauseButtonToggled(bool)));
    QObject::connect(*data->menu.getValue<ButtonHandle>("auto-cap-stop"),SIGNAL(clicked()),widget,SLOT(stopButtonClicked()));
    QObject::connect(*data->menu.getValue<ComboHandle>("auto-cap-mode"),SIGNAL(currentIndexChanged(int)),widget,SLOT(stopButtonClicked()));
    QObject::connect(*data->menu.getValue<SpinnerHandle>("auto-cap-frameskip"),SIGNAL(valueChanged(int)),widget,SLOT(skipFramesChanged(int)));
    
    QObject::connect(*data->menu.getValue<TabHandle>("root"),SIGNAL(currentChanged(int)),widget,SLOT(menuTabChanged(int)));
  }

  // }}}
  
  void update_data(const Size &newImageSize, ICLWidget2::Data *data){
    // {{{ open

    data->menuMutex.lock();
    if(data->menuptr){
      if(data->zoomAdjuster->isVisible()){
        data->zoomAdjuster->updateImageSize(newImageSize);
      }
    }
    data->menuMutex.unlock();
  }

  // }}}

  QPushButton *create_top_button(const QString &text,QWidget *parent, int x, int w,bool checkable,bool checked,const char *signal, const char *slot){
    // {{{ open

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

  // }}}

  // ------------ ICLWidget2 ------------------------------

  ICLWidget2::ICLWidget2(QWidget *parent) : 
    // {{{ open

    m_data(new ICLWidget2::Data(this)){
    
    // TODO (just if mouse interaction receiver is added)
    setMouseTracking(true);
    setWindowIcon(QIcon(QPixmap(ICL_WINDOW_ICON)));
    
    m_data->showMenuButton = create_top_button("menu",this,2,45,false,false,SIGNAL(clicked()),SLOT(showHideMenu()));
    m_data->embedMenuButton = create_top_button("embedded",this,49,65,true,true,SIGNAL(toggled(bool)),SLOT(setMenuEmbedded(bool)));
    //create_menu(this,m_data);
  }

  // }}}
  
  ICLWidget2::~ICLWidget2(){
    // {{{ open

    ICL_DELETE(m_data->outputCap);// just because of the classes definition order 
    delete m_data; 
  }

  // }}}
  
  void ICLWidget2::bciModeChanged(int modeIdx){
    // {{{ open

    switch(modeIdx){
      case 0: m_data->rm = rmOff; break;
      case 1: m_data->rm = rmAuto; break;
      case 2: m_data->rm = rmOn; break;
      default: ERROR_LOG("invalid range mode index");
    }
    if(*m_data->bciUpdateAuto){
      rebufferImageInternal();
    }
  }

  // }}}

  void ICLWidget2::brightnessChanged(int val){
    // {{{ open

    m_data->bci[0] = val;
    if(*m_data->bciUpdateAuto){
      rebufferImageInternal();
    }
  }

  // }}}

  void ICLWidget2::contrastChanged(int val){
    // {{{ open

    m_data->bci[1] = val;
    if(*m_data->bciUpdateAuto){
      rebufferImageInternal();
    }
  }

  // }}}

  void ICLWidget2::intensityChanged(int val){
    // {{{ open

    m_data->bci[2] = val;
    if(*m_data->bciUpdateAuto){
      rebufferImageInternal();
    }
  }

  // }}}
  
  void ICLWidget2::scaleModeChanged(int modeIdx){
    // {{{ open

    //hold aspect ratio,force fit,no scale, zoom
    switch(modeIdx){
      case 0: m_data->fm = fmHoldAR; break;
      case 1: m_data->fm = fmFit; break;
      case 2: m_data->fm = fmNoScale; break;
      case 3: m_data->fm = fmZoom; break;
      default: ERROR_LOG("invalid scale mode index");
    }
    update();
  }

  // }}}

  void ICLWidget2::currentChannelChanged(int modeIdx){
    // {{{ open

    m_data->selChannel = modeIdx - 1;
    if(*m_data->channelUpdateAuto){
      rebufferImageInternal();
    }
  }

  // }}}
  

  void ICLWidget2::showHideMenu(){
    // {{{ open

    if(!m_data->menuptr){
      create_menu(this,m_data);
    }
    m_data->menuptr->setVisible(!m_data->menuptr->isVisible());

    m_data->adaptMenuSize(size());
  }

  // }}}
  void ICLWidget2::setMenuEmbedded(bool embedded){
    // {{{ open

    if(!m_data->menuptr){
      create_menu(this,m_data);
    }

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

  // }}}

  void ICLWidget2::recordButtonToggled(bool checked){
    // {{{ open

    if(!m_data->outputCap){
      m_data->outputCap = new OutputBufferCapturer(this,m_data);
    }
    if(checked){
      Mutex::Locker l(m_data->menuMutex);
      OutputBufferCapturer::CaptureTarget t = m_data->menu.getValue<ComboHandle>("auto-cap-mode").getSelectedIndex()?
                                       OutputBufferCapturer::SET_IMAGES :  OutputBufferCapturer::FRAME_BUFFER;

      const std::string filePattern = m_data->menu.getValue<StringHandle>("auto-cap-filepattern").getCurrentText();
      int frameSkip = m_data->menu.getValue<SpinnerHandle>("auto-cap-frameskip").getValue(); //xxx
      m_data->outputCap->startRecording(t,filePattern,frameSkip);
    }else{
      m_data->outputCap->stopRecording();
    }
  }

  // }}}

  void ICLWidget2::pauseButtonToggled(bool checked){
    // {{{ open

    if(!m_data->outputCap){
      m_data->outputCap = new OutputBufferCapturer(this,m_data);
    }
    m_data->outputCap->setPaused(checked);
  }

  // }}}

  void ICLWidget2::stopButtonClicked(){
    // {{{ open

    m_data->outputCap->stopRecording();
    Mutex::Locker l(m_data->menuMutex);
    (*m_data->menu.getValue<ButtonHandle>("auto-cap-record"))->setChecked(false);
  }

  // }}}
  
  void ICLWidget2::skipFramesChanged(int frameSkip){
    // {{{ open

    if(!m_data->outputCap){
      m_data->outputCap->frameSkip = frameSkip;
    }
  }

  // }}}

  void ICLWidget::menuTabChanged(int index){
    // {{{ open

    if(index == 4){
      updateInfoTab();
    }
  }

  // }}}

  void ICLWidget2::updateInfoTab(){
    // {{{ open

    // xxx TODO
  }

  // }}}
  
  std::string ICLWidget2::getImageCaptureFileName(){
    // {{{ open

    Mutex::Locker l(m_data->menuMutex);
    ComboHandle &h = m_data->menu.getValue<ComboHandle>("cap-filename");
    std::string filename="image.pnm";
    switch(h.getSelectedIndex()){
      case 1:{
        string t = Time::now().toString();
        for(unsigned int i=0;i<t.length();i++){
          if(t[i]=='/')t[i]='.';
          if(t[i]==' ')t[i]='_';
        }
        filename = str("image_")+t+str(".pnm");
        break;
      }
      case 2:{
        QString name = QFileDialog::getSaveFileName(this,"filename ...","./",
                                                    "common formats (*.pnm *.ppm *.pgm *.jpg *.png)\nall files (*.*)");
        filename = name.toLatin1().data();
        break;
      }
      default:
        break;
    }
    return filename;
  }

  // }}}

  void ICLWidget2::captureCurrentImage(){
    // {{{ open

    ImgBase *buf = 0;
    {
      LOCK_SECTION;
      if(m_data->image){
        buf = m_data->image->deepCopy();
      }
    }

    if(buf){
      std::string filename = getImageCaptureFileName();
      if(filename != ""){
        try{
          FileWriter(filename).write(buf);          
        }catch(const ICLException &ex){
          ERROR_LOG("unable to capture current image: " << ex.what());
        }
      }
      delete buf;
    }
  }

  // }}}

  const Img8u &ICLWidget2::grabFrameBufferICL(){
    // {{{ open

    if(!m_data->qic){
      m_data->qic = new QImageConverter;
    }
    QImage qim = grabFrameBuffer();
    m_data->qic->setQImage(&qim);
    return *m_data->qic->getImg<icl8u>();
  }

  // }}}
  
  void ICLWidget2::captureCurrentFrameBuffer(){
    // {{{ open

    const Img8u &fb = grabFrameBufferICL();
    std::string filename = getImageCaptureFileName();
    if(filename == "") return;
    
    try{
      FileWriter(filename).write(&fb);
    }catch(ICLException &ex){
      ERROR_LOG("error capturing frame buffer: " << ex.what());
    }
  }

  // }}}
  
  void ICLWidget2::rebufferImageInternal(){
    // {{{ open

    m_data->mutex.lock();
    if(m_data->image && m_data->image->hasImage()){
      if(m_data->channelSelBuf){
        m_data->mutex.unlock();
        setImage(m_data->channelSelBuf);
      }else{
        ImgBase *tmpImage = m_data->image->deepCopy();
        m_data->mutex.unlock();
        setImage(tmpImage);
        delete tmpImage;
      }
    }else{
      m_data->mutex.unlock();
    }
    update();
  }

  // }}}
  
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
      customPaintEvent(&pe);
      /****
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
    if(m_data->outputCap){
      m_data->outputCap->captureFrameBufferHook();
    }
    
  }

  // }}}

  void ICLWidget2::paintEvent(QPaintEvent *e){
    // {{{ open

    QGLWidget::paintEvent(e);    
  }

  // }}}

  
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

    if(m_data->outputCap){
      m_data->outputCap->captureImageHook();
    }
    if(m_data->infoTab && m_data->infoTab->isVisible()){
      updateInfoTab();
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
  }
  // }}}

  void ICLWidget2::mouseReleaseEvent(QMouseEvent *e){
    // {{{ open

    switch(e->button()){
      case Qt::LeftButton: m_data->downMask[0]=false; break;
      case Qt::RightButton: m_data->downMask[2]=false; break;
      default: m_data->downMask[1] = false; break;
    }

  }
  // }}}
  void ICLWidget2::mouseMoveEvent(QMouseEvent *e){
    // {{{ open
  }
  // }}}
  void ICLWidget2::enterEvent(QEvent *e){
    // {{{ open
    if(m_data->menuEnabled){
      m_data->showMenuButton->show();
      m_data->embedMenuButton->show();
    }
  }
  // }}}

  void ICLWidget2::setVisible(bool visible){
    // {{{ open
    QGLWidget::setVisible(visible);
    m_data->showMenuButton->setVisible(false);
    m_data->embedMenuButton->setVisible(false);
    if(m_data->menuptr){
      m_data->menuptr->setVisible(false);
      m_data->adaptMenuSize(size());
    }
  }

  // }}}
  void ICLWidget2::leaveEvent(QEvent*){
    // {{{ open
    if(m_data->menuEnabled){
      m_data->showMenuButton->hide();
      m_data->embedMenuButton->hide();
    }
  }

  // }}}
  void ICLWidget2::resizeEvent(QResizeEvent *e){
    // {{{ open
    resizeGL(e->size().width(),e->size().height());
    m_data->adaptMenuSize(size());

  }
  // }}}

  void ICLWidget2::updateFromOtherThread(){
    // {{{ open

    QApplication::postEvent(this,new QEvent(QEvent::User),Qt::HighEventPriority);
  }

  // }}}

  void ICLWidget2::setMenuEnabled(bool enabled){
    // {{{ open

    m_data->menuEnabled = enabled;
  }

  // }}}
 
  std::vector<std::string> ICLWidget2::getImageInfo(){
    // {{{ open
    std::vector<string> info;

    GLTextureMapBaseImage* i = m_data->image;
    if(!i || !i->hasImage()){
      info.push_back("Image is NULL");
      return info;
    }
    info.push_back(string("depth:   ")+translateDepth(i->getDepth()));
    info.push_back(string("size:    ")+translateSize(i->getSize()));
    info.push_back(string("channels:")+str(i->getChannels()));
    info.push_back(string("format:  ")+translateFormat(i->getFormat()));
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
  // }}}



}
