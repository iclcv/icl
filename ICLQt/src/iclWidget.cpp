// {{{ includes

#include <iclWidget.h>
#include <iclImg.h>
#include <iclGLTextureMapBaseImage.h>
#include <iclGLPaintEngine.h>
#include <iclFileWriter.h>
#include <string>
#include <vector>
#include <iclTime.h>
#include <iclQImageConverter.h>
#include <QImage>

#include <iclIconFactory.h>

#include <iclFileWriter.h>
#include <iclThread.h>

#include <QPainter>
#include <QTimer>
#include <QLabel>
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
#include <iclSplitterHandle.h>
#include <iclLabelHandle.h>
#include <iclButtonHandle.h>
#include <iclComboHandle.h>
#include <iclStringHandle.h>
#include <iclSpinnerHandle.h>
#include <iclSliderHandle.h>
#include <iclStringUtils.h>
#include <iclThreadedUpdatableWidget.h>

#include <iclRect32f.h>
#include <iclRange.h>
#include <iclTypes.h>

// }}}

using namespace std;
namespace icl{

#define LOCK_SECTION QMutexLocker SECTION_LOCKER(&m_data->mutex)

  class ZoomAdjustmentWidgetParent;
  
  struct HistogrammWidget : public ThreadedUpdatableWidget{
    // {{{ open

    static inline int median_of_3(int a, int b, int c){
      if( a > b){
        if(a > c) return c;
        else return a;
      }else{
        if(b > c) return c;
        else return b;
      }
    }
    static inline int median_of_5(int *p){
      int a[5]= {p[0],p[1],p[2],p[3],p[4]};
      std::sort(a,a+5);
      return a[2];
    }

    static inline int mean_of_3(int a, int b, int c){
      return (a+b+b+c)/4;
    }
    
    static inline int mean_of_5(int *p){
      return (p[0]+3*p[1]+5*p[2]+3*p[3]+p[4])/13;
    }
    struct Entry{
      float color[3];
      std::vector<int> histo;
    };
    std::vector<Entry> entries;

    bool logOn,meanOn,medianOn,fillOn,accuMode;
    int selChannel;
    Mutex mutex;

    /*
        virtual QSize sizeHint(){
        return QSize(1000,1000);
        }
    */
    HistogrammWidget(QWidget *parent):
      ThreadedUpdatableWidget(parent),logOn(false),meanOn(false),medianOn(false),
      fillOn(false),selChannel(-1),accuMode(false){
      
      /*
          setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
          if(parent){
          parent->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
          }
      */
    }
    
    void setFeatures(bool logOn, bool meanOn, bool medianOn, bool fillOn, int selChannel, bool accuMode=false){
      this->logOn = logOn;
      this->meanOn = meanOn;
      this->medianOn = medianOn;
      this->fillOn = fillOn;
      this->selChannel = selChannel;
      this->accuMode = accuMode;
    }
    void fillColor(int i,float color[3]){
      switch(i){
        case 0: color[0]=255;color[1]=0;color[2]=0; break;
        case 1: color[0]=0;color[1]=255;color[2]=0; break;
        case 2: color[0]=0;color[1]=0;color[2]=255; break;
        default: color[0]=255;color[1]=255;color[2]=255; break;
      }
    }
    virtual void update(){
      QWidget::update();
    }
    void update(const ImageStatistics &s){
      Mutex::Locker l(mutex);
      if(s.isNull){
        entries.clear();
        return;
      }
      
      // xxx todo process somewhere else 
      /** ImgParams params;
          depth d;
          std::vector<Range64f> ranges;
      ***/
      entries.resize(s.histos.size());
      for(unsigned int i=0;i<s.histos.size();++i){
        entries[i].histo = s.histos[i];
        fillColor(i,entries[i].color);
      }
      updateFromOtherThread();
    }
    
