#include <iclWidget.h>
#include <iclImg.h>
#include <iclGLTextureMapBaseImage.h>
#include <iclGLPaintEngine.h>
#include <iclFileWriter.h>
#include <iclOSD.h>
#include <string>
#include <vector>

using namespace std;
namespace icl{
  
  namespace{
    Rect computeRect(const Size &imageSize, const Size &widgetSize, ICLWidget::fitmode mode){
      // {{{ open

      int iImageW = imageSize.width;
      int iImageH = imageSize.height;
      int iW = widgetSize.width;
      int iH = widgetSize.height;
      
      switch(mode){
        case ICLWidget::fmNoScale:
          // set up the image rect to be centered
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
      }
      return Rect(0,0,iW,iH);
    }
    // }}}
  }
  
  ICLWidget::ICLWidget(QWidget *parent) : 
    QGLWidget(parent),m_poOSD(0),
    m_poCurrOSD(0),m_poShowOSD(0),m_iMouseX(-1), m_iMouseY(-1),
    m_iCurrSelectedChannel(-1){
    // {{{ open

    makeCurrent();
    setAttribute(Qt::WA_PaintOnScreen); 
    setAttribute(Qt::WA_NoBackground);
    m_poImage = new GLTextureMapBaseImage(0,false);
    m_eFitMode = fmHoldAR;
    m_eRangeMode = rmOff;

    memset(m_aiBCI,0,3*sizeof(int));

    setMouseTracking(true);
    memset(aiDown,0,3*sizeof(int));
  }

  // }}}
  ICLWidget::~ICLWidget(){
    // {{{ open
    if(m_poImage)delete m_poImage;
    if (m_poOSD) { delete m_poOSD; m_poOSD = 0; }
    if (m_poShowOSD)  { delete m_poShowOSD; m_poShowOSD = 0; }
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

    m_oMutex.lock();
    glViewport(0, 0, (GLint)w, (GLint)h);
    m_oMutex.unlock();
  }

  // }}}
  void ICLWidget::paintGL(){
    // {{{ open

    m_oMutex.lock();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   
    if(m_poImage && m_poImage->hasImage()){
      Rect r = computeRect(m_poImage->getSize(),getSize(),m_eFitMode);
      m_poImage->drawTo(r,getSize());
    }else{
      GLPaintEngine pe(this);
      pe.fill(0,0,0,255);
      Rect fullRect(0,0,width(),height());
      pe.rect(fullRect);
      pe.color(255,255,255,255);
      pe.fill(255,255,255,255);
      pe.text(fullRect,"no image");
    }

    m_oMutex.unlock();

    GLPaintEngine pe(this);


    m_oOSDMutex.lock();
    if(m_poCurrOSD){
      float m = std::min(((float)std::min(width(),height()))/100,6.0f);
      pe.font("Arial",(int)(1.5*m)+5,PaintEngine::DemiBold);
      m_poCurrOSD->_drawSelf(&pe,m_iMouseX,m_iMouseY,aiDown);
    }
    m_oOSDMutex.unlock();

    customPaintEvent(&pe);
  }

  // }}}icl
  
  void ICLWidget::setImage(const ImgBase *image){ 
    // {{{ open
    ICLASSERT_RETURN(image);
    m_oMutex.lock();

    if(m_eRangeMode == rmAuto){
      m_poImage->bci(-1,-1,-1);
    }else if(m_eRangeMode == rmOn){
      m_poImage->bci(m_aiBCI[0],m_aiBCI[1],m_aiBCI[2]);
    }
    if(m_iCurrSelectedChannel != -1 && m_iCurrSelectedChannel >= 0 && m_iCurrSelectedChannel < image->getChannels()){
      const ImgBase *selectedChannel = image->selectChannel(m_iCurrSelectedChannel);
      m_poImage->updateTextures(selectedChannel);
      delete selectedChannel;
    }else{
      m_poImage->updateTextures(image);
    }
    m_oMutex.unlock();
  }

  // }}}
  void ICLWidget::setFitMode(fitmode fm){
    // {{{ open

    m_eFitMode = fm;
  }

  // }}}
  void ICLWidget::setRangeMode(rangemode rm){
    // {{{ open

    m_eRangeMode = rm;
  }

  // }}}
  void ICLWidget::setBCI(int brightness, int contrast, int intensity){
    // {{{ open

    m_aiBCI[0] = brightness;    
    m_aiBCI[1] = contrast;  
    m_aiBCI[2] = intensity;
  }

  // }}}
  void ICLWidget::customPaintEvent(PaintEngine *e){
    // {{{ open
    (void)e;
  }

  // }}}

