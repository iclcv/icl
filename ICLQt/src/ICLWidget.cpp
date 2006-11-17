#include "ICLWidget.h"
#include <QImage>
#include <QPainter>
#include <QResizeEvent>

#include "OSD.h"
#include "ICLcc.h"
#include "Img.h"
#include "Timer.h"

#ifdef USE_OPENGL_ACCELERATION
#include "GLPaintEngine.h"
#else
#include "QtPaintEngine.h"
#endif

using std::string;
namespace icl{  
  const int ICLWidget::SHOW_OSD_ID;
  
  // {{{ Constructor

  ICLWidget::ICLWidget(QWidget *poParent):
    // {{{ open
    ParentWidgetClass(poParent),m_poImage(imgNew()),  m_poOSD(0),
    m_poCurrOSD(0),m_poShowOSD(0),m_iMouseX(-1), m_iMouseY(-1){
   
    setParent(poParent);
    setMouseTracking(true);
    // TODO what is optimal for the non-GL class
#ifdef USE_OPENGL_ACCELERATION
    setAttribute(Qt::WA_PaintOnScreen); 
    setAttribute(Qt::WA_NoBackground);
#endif

    op.fm = fmHoldAR;
    op.rm = rmOn;
    op.on = true;
    op.c = -1;
    op.brightness=0;
    op.contrast=0;
    op.intensity=0;
    
    memset(aiDown,0,3*sizeof(int));

  }
  
  // }}}
  
  //Destructor
  ICLWidget::~ICLWidget(void)
  {
    delete m_poImage;
    if (m_poOSD) { delete m_poOSD; m_poOSD = 0; }
    if (m_poShowOSD)  { delete m_poShowOSD; m_poShowOSD = 0; }
  }

  // }}}