    virtual void paintEvent(QPaintEvent *e){
      Mutex::Locker l(mutex);
      QWidget::paintEvent(e);
      QPainter p(this);

      p.setRenderHint(QPainter::Antialiasing);
      p.setBrush(Qt::NoBrush);
      p.setPen(QColor(50,50,50));
      p.drawRect(QRectF(0,0,width(),height()));


      static const int BORDER = 2;
      static float GAP = 0;
      Rect32f r = Rect32f(0,0,width(),height()).enlarged(-BORDER);
      
      std::vector<QPolygonF> polys(entries.size());
      if(fillOn){
        for(unsigned int e=0;e<entries.size();++e){
          polys[e] << QPointF(width(),height());
          polys[e] << QPointF(0,height());
        }
      }
      // TODO fix accuMode
      // TODO use overAll max elem to be able to compare bins
      float maxElem = 0;
      if(accuMode){
        for(unsigned int e=1;e<entries.size();++e){
          ICLASSERT_RETURN(entries[0].histo.size() == entries[e].histo.size());
        }
        for(unsigned int i=0;i<entries[0].histo.size();++i){
          int accu = 0;
          for(unsigned int e=1;e<entries.size();++e){
            accu += entries[e].histo[i];
          }
          if(accu > maxElem) maxElem = accu;
        }
      }

      std::vector<std::vector<int> > oldHistos(entries.size());
      for(unsigned int e=0;e<entries.size();++e){
        if(!(selChannel == -1 || selChannel == e)) continue;
        std::vector<int> histo = entries[e].histo;
        if(!histo.size()) continue;
        int n = (int)histo.size();
        
        if(medianOn){
          histo[1] =  median_of_3(entries[e].histo[0],entries[e].histo[1],entries[e].histo[2]);
          for(int i=2;i<n-2;++i){
            histo[i] = median_of_5(&entries[e].histo[i-2]);
          }
          histo[n-2] =  median_of_3(entries[e].histo[n-3],entries[e].histo[n-2],entries[e].histo[n-1]);
        }else if(meanOn){
          histo[1] =  mean_of_3(entries[e].histo[0],entries[e].histo[1],entries[e].histo[2]);
          for(int i=2;i<n-2;++i){
            histo[i] = mean_of_5(&entries[e].histo[i-2]);
          }      
          histo[n-2] =  mean_of_3(entries[e].histo[n-3],entries[e].histo[n-2],entries[e].histo[n-1]);
        }
      
        p.setPen(QColor(entries[e].color[0],entries[e].color[1],entries[e].color[2]));

        
        if(!accuMode){
          maxElem = *max_element(histo.begin(),histo.end());
        }
        if(logOn) maxElem = ::log(maxElem);
        
        if(maxElem){
          float binDistance = r.width/n;
          float binWidth = binDistance-GAP;
          
          float lastX = 0,lastY=0;
          for(int i=0;i<n;i++){
            float val = histo[i];
            float valUnder = 0;
            if(accuMode){
              for(int j=0;j<e;++j){
                valUnder += oldHistos[j][i];
              }
              val += valUnder;
            }
            
            if(logOn && val) val=(::log(val));
            
            float h = (r.height/maxElem)*val;
            if(accuMode){
              h -= (r.height/maxElem)*valUnder;
            }
            float y = r.y+r.height-h;
            float x = r.x+(int)(i*binDistance);
            if(fillOn){
              // old xxx p.drawRect(QRectF(x,y,binWidth,h));
              polys[e] << QPointF(lastX+binWidth/2,lastY);
              if(i==n-1){
                polys[e] << QPointF(x+binWidth/2,y);
              }
            }else if(i>0){
              p.drawLine(QPointF(x+binWidth/2,y),QPointF(lastX+binWidth/2,lastY));
            }
            lastX = x;
            lastY = y;
          }
        }
        if(fillOn){
          p.setPen(QColor(entries[e].color[0],entries[e].color[1],entries[e].color[2],255));
          p.setBrush(QColor(entries[e].color[0],entries[e].color[1],entries[e].color[2],100));
          p.drawPolygon(polys[e]);
        }
        oldHistos[e]=histo;
      }
    }
  };

  // }}}
  
  struct OSDGLButton{
    // {{{ open
    enum IconType {Tool,Zoom,Lock, Unlock,RedZoom,RedCam, NNInter, LINInter};
    enum Event { Move, Press, Release, Enter, Leave,Draw};
    Rect bounds;
    bool toggable;
    Img8u icon;    
    Img8u downIcon;
    
    bool over;
    bool down;
    bool toggled;
    bool visible;
    
    GLTextureMapBaseImage tmImage;

    typedef void (ICLWidget::*VoidCallback)();
    typedef void (ICLWidget::*BoolCallback)(bool);
    VoidCallback vcb;
    BoolCallback bcb;
    
    static inline const Img8u &get_icon(IconType icon){
      switch(icon){
        case Tool: return IconFactory::create_image("tool");
        case Zoom: return IconFactory::create_image("zoom");
        case RedZoom: return IconFactory::create_image("red-zoom");
        case Unlock: return IconFactory::create_image("detached");
        case Lock: return IconFactory::create_image("embedded");
        case RedCam: return IconFactory::create_image("red-camera");
        case NNInter: return IconFactory::create_image("inter-nn");
        case LINInter: return IconFactory::create_image("inter-lin");
      }
      static Img8u undef(Size(32,32),4);
      return undef;
    }
    OSDGLButton(int x, int y, int w, int h, IconType icon, VoidCallback vcb=0):
      bounds(x,y,w,h),toggable(false),over(false),down(false),
      toggled(false),visible(false),vcb(vcb),bcb(0){
      this->icon = get_icon(icon);
    }
    OSDGLButton(int x, int y, int w, int h, IconType icon, IconType downIcon, BoolCallback bcb=0, bool toggled = false):
      bounds(x,y,w,h),toggable(true),over(false),down(false),
      toggled(toggled),visible(false),vcb(0),bcb(bcb){
      this->icon = get_icon(icon);
      this->downIcon = get_icon(downIcon);
    }
    void drawGL(const Size &windowSize){
      if(!visible) return;
      tmImage.bci(over*20,down*20,0);
      const Img8u &im = toggled ? downIcon : icon;
      tmImage.updateTextures(&im);
      tmImage.drawTo(bounds,windowSize,interpolateLIN);
    }
    bool update_mouse_move(int x, int y, ICLWidget *parent){
      if(!visible) return false;
      return (over = bounds.contains(x,y)); 
    }

    bool update_mouse_press(int x, int y,ICLWidget *parent){
      if(!visible) return false;
      if(bounds.contains(x,y)){
        down = true;
        if(toggable){
          toggled = !toggled;
          if(bcb) (parent->*bcb)(toggled);
        }else{
          if(vcb) (parent->*vcb)();
        }
        return true;
      }else{
        return false;
      }
    }
    bool update_mouse_release(int x, int y, ICLWidget *parent){
      if(!visible) return false;
      down = false;
      return bounds.contains(x,y);
    }

    bool event(int x, int y, Event evt, ICLWidget *parent){

      switch(evt){
        case Move: return update_mouse_move(x,y,parent);
        case Press: return update_mouse_press(x,y,parent);
        case Release: return update_mouse_release(x,y,parent);
        case Enter:
        case Leave: 
          visible = evt==Enter;
          return bounds.contains(x,y);
        case Draw: 
          drawGL(Size(parent->width(),parent->height()));
          return false;
      }
    }
  };
  // }}}