  void ICLWidget::mousePressEvent(QMouseEvent *e){
    // {{{ open
    switch(e->button()){
      case Qt::LeftButton: aiDown[0]=1; break;
      case Qt::RightButton: aiDown[2]=1; break;
      default: aiDown[1] = 1; break;
    }
    
    m_oOSDMutex.lock();
    if(m_poCurrOSD && m_poCurrOSD->mouseOver(e->x(),e->y())){
      m_iMouseX = e->x();
      m_iMouseY = e->y();
      m_poCurrOSD->_mousePressed(e->x(),e->y(),e->button());
    }else{
      /// emitting signal
      emit mouseEvent(updateMouseInfo(MouseInteractionInfo::pressEvent));
    }
    m_oOSDMutex.unlock();
    update();
  }
  // }}}
  void ICLWidget::mouseReleaseEvent(QMouseEvent *e){
    // {{{ open
    
    switch(e->button()){
     case Qt::LeftButton: aiDown[0]=0; break;
     case Qt::RightButton: aiDown[2]=0; break;
     default: aiDown[1] = 0; break;
   }
   m_oOSDMutex.lock();
   m_iMouseX = e->x();
   m_iMouseY = e->y();
   if(m_poCurrOSD){
     m_poCurrOSD->_mouseReleased(e->x(),e->y(),e->button());
   }
   if(!m_poCurrOSD->mouseOver(e->x(),e->y())){
     /// emitting signal
     emit mouseEvent(updateMouseInfo(MouseInteractionInfo::releaseEvent));
   }
   m_oOSDMutex.unlock();
   update();
 }
  // }}}
  void ICLWidget::mouseMoveEvent(QMouseEvent *e){
    // {{{ open
    m_oOSDMutex.lock();
    m_iMouseX = e->x();
    m_iMouseY = e->y();
    if(m_poCurrOSD && m_poCurrOSD->mouseOver(e->x(),e->y())){
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
    update();
  }
  // }}}
  void ICLWidget::enterEvent(QEvent *e){
    // {{{ open
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
  }

  // }}}
  void ICLWidget::leaveEvent(QEvent *e){
    // {{{ open
    (void)e;
    m_oOSDMutex.lock();
    if(m_poCurrOSD == m_poShowOSD){
      m_poCurrOSD = 0;
    }
    m_oOSDMutex.unlock();
    m_iMouseX = -1;
    m_iMouseY = -1;
   
    update();
    emit mouseEvent(updateMouseInfo(MouseInteractionInfo::leaveEvent));
  }

  // }}}
  void ICLWidget::resizeEvent(QResizeEvent *e){
    // {{{ open
    resizeGL(e->size().width(),e->size().height());

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
  }
  // }}}
  void ICLWidget::childChanged(int id, void *val){  
    // {{{ open
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
        break;
      case OSD::ADJUST_CONTRAST_SLIDER_ID:
        m_aiBCI[1] = *(int*)val;
        break;
      case OSD::ADJUST_INTENSITY_SLIDER_ID:
        m_aiBCI[2] = *(int*)val;
        break;
      case OSD::ADJUST_MODE_NONE_ID:
        m_eRangeMode = rmOff;
        break;
      case OSD::ADJUST_MODE_MANUAL_ID:
        m_eRangeMode = rmOn;
        break;
      case OSD::ADJUST_MODE_AUTO_ID:
        m_eRangeMode = rmAuto;
        break;
      case OSD::CHANNELS_SLIDER_ID:
        m_iCurrSelectedChannel = *(int*)val;
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
          FileWriter("./image-snapshot.ppm").write(buf);
          delete buf;
        }
        break;
      }
      default:
        break;
    }

    update();
  }

  // }}}
 
  std::vector<std::string> ICLWidget::getImageInfo(){
    // {{{ open
    std::vector<string> info;
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
    
    for(int a=0;a<i->getChannels();a++){
      char ac[200];
      Range<icl32f> r = i->getMinMax(a);
      sprintf(ac,"channel %d, min:%f, max:%f",a,r.minVal,r.maxVal);
      info.push_back(ac);
    }
    return info;
  }

  // }}}
  Size ICLWidget::getImageSize(){
    // {{{ open
    m_oMutex.lock();
    Size s;
    if(m_poImage){
      s = m_poImage->getSize(); 
    }else{
      s = Size(width(),height());
    }
    m_oMutex.unlock();
    return s;
  }

  // }}}
  Rect ICLWidget::getImageRect(){
    // {{{ open

    return computeRect(getImageSize(),Size(width(),height()),m_eFitMode);
  }

  // }}}
  MouseInteractionInfo *ICLWidget::updateMouseInfo(MouseInteractionInfo::Type type){
    // {{{ open
    if(!m_poImage || !m_poImage->hasImage() ) return &m_oMouseInfo;
    m_oMouseInfo.type = type;
    m_oMouseInfo.widgetX = m_iMouseX;
    m_oMouseInfo.widgetY = m_iMouseY;
    memcpy(m_oMouseInfo.downmask,aiDown,3*sizeof(int));
    
    m_oMutex.lock();
    Rect r = computeRect(m_poImage->getSize(), Size(width(),height()), m_eFitMode);
    //if(m_poImage && op.on && r.contains(m_iMouseX, m_iMouseY)){
    if(r.contains(m_iMouseX, m_iMouseY)){
      float boxX = m_iMouseX - r.x;
      float boxY = m_iMouseY - r.y;
      m_oMouseInfo.imageX = (int) rint((boxX*(m_poImage->getSize().width))/r.width);
      m_oMouseInfo.imageY = (int) rint((boxY*(m_poImage->getSize().height))/r.height);
      m_oMouseInfo.color = m_poImage->getColor(m_oMouseInfo.imageX,m_oMouseInfo.imageY);
    }else{
      m_oMouseInfo.imageX = -1;
      m_oMouseInfo.imageY = -1;
      m_oMouseInfo.color.resize(0);
    }
    m_oMutex.unlock();
    return &m_oMouseInfo;
  }

  // }}}

}