  // {{{ mouse<Press|Release|Move|Enter|Release>()- and resizeEvent

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
    up();

    
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
    up();

    
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
    up();
  }

  // }}}
  
  void ICLWidget::enterEvent(QEvent *e){
    // {{{ open

    (void)e;
    m_oOSDMutex.lock();
    m_iMouseX = -1;
    m_iMouseY = -1;
    if(!m_poOSD){
      m_poOSD = new OSD(0,Rect(3,3,w()-10,h()-10),this,0);
      m_poShowOSD = new OSDButton(SHOW_OSD_ID,Rect(width()-80,height()-23,75,18),this,0,"menu");
    }
    if(m_poCurrOSD == 0){
      m_poCurrOSD = m_poShowOSD;
    }
    m_oOSDMutex.unlock();
    up();

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
   
    up();
    emit mouseEvent(updateMouseInfo(MouseInteractionInfo::leaveEvent));
  }

  // }}}
  
  void ICLWidget::resizeEvent(QResizeEvent *e){
    // {{{ open

    QSize s = e->size();

    m_oOSDMutex.lock();
    if(m_poOSD && isVisible()){
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

  // }}}
  
  // {{{ childChanged() and setImage()

  void ICLWidget::childChanged(int id, void *val){  
    // {{{ open

    (void)val;
    switch(id){
      case SHOW_OSD_ID:
        m_poCurrOSD = m_poOSD;
#ifndef USE_OPENGL_ACCELERATION   
        //      setAttribute(Qt::WA_PaintOnScreen,false); 
        // setAttribute(Qt::WA_NoBackground,false);
#endif
        break;
      case OSD::BOTTOM_EXIT_ID:
        m_poCurrOSD = m_poShowOSD;
#ifndef USE_OPENGL_ACCELERATION   
        //   setAttribute(Qt::WA_PaintOnScreen,true); 
        // setAttribute(Qt::WA_NoBackground,true);
#endif
        break;
      case OSD::FITMODE_NOSCALE_ID:
        op.fm = fmNoScale;
        break;
      case OSD::FITMODE_HOLDAR_ID:
        op.fm = fmHoldAR;
        break;
      case OSD::FITMODE_FIT_ID:
        op.fm = fmFit;
        break;
      case OSD::ADJUST_BRIGHTNESS_SLIDER_ID:
        op.brightness = *(int*)val;
        break;
      case OSD::ADJUST_CONTRAST_SLIDER_ID:
        op.contrast = *(int*)val;
        break;
      case OSD::ADJUST_INTENSITY_SLIDER_ID:
        op.intensity = *(int*)val;
        break;
      case OSD::ADJUST_MODE_NONE_ID:
        op.rm = rmOff;
        break;
      case OSD::ADJUST_MODE_MANUAL_ID:
        op.rm = rmOn;
        break;
      case OSD::ADJUST_MODE_AUTO_ID:
        op.rm = rmAuto;
        break;
      case OSD::CHANNELS_SLIDER_ID:
        op.c = *(int*)val;
        break;
      default:
        break;
    }

    up();
  }

  // }}}
  
  void ICLWidget::setImage(ImgBase *input){
    // {{{ open

    if(!op.on || !input){
      return;
    }
    
    m_oMutex.lock();    
    if(op.c<0){
      ensureCompatible(&m_poImage,input);
      input->deepCopy(m_poImage);
    }else if(op.c < input->getChannels()){ // copy the specific image channel
      ensureCompatible(&m_poImage,input->getDepth(),input->getSize(),1,formatMatrix);
      switch(input->getDepth()){
        case depth8u: deepCopyChannel(input->asImg<icl8u>(),op.c,m_poImage->asImg<icl8u>(),0); break;
        case depth32f: deepCopyChannel(input->asImg<icl32f>(),op.c,m_poImage->asImg<icl32f>(),0); break;
      }
    }else{
      if(m_poImage){
        delete m_poImage;
        m_poImage = 0;
      }
    }
    m_oMutex.unlock();    
  }

  // }}}

  // }}}
  
  // {{{ drawOSD(), drawImage() and paintGL()

  void ICLWidget::drawOSD(PaintEngine *e){
    // {{{ open

    m_oOSDMutex.lock();
    if(m_poCurrOSD){
      float m = std::min(((float)std::min(w(),h()))/100,6.0f);
      e->font("Arial",(int)(2*m)+5,PaintEngine::DemiBold);
      m_poCurrOSD->_drawSelf(e,m_iMouseX,m_iMouseY,aiDown);
    }
    m_oOSDMutex.unlock();
  }

  // }}}
  
#ifdef USE_OPENGL_ACCELERATION
  void ICLWidget::paintGL(){
    // {{{ open

    // {{{
    GLPaintEngine e(this);

    drawImage(&e);
    customPaintEvent(&e);
    drawOSD(&e);
  }

  // }}}
#else
  void ICLWidget::paintEvent(QPaintEvent *evt){
    // {{{ open
    (void)evt;

    QtPaintEngine e(this);
    
    drawImage(&e);
    customPaintEvent(&e);
    drawOSD(&e);
  }

  // }}}
#endif
  void ICLWidget::drawImage(PaintEngine *e){
    // {{{ open
    
    if(!m_poImage || !m_poImage->getDim()) return;
    int _w = w();
    int _h = h();
    Rect r(0,0,_w,_h);

    m_oMutex.lock();
    if(!op.on || !m_poImage){
      e->fill(0,0,0,255);
      e->rect(r);
      e->color(255,255,255);
      e->text(r,m_poImage ? "disabled" : "image is null", PaintEngine::Centered);
      m_oMutex.unlock();
      return;
    }
    switch(op.rm){
      case rmOff:
        e->bci();
        break;
      case rmOn:
        e->bci(op.brightness,op.contrast,op.intensity);
        break;
      case rmAuto:
        e->bciAuto();
        break;
    }
 
    e->image( computeImageRect(m_poImage->getSize(),Size(w(),h()),op.fm) , m_poImage, PaintEngine::Justify);
    m_oMutex.unlock();
  }

  // }}}

  // }}}

  // {{{ getImage<Info|Rect|Size>() up(), w() and h()

  std::vector<std::string> ICLWidget::getImageInfo(){
    // {{{ open

    std::vector<string> info;
    ImgBase* i = m_poImage;
    if(!i){
      info.push_back("Image is NULL");
      return info;
    }
    info.push_back(string("depth:   ")+translateDepth(i->getDepth()).c_str());
    info.push_back(string("size:    ")+QString::number(i->getSize().width).toLatin1().data()+" x "+
                   QString::number(i->getSize().height).toLatin1().data());
    info.push_back(string("channels:")+QString::number(i->getChannels()).toLatin1().data());
    info.push_back(string("format:  ")+translateFormat(i->getFormat()).c_str());
    if(i->hasFullROI()){
      info.push_back("roi:   full");
    }else{
      char ac[200];
      Rect r = i->getROI();
      sprintf(ac,"roi:   ((%d,%d),(%d x %d))",r.x,r.y,r.width,r.height);
      info.push_back(ac);
    }
    if(i->getDepth()==depth8u){
      for(int a=0;a<i->getChannels();a++){
        char ac[200];
        sprintf(ac,"channel %d, min:%d, max:%d",a,i->asImg<icl8u>()->getMin(a),i->asImg<icl8u>()->getMax(a));
        info.push_back(ac);
      }
    }else{
      for(int a=0;a<i->getChannels();a++){
        char ac[200];
        sprintf(ac,"channel %d, min:%f, max:%f",a,i->asImg<icl32f>()->getMin(a),i->asImg<icl32f>()->getMax(a));
        info.push_back(ac);
      }
    }
    return info;
  }

  // }}}

  void ICLWidget::up(){  update(); }
  int ICLWidget::w(){ return width(); }
  int ICLWidget::h(){ return height(); }

  Size ICLWidget::getImageSize(){
    // {{{ open

    m_oMutex.lock();
    Size s;
    if(m_poImage){
      s = m_poImage->getSize(); 
    }else{
      s = Size(w(),h());
    }
    m_oMutex.unlock();
    return s;
  }

  // }}}

  Rect ICLWidget::getImageRect(){
    // {{{ open

    return computeImageRect(getImageSize(),Size(w(),h()),op.fm);
  }

  // }}}

  // }}}

  // {{{ private utility functions computeImageRect() and updateMouseInfo()

  Rect ICLWidget::computeImageRect(Size oImageSize, Size oWidgetSize, fitmode eFitMode){
    // {{{ open

    int iImageW = oImageSize.width;
    int iImageH = oImageSize.height;
    int iW = oWidgetSize.width;
    int iH = oWidgetSize.height;
  
    switch(eFitMode){
      case fmNoScale:
        // set up the image rect to be centered
        return Rect((iW -iImageW)/2,(iH -iImageH)/2,iImageW,iImageH); 
        break;
      case fmHoldAR:{
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
      case fmFit:
        // the image is force to fit into the widget
        return Rect(0,0,iW,iH);
        break;
    }
    return Rect(0,0,iW,iH);
  }

  // }}}

  MouseInteractionInfo *ICLWidget::updateMouseInfo(MouseInteractionInfo::Type type){
    // {{{ open
    m_oMouseInfo.type = type;
    m_oMouseInfo.widgetX = m_iMouseX;
    m_oMouseInfo.widgetY = m_iMouseY;
    memcpy(m_oMouseInfo.downmask,aiDown,3*sizeof(int));
    
    m_oMutex.lock();
    Rect r = computeImageRect(m_poImage->getSize(), Size(w(),h()), op.fm);
    if(m_poImage && op.on && r.contains(m_iMouseX, m_iMouseY)){
      float boxX = m_iMouseX - r.x;
      float boxY = m_iMouseY - r.y;
      
      m_oMouseInfo.imageX = (int)round((boxX*(m_poImage->getSize().width))/r.width);
      m_oMouseInfo.imageY = (int)round((boxY*(m_poImage->getSize().height))/r.height);
      m_oMouseInfo.color.resize(0);
      if(m_poImage->getDepth() == depth8u){
        for(int c=0;c<m_poImage->getChannels();c++){
          m_oMouseInfo.color.push_back((*(m_poImage->asImg<icl8u>()))(m_oMouseInfo.imageX,m_oMouseInfo.imageY,c));
        }        
      }else{
        for(int c=0;c<m_poImage->getChannels();c++){
          m_oMouseInfo.color.push_back((*(m_poImage->asImg<icl32f>()))(m_oMouseInfo.imageX,m_oMouseInfo.imageY,c));
        }
      }
    }else{
      m_oMouseInfo.imageX = -1;
      m_oMouseInfo.imageY = -1;
      m_oMouseInfo.color.resize(0);
    }
    m_oMutex.unlock();
    return &m_oMouseInfo;
  }

  // }}}

  // }}}

}