  struct ImageInfoIndicator : public ThreadedUpdatableWidget{
    // {{{ open

   ImgParams p;
   icl::depth d;
   ImageInfoIndicator(QWidget *parent):ThreadedUpdatableWidget(parent){
     d = (icl::depth)(-2);
     setBackgroundRole(QPalette::Window);
     //#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
     //setAttribute(Qt::WA_TranslucentBackground);
     //#endif

   }

   void update(const ImgParams p, icl::depth d){
     this->p = p;
     this->d = d;
     updateFromOtherThread();
   }
   inline std::string dstr(){
     switch(d){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return #D;
       ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
       default: return "?";
     }
   }
   inline std::string rstr(){
     if(p.getROISize() == p.getSize()) return "full";
     return translateRect(p.getROI());
   }

   inline std::string fstr(){
     if(p.getFormat() == formatMatrix){
       return str("mat(")+str(p.getChannels())+")";
     }else{
       return translateFormat(p.getFormat());
     }
   }
   virtual void paintEvent(QPaintEvent *e){
     QWidget::paintEvent(e);
     QPainter pa(this);
     pa.setRenderHint(QPainter::Antialiasing);

     pa.setBrush(QColor(210,210,210));
     pa.setPen(QColor(50,50,50));

     pa.drawRect(QRectF(0,0,width(),height()));
     static const char D[] = "-";
     
     std::string info = dstr()+D+translateSize(p.getSize())+D+rstr()+D+fstr();
     pa.drawText(QRectF(0,0,width(),height()),Qt::AlignCenter,info.c_str());
   }
  };

  // }}}
  
  struct ICLWidget::Data{
    // {{{ open

    Data(ICLWidget *parent):
      parent(parent),channelSelBuf(0),image(new GLTextureMapBaseImage(0,false)),
      qimageConv(0),qimage(0),mutex(QMutex::Recursive),fm(fmHoldAR),fmSave(fmHoldAR),
      rm(rmOff),bciUpdateAuto(false),channelUpdateAuto(false),
      mouseX(-1),mouseY(-1),selChannel(-1),showNoImageWarnings(true),
      outputCap(0),menuOn(true),menuptr(0),zoomAdjuster(0),
      qic(0),menuEnabled(true),infoTab(0),
      imageInfoIndicatorEnabled(true),infoTabVisible(false),
      selectedTabIndex(0),embeddedZoomMode(false),
      embeddedZoomModeJustEnabled(false),embeddedZoomRect(0),
      useLinInterpolation(false)
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
      ICL_DELETE(embeddedZoomRect);
      // ICL_DELETE(outputCap); this must be done by the parent widget
    }
    ICLWidget *parent;
    ImgBase *channelSelBuf;
    GLTextureMapBaseImage *image;
    QImageConverter *qimageConv;
    QImage *qimage;
    QMutex mutex;
    fitmode fm;
    fitmode fmSave;
    rangemode rm;
    int bci[3];
    bool *bciUpdateAuto;
    bool *channelUpdateAuto;
    bool downMask[3];
    int mouseX,mouseY;
    int selChannel;
    MouseInteractionInfo mouseInfo;
    bool showNoImageWarnings;
    ICLWidget::OutputBufferCapturer *outputCap;
    bool menuOn;

    Mutex menuMutex;    
    GUI menu;
    QWidget *menuptr;
    Rect32f zoomRect;
    ZoomAdjustmentWidgetParent *zoomAdjuster;
    QImageConverter *qic;
    bool menuEnabled;
    QWidget *infoTab;
    HistogrammWidget *histoWidget;
    ImageInfoIndicator *imageInfoIndicator;
    bool imageInfoIndicatorEnabled;
    bool infoTabVisible; // xxx
    bool selectedTabIndex;
    bool embeddedZoomMode;
    bool embeddedZoomModeJustEnabled;
    Rect32f *embeddedZoomRect;
    std::vector<OSDGLButton*> glbuttons;
    bool useLinInterpolation;
    
    bool event(int x, int y, OSDGLButton::Event evt){
      bool any = false;
      for(unsigned int i=0;i<glbuttons.size();++i){
        if(i == glbuttons.size()-1 && (evt == OSDGLButton::Enter || evt == OSDGLButton::Leave)){
          break;
        }
        any |= glbuttons[i]->event(x,y,evt,parent);
      }
      return any;
    }

    void updateImageInfoIndicatorGeometry(const QSize &parentSize){
      imageInfoIndicator->setGeometry(QRect(parentSize.width()-252,parentSize.height()-18,250,18));
    }
    

    void adaptMenuSize(const QSize &parentSize){
      if(!menuptr) return;
      static const int MARGIN = 5;
      static const int TOP_MARGIN = 20;
      int w = parentSize.width()-2*MARGIN;
      int h = parentSize.height()-(MARGIN+TOP_MARGIN);
      
      w = iclMax(menuptr->minimumWidth(),w);
      h = iclMax(menuptr->minimumHeight(),h);
      updateImageInfoIndicatorGeometry(parentSize);
    }
    void pushScaleModeAndChangeToZoom(){
      fmSave = fm;
      fm = fmZoom;
    }
    void popScaleMode(){
      fm = fmSave;
    }
  };

  // }}}

  struct ICLWidget::OutputBufferCapturer{
    // {{{ open

    ICLWidget *parent;
    ICLWidget::Data *data;
    
    bool recording;
    bool paused;
    enum CaptureTarget { SET_IMAGES, FRAME_BUFFER };
    CaptureTarget target;
    Mutex mutex;
    FileWriter *fileWriter;
    std::string filePattern;
    int frameSkip;
    int frameIdx;
  public:
    OutputBufferCapturer(ICLWidget *parent, ICLWidget::Data *data):
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
      this->filePattern = filePattern;
      try{
        fileWriter = new FileWriter(filePattern);
        ensureDirExists(File(filePattern).getDir().c_str());
      }catch(ICLException &ex){
        ERROR_LOG("unable to create filewriter with this pattern: " << filePattern << "\nerror:"<< ex.what());
        return false;
      }
      recording = true;
      return true;
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
  
  static Rect computeRect(const Size &imageSize, const Size &widgetSize, ICLWidget::fitmode mode,const Rect32f &relZoomRect=Rect32f::null){
    // {{{ open

    int iImageW = imageSize.width;
    int iImageH = imageSize.height;
    int iW = widgetSize.width;
    int iH = widgetSize.height;
    
    switch(mode){
      case ICLWidget::fmNoScale:
        // set up the image rect to be centeed
        return Rect((iW -iImageW)/2,(iH -iImageH)/2,iImageW,iImageH); 
        break;
      case ICLWidget::fmHoldAR:{
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
      case ICLWidget::fmFit:
        // the image is force to fit into the widget
        return Rect(0,0,iW,iH);
        break;
      case ICLWidget::fmZoom:{
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
      Rect r = computeRect(imageSize,Size(width(),height()), ICLWidget::fmHoldAR);
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

  static void create_menu(ICLWidget *widget,ICLWidget::Data *data){
    // {{{ open

    Mutex::Locker locker(data->menuMutex);

    // OK, we need to extract default values for all gui elements if gui is already defined!
    data->menu = GUI("tab(bci,scale,channel,capture,info)[@handle=root@minsize=5x7]",widget);

    GUI bciGUI("vbox");

    std::string bcis[3]={"custom,","off,","auto"};
    bcis[((int)data->rm)-1] = str("!")+bcis[((int)data->rm)-1];
    bool bciAuto = data->bciUpdateAuto && *data->bciUpdateAuto;
    bciGUI << ( GUI("hbox")
                << str("combo(")+bcis[0]+bcis[1]+bcis[2]+")[@label=bci-mode@handle=bci-mode@out=bci-mode-out]"
                << str("togglebutton(manual,")+(bciAuto?"!":"")+"auto)[@label=update mode@out=bci-update-mode]"
              )
           << str("slider(-255,255,")+str(data->bci[0])+")[@handle=brightness@label=brightness@out=_2]"
           << str("slider(-255,255,")+str(data->bci[1])+")[@handle=contrast@label=contrast@out=_1]"
           << str("slider(-255,255,")+str(data->bci[2])+")[@handle=intensity@label=intensity@out=_0]";
    
    GUI scaleGUI("vbox");
    std::string em[4]={"no scale,","hold aspect ratio,","force fit,","zoom"};
    em[data->fm] = str("!")+em[data->fm];
    scaleGUI << str("combo(")+em[0]+em[1]+em[2]+em[3]+")[@maxsize=100x2@handle=fit-mode@label=scale mode@out=_3]";
    scaleGUI << "hbox[@handle=scale-widget@minsize=4x3]";

    GUI channelGUI("vbox");
    channelGUI << "combo(all,channel #0,channel #1,channel #2,channel #3)[@maxsize=100x2@handle=channel@label=visualized channel@out=_4]"
               << "togglebutton(manual,auto)[@maxsize=100x2@label=update mode@out=channel-update-mode]";

    GUI captureGUI("vbox");
     
    captureGUI << ( GUI("hbox")
                    << ( GUI("hbox[@label=single shot]") 
                         << "button(image)[@handle=cap-image]"
                         << "button(frame buffer)[@handle=cap-fb]"
                       )
                    << "combo(image.pnm,image_TIME.pnm,ask me)[@label=filename@handle=cap-filename@out=_5]"
                  );
                  
    
    bool autoCapFB = data->outputCap && data->outputCap->target==ICLWidget::OutputBufferCapturer::FRAME_BUFFER;
    int autoCapFS = data->outputCap ? (data->outputCap->frameSkip) : 0;
    std::string autoCapFP = data->outputCap ? data->outputCap->filePattern : str("captured/image_####.ppm");
    bool autoCapRec = data->outputCap && data->outputCap->recording;
    bool autoCapPau = data->outputCap && data->outputCap->paused;

    GUI autoCapGUI("vbox[@label=automatic]");
    autoCapGUI << ( GUI("hbox")
                    << str("combo(image,")+(autoCapFB?"!":"")+"frame buffer)[@label=mode@handle=auto-cap-mode@out=_6]"
                    << str("spinner(0,100,")+str(autoCapFS)+")[@label=frame skip@handle=auto-cap-frameskip@out=_10]"
                    << str("string(")+autoCapFP+",100)[@label=file pattern@handle=auto-cap-filepattern@out=_7]"
                  )
               << ( GUI("hbox")
                    << str("togglebutton(record,")+(autoCapRec?"!":"")+"record)[@handle=auto-cap-record@out=_8]"
                    << str("togglebutton(pause,")+(autoCapPau?"!":"")+"pause)[@handle=auto-cap-pause@out=_9]"
                    << "button(stop)[@handle=auto-cap-stop]"
                  );
    captureGUI << autoCapGUI;    
    
    GUI infoGUI("vsplit[@handle=info-tab]");
    infoGUI << ( GUI("hbox")
                 << "combo(all ,channel #0, channel #1,channel #2, channel #3)[@handle=histo-channel@out=_15]"
                 << "togglebutton(median,median)[@handle=median@out=median-on]"
                 << "togglebutton(log,log)[@handle=log@out=log-on]"
                 << "togglebutton(blur,blur)[@handle=blur@out=blur-on]"
                 << "togglebutton(fill,fill)[@handle=fill@out=fill-on]"
               )
            <<  ( GUI("hsplit")
                  << "hbox[@label=histogramm@handle=histo-box@minsize=12x10]"
                  << "label[@label=params@handle=image-info-label]"
                );

    data->menu << bciGUI << scaleGUI << channelGUI << captureGUI << infoGUI;
    
    data->menu.create();

    data->menuptr = data->menu.getRootWidget();
    data->menuptr->setParent(widget);
    // xxx new layout test
    if(!widget->layout()){
      widget->setLayout(new QGridLayout);
      widget->layout()->setContentsMargins(5,22,5,5);
    }
    widget->layout()->addWidget(data->menuptr);


    /// xxx new bg tset
    data->menuptr->setAutoFillBackground(true);
    //data->menuptr->setStyleSheet("QWidget { background : rgba(200,200,200,10) }");
    
    
    data->bciUpdateAuto = &data->menu.getValue<bool>("bci-update-mode");
    data->channelUpdateAuto = &data->menu.getValue<bool>("channel-update-mode");

    data->infoTab = *data->menu.getValue<SplitterHandle>("info-tab");
    
    data->histoWidget = new HistogrammWidget(*data->menu.getValue<BoxHandle>("histo-box"));
    data->menu.getValue<BoxHandle>("histo-box").add(data->histoWidget);
    
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
    
    QObject::connect(*data->menu.getValue<ButtonHandle>("fill"),SIGNAL(toggled(bool)),widget,SLOT(histoPanelParamChanged()));
    QObject::connect(*data->menu.getValue<ButtonHandle>("blur"),SIGNAL(toggled(bool)),widget,SLOT(histoPanelParamChanged()));
    QObject::connect(*data->menu.getValue<ButtonHandle>("median"),SIGNAL(toggled(bool)),widget,SLOT(histoPanelParamChanged()));
    QObject::connect(*data->menu.getValue<ButtonHandle>("log"),SIGNAL(toggled(bool)),widget,SLOT(histoPanelParamChanged()));
    QObject::connect(*data->menu.getValue<ComboHandle>("histo-channel"),SIGNAL(currentIndexChanged(int)),widget,SLOT(histoPanelParamChanged()));

  }

  // }}}
  
  void update_data(const Size &newImageSize, ICLWidget::Data *data){
    // {{{ open

    data->menuMutex.lock();
    if(data->menuptr){
      // TODO this cannot be done at runtime -> so the zoom adjusters state must be tracked in 
      // another way ...
      if(data->zoomAdjuster->isVisible()){
        data->zoomAdjuster->updateImageSize(newImageSize);
      }
    }
    data->menuMutex.unlock();
  }

  // }}}

  // ------------ ICLWidget ------------------------------

  ICLWidget::ICLWidget(QWidget *parent) : 
    // {{{ open

    m_data(new ICLWidget::Data(this)){
    
    setMouseTracking(true);
    setWindowIcon(IconFactory::create_icl_window_icon_as_qicon());
    
    int x = 2;
    m_data->glbuttons.push_back(new OSDGLButton(x,2,20,20,OSDGLButton::Tool,&ICLWidget::showHideMenu));
    x+=22;
    m_data->glbuttons.push_back(new OSDGLButton(x,2,20,20,OSDGLButton::Unlock,OSDGLButton::Lock,&ICLWidget::setMenuEmbedded,true));
    x+=22;
    m_data->glbuttons.push_back(new OSDGLButton(x,2,20,20,OSDGLButton::NNInter,OSDGLButton::LINInter,&ICLWidget::setLinInterpolationEnabled,false));
    x+=22;
    m_data->glbuttons.push_back(new OSDGLButton(x,2,20,20,OSDGLButton::Zoom,OSDGLButton::RedZoom,&ICLWidget::setEmbeddedZoomModeEnabled,false));
    x+=22;
    m_data->glbuttons.push_back(new OSDGLButton(x,2,20,20,OSDGLButton::RedCam,&ICLWidget::stopButtonClicked));
    x+=22;

    m_data->imageInfoIndicator = new ImageInfoIndicator(this);
  }

  // }}}
  
  ICLWidget::~ICLWidget(){
    // {{{ open

    ICL_DELETE(m_data->outputCap);// just because of the classes definition order 
    delete m_data; 
  }

  // }}}

  void ICLWidget::setEmbeddedZoomModeEnabled(bool enabled){
    // {{{ open
    m_data->embeddedZoomMode = enabled;
    if(!enabled){
      ICL_DELETE(m_data->embeddedZoomRect);
      m_data->zoomRect = Rect32f(0,0,getImageSize().width,getImageSize().height);
      m_data->popScaleMode();
      update();
    }else{
      m_data->embeddedZoomModeJustEnabled = true;
    }
    
  }
  // }}} 

  void ICLWidget::setLinInterpolationEnabled(bool enabled){
    // {{{ open
    m_data->useLinInterpolation = enabled;
  }
  // }}}

  void ICLWidget::bciModeChanged(int modeIdx){
    // {{{ open

    switch(modeIdx){
      case 0: m_data->rm = rmOn; break;
      case 1: m_data->rm = rmOff; break;
      case 2: m_data->rm = rmAuto; break;
      default: ERROR_LOG("invalid range mode index");
    }
    if(*m_data->bciUpdateAuto){
      rebufferImageInternal();
    }
  }

  // }}}

  void ICLWidget::brightnessChanged(int val){
    // {{{ open

    m_data->bci[0] = val;
    if(*m_data->bciUpdateAuto){
      rebufferImageInternal();
    }
  }

  // }}}

  void ICLWidget::contrastChanged(int val){
    // {{{ open

    m_data->bci[1] = val;
    if(*m_data->bciUpdateAuto){
      rebufferImageInternal();
    }
  }

  // }}}

  void ICLWidget::intensityChanged(int val){
    // {{{ open

    m_data->bci[2] = val;
    if(*m_data->bciUpdateAuto){
      rebufferImageInternal();
    }
  }

  // }}}
  
  void ICLWidget::scaleModeChanged(int modeIdx){
    // {{{ open

    //hold aspect ratio,force fit,no scale, zoom
    switch(modeIdx){
      case 0: m_data->fm = fmNoScale; break;
      case 1: m_data->fm = fmHoldAR; break;
      case 2: m_data->fm = fmFit; break;
      case 3: m_data->fm = fmZoom; break;
      default: ERROR_LOG("invalid scale mode index");
    }
    update();
  }

  // }}}

  void ICLWidget::currentChannelChanged(int modeIdx){
    // {{{ open

    m_data->selChannel = modeIdx - 1;
    if(*m_data->channelUpdateAuto){
      rebufferImageInternal();
    }
  }

  // }}}
  
  void ICLWidget::showHideMenu(){
    // {{{ open

    if(!m_data->menuptr){
      create_menu(this,m_data);
    }

    m_data->menuptr->setVisible(!m_data->menuptr->isVisible());
    if(m_data->menuptr->isVisible()){
      if(m_data->selectedTabIndex == 4){
        m_data->infoTabVisible = true;
      }else{
        m_data->infoTabVisible = false;
      }
    }else{
      m_data->infoTabVisible = false;
    }
    m_data->adaptMenuSize(size());
  }

  // }}}
  
  void ICLWidget::setMenuEmbedded(bool embedded){
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
      m_data->menuptr->setWindowTitle("menu...");
      m_data->menuptr->setGeometry(QRect(mapToGlobal(pos())+QPoint(2,2),QSize(width()-4,height()-2)));
    }
    m_data->menuptr->setVisible(visible);
  }

  // }}}

  void ICLWidget::recordButtonToggled(bool checked){
    // {{{ open

    if(!m_data->outputCap){
      m_data->outputCap = new OutputBufferCapturer(this,m_data);
    }
    if(checked){
      Mutex::Locker l(m_data->menuMutex);
      OutputBufferCapturer::CaptureTarget t = m_data->menu.getValue<ComboHandle>("auto-cap-mode").getSelectedIndex()?
                                              OutputBufferCapturer::FRAME_BUFFER :  OutputBufferCapturer::SET_IMAGES;

      const std::string filePattern = m_data->menu.getValue<StringHandle>("auto-cap-filepattern").getCurrentText();
      int frameSkip = m_data->menu.getValue<SpinnerHandle>("auto-cap-frameskip").getValue();
      bool ok = m_data->outputCap->startRecording(t,filePattern,frameSkip);
      if(!ok){
        (*m_data->menu.getValue<ButtonHandle>("auto-cap-record"))->setChecked(false);
      }else{
        m_data->glbuttons.back()->visible = true;
        update();
      }
    }else{
      m_data->outputCap->stopRecording();
      m_data->glbuttons.back()->visible = false;
      update();
    }
  }

  // }}}

  void ICLWidget::pauseButtonToggled(bool checked){
    // {{{ open

    if(!m_data->outputCap){
      m_data->outputCap = new OutputBufferCapturer(this,m_data);
    }
    m_data->outputCap->setPaused(checked);
  }

  // }}}

  void ICLWidget::stopButtonClicked(){
    // {{{ open
    if(m_data->outputCap){
      m_data->outputCap->stopRecording();
      Mutex::Locker l(m_data->menuMutex);
      (*m_data->menu.getValue<ButtonHandle>("auto-cap-record"))->setChecked(false);
    }
    m_data->glbuttons[3]->visible = false;
    update();
  }

  // }}}
  
  void ICLWidget::skipFramesChanged(int frameSkip){
    // {{{ open

    if(m_data->outputCap){
      m_data->outputCap->frameSkip = frameSkip;
    }
  }

  // }}}

  void ICLWidget::menuTabChanged(int index){
    // {{{ open
    m_data->selectedTabIndex = index;
    if(index == 4){
      m_data->infoTabVisible = true;
      updateInfoTab();
    }else{
      m_data->infoTabVisible = false;
    }
  }

  // }}}

  void ICLWidget::histoPanelParamChanged(){
    // {{{ open
    Mutex::Locker l(m_data->menuMutex);
    if(!m_data->histoWidget) return;
    m_data->histoWidget->setFeatures(m_data->menu.getValue<bool>("log-on"),
                                     m_data->menu.getValue<bool>("blur-on"),
                                     m_data->menu.getValue<bool>("median-on"),
                                     m_data->menu.getValue<bool>("fill-on"),
                                     m_data->menu.getValue<ComboHandle>("histo-channel").getSelectedIndex()-1);

    m_data->histoWidget->update();
    
  }
  // }}}
 
  void ICLWidget::updateInfoTab(){
    // {{{ open
    Mutex::Locker l(m_data->menuMutex);
    if(m_data->histoWidget && m_data->infoTabVisible){
      m_data->histoWidget->update(getImageStatistics());
      // xxx TODO
      std::vector<string> s = getImageInfo();
      m_data->menu.getValue<LabelHandle>("image-info-label") = cat(s,"\n");
    }

  }

  // }}}
  
  std::string ICLWidget::getImageCaptureFileName(){
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

  void ICLWidget::captureCurrentImage(){
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

  const Img8u &ICLWidget::grabFrameBufferICL(){
    // {{{ open

    if(!m_data->qic){
      m_data->qic = new QImageConverter;
    }
    QImage qim = grabFrameBuffer();
    m_data->qic->setQImage(&qim);
    return *m_data->qic->getImg<icl8u>();
  }

  // }}}
  
  void ICLWidget::captureCurrentFrameBuffer(){
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
  
  void ICLWidget::rebufferImageInternal(){
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
  
  void ICLWidget::initializeGL(){
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

  void ICLWidget::resizeGL(int w, int h){
    // {{{ open

    LOCK_SECTION;
    makeCurrent();
    glViewport(0, 0, (GLint)w, (GLint)h);
  }

  // }}}
 
  void ICLWidget::paintGL(){
    // {{{ open
    m_data->mutex.lock();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
    if(m_data->image && m_data->image->hasImage()){
      Rect r;
      if(m_data->fm == fmZoom){
        Mutex::Locker locker(m_data->menuMutex);
        r = computeRect(m_data->image->getSize(),getSize(),fmZoom,m_data->zoomRect);//zoomAdjuster->aw->r);
      }else{
        r = computeRect(m_data->image->getSize(),getSize(),m_data->fm);
      }
      m_data->image->drawTo(r,getSize(),m_data->useLinInterpolation?interpolateLIN:interpolateNN);
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
    m_data->mutex.unlock();

    Thread::msleep(0);
    
    GLPaintEngine pe(this);

    // m_data->mutex.lock();
    customPaintEvent(&pe);
    // m_data->mutex.unlock();


    m_data->event(0,0,OSDGLButton::Draw);


    Thread::msleep(0);
    
    if(m_data->embeddedZoomRect){
      pe.color(0,150,255,200);
      pe.fill(0,150,255,50);
      Rect32f &r = *m_data->embeddedZoomRect; 
      pe.rect(Rect((int)r.x,(int)r.y,(int)r.width,(int)r.height));
    }
    
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
    if(m_data->outputCap){
      m_data->outputCap->captureFrameBufferHook();
    }
  }

  // }}}

  void ICLWidget::paintEvent(QPaintEvent *e){
    // {{{ open

    QGLWidget::paintEvent(e);    
  }

  // }}}

  void ICLWidget::setImage(const ImgBase *image){ 
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
    m_data->imageInfoIndicator->update(image->getParams(),image->getDepth());
    updateInfoTab();
  }

  // }}}

  void ICLWidget::setFitMode(fitmode fm){
    // {{{ open
    m_data->fm = fm;
  }

  // }}}

  void ICLWidget::setRangeMode(rangemode rm){
    // {{{ open
    m_data->rm = rm;
  }

  // }}}

  void ICLWidget::setBCI(int brightness, int contrast, int intensity){
    // {{{ open
    m_data->bci[0] = brightness;
    m_data->bci[1] = contrast;
    m_data->bci[2] = intensity;
  }
  // }}}

  void ICLWidget::customPaintEvent(PaintEngine*){
    // {{{ open
  }
  // }}}

  void ICLWidget::mousePressEvent(QMouseEvent *e){
    // {{{ open
    
    if(m_data->embeddedZoomMode){
      m_data->embeddedZoomRect = new Rect32f(e->x(),e->y(),0.1,0.1);
      update();
      return;
    }

    if(m_data->event(e->x(),e->y(),OSDGLButton::Press)){
      update();
      return;
    }

    switch(e->button()){
      case Qt::LeftButton: m_data->downMask[0]=true; break;
      case Qt::RightButton: m_data->downMask[2]=true; break;
      default: m_data->downMask[1] = true; break;
    }
    emit mouseEvent(updateMouseInfo(MouseInteractionInfo::pressEvent));
    update();
  }
  // }}}

  void ICLWidget::mouseReleaseEvent(QMouseEvent *e){
    // {{{ open

    if(m_data->embeddedZoomMode){
      if(m_data->embeddedZoomModeJustEnabled){
        m_data->embeddedZoomModeJustEnabled = false;
      }else{
        if(m_data->embeddedZoomRect){
          Rect32f &r = *m_data->embeddedZoomRect;
          //const Size wSize(width(),height());
          Rect ir = getImageRect();
          
          Rect32f &nr = m_data->zoomRect;
          nr.x = (r.x-ir.x)/float(ir.width);
          nr.y = (r.y-ir.y)/float(ir.height);
          nr.width = r.width/ir.width;
          nr.height = r.height/ir.height;
          /*
              m_data->zoomRect.x = r.x/wSize.width;
              m_data->zoomRect.y = r.y/wSize.height;
              m_data->zoomRect.width = r.width/wSize.width;
              m_data->zoomRect.height = r.height/wSize.height;
          */
          m_data->pushScaleModeAndChangeToZoom();
        }
        ICL_DELETE(m_data->embeddedZoomRect);
        m_data->embeddedZoomMode = false;
        m_data->embeddedZoomModeJustEnabled = false;
        update();
        return;
      }
    }

    if(m_data->event(e->x(),e->y(),OSDGLButton::Release)){
      update();
      return;
    }

    switch(e->button()){
      case Qt::LeftButton: m_data->downMask[0]=false; break;
      case Qt::RightButton: m_data->downMask[2]=false; break;
      default: m_data->downMask[1] = false; break;
    }
    emit mouseEvent(updateMouseInfo(MouseInteractionInfo::releaseEvent));
    update();
  }
  // }}}

  void ICLWidget::mouseMoveEvent(QMouseEvent *e){
    // {{{ open
    
    if(m_data->embeddedZoomMode && m_data->embeddedZoomRect){
      m_data->embeddedZoomRect->width = e->x()-m_data->embeddedZoomRect->x;
      m_data->embeddedZoomRect->height = e->y()-m_data->embeddedZoomRect->y;
      *m_data->embeddedZoomRect = m_data->embeddedZoomRect->normalized();
      update();
      return;
    }


    if(m_data->event(e->x(),e->y(),OSDGLButton::Move)){
      update();
      return;
    }
    m_data->mouseX = e->x();
    m_data->mouseY = e->y();
    
    if(m_data->downMask[0] || m_data->downMask[1] || m_data->downMask[2]){
      emit mouseEvent(updateMouseInfo(MouseInteractionInfo::dragEvent));
    }else{
      emit mouseEvent(updateMouseInfo(MouseInteractionInfo::moveEvent));
    }
    update();
  }
  // }}}
  
  void ICLWidget::enterEvent(QEvent*){
    // {{{ open

    if(m_data->menuEnabled){
      if(m_data->event(-1,-1,OSDGLButton::Enter)){
        update();
        return;
      }
    }
    if(m_data->imageInfoIndicatorEnabled){
      m_data->imageInfoIndicator->show();
      m_data->updateImageInfoIndicatorGeometry(size());
    }
    emit mouseEvent(updateMouseInfo(MouseInteractionInfo::enterEvent));
    update();
  }
  // }}}

  void ICLWidget::leaveEvent(QEvent*){
    // {{{ open
    if(m_data->menuEnabled){
      if(m_data->event(-1,-1,OSDGLButton::Leave)){
        update();
        return;
      }
    }
    if(m_data->imageInfoIndicatorEnabled){
      m_data->imageInfoIndicator->hide();
    }
    emit mouseEvent(updateMouseInfo(MouseInteractionInfo::leaveEvent));
    update();
  }

  // }}}


  void ICLWidget::resizeEvent(QResizeEvent *e){
    // {{{ open
    resizeGL(e->size().width(),e->size().height());
    m_data->adaptMenuSize(size());

  }
  // }}}

  void ICLWidget::setVisible(bool visible){
    // {{{ open
    QGLWidget::setVisible(visible);
    // xxx m_data->showMenuButton->setVisible(false);
    // xxx m_data->embedMenuButton->setVisible(false);
    m_data->imageInfoIndicator->setVisible(false);
    if(m_data->menuptr){
      m_data->menuptr->setVisible(false);
      m_data->adaptMenuSize(size());
    }
  }

  // }}}


  ICLWidget::fitmode ICLWidget::getFitMode(){
    // {{{ open

    return m_data->fm;
  }

  // }}}
  
  ICLWidget::rangemode ICLWidget::getRangeMode(){
    // {{{ open

    return m_data->rm;
  }

  // }}}

  void ICLWidget::setShowNoImageWarnings(bool showWarnings){
    // {{{ open

    m_data->showNoImageWarnings = showWarnings;
  }

  // }}}

  void ICLWidget::updateFromOtherThread(){
    // {{{ open

    QApplication::postEvent(this,new QEvent(QEvent::User),Qt::HighEventPriority);
  }

  // }}}

  void ICLWidget::setMenuEnabled(bool enabled){
    // {{{ open

    m_data->menuEnabled = enabled;
  }

  // }}}

  void ICLWidget::setImageInfoIndicatorEnabled(bool enabled){
    // {{{ open
    m_data->imageInfoIndicatorEnabled = enabled;
  }
  // }}}
 
  std::vector<std::string> ICLWidget::getImageInfo(){
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
      info.push_back(translateRect(i->getROI()));
    }
    
    std::vector<Range<icl32f> > ranges = i->getMinMax();
    for(int a=0;a<i->getChannels();a++){
      info.push_back(str("channel "+str(a)+":"));
      info.push_back(str("   ")+translateRange(ranges[a]));
    }
    return info;
  }

  // }}}

  Size ICLWidget::getImageSize(){
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

  Rect ICLWidget::getImageRect(){
    // {{{ open
    Rect r;
    if(m_data->fm == fmZoom){
      Mutex::Locker locker(m_data->menuMutex);
      r = computeRect(m_data->image->getSize(),getSize(),fmZoom,m_data->zoomRect);
    }else{
      r = computeRect(m_data->image->getSize(),getSize(),m_data->fm);
    }
    return r;
  }

  // }}}

  MouseInteractionInfo *ICLWidget::updateMouseInfo(MouseInteractionInfo::Type type){
    // {{{ open
    MouseInteractionInfo &mii = m_data->mouseInfo;

    if(!m_data->image || !m_data->image->hasImage()){
      return &mii;
    }
    mii.type = type;
    mii.widgetX = m_data->mouseX;
    mii.widgetY = m_data->mouseY;
    
    std::copy(m_data->downMask,m_data->downMask+3,mii.downmask);
    
    LOCK_SECTION;
    Rect r = getImageRect();
        //if(m_poImage && op.on && r.contains(m_iMouseX, m_iMouseY)){
        //    if(r.contains(m_iMouseX, m_iMouseY)){
    float boxX = m_data->mouseX - r.x;
    float boxY = m_data->mouseY - r.y;
    mii.imageX = (int) rint((boxX*(m_data->image->getSize().width))/r.width);
    mii.imageY = (int) rint((boxY*(m_data->image->getSize().height))/r.height);
    if(r.contains(m_data->mouseX,m_data->mouseY)){
      mii.color = m_data->image->getColor(mii.imageX,mii.imageY);
    }else{
      mii.color.resize(0);
    }
    mii.relImageX = float(mii.imageX)/m_data->image->getSize().width;
    mii.relImageY = float(mii.imageY)/m_data->image->getSize().height;
    return &mii;
  }

  // }}}

  const ImageStatistics &ICLWidget::getImageStatistics() {
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
