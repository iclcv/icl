#include "ICLWidget.h"
#include <QImage>
#include <QPainter>
#include <QResizeEvent>

#include "OSD.h"
#include "ICLcc.h"
#include "Img.h"
#include "Timer.h"


namespace icl{  
  const int ICLWidget::SHOW_OSD_ID;
  
  ICLWidget::ICLWidget(QWidget *poParent):ParentWidgetClass(poParent),
    // {{{ open Constructor definition
    m_poImage(new Img8u(Size(1,1),4)),
    m_poOSD(0),m_poCurrOSD(0),m_poShowOSD(0),m_iMouseX(-1), m_iMouseY(-1){
   
    setParent(poParent);
    setMouseTracking(true);

    for(int i=0;i<256;i++){
      m_oColorTable.push_back(qRgb(i,i,i));
    }
    setAttribute(Qt::WA_PaintOnScreen); 
    setAttribute(Qt::WA_NoBackground);

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
    if(m_poCurrOSD){
      m_iMouseX = e->x();
      m_iMouseY = e->y();
      m_poCurrOSD->_mouseReleased(e->x(),e->y(),e->button());
    }
    m_oOSDMutex.unlock();
    up();
  }
  // }}}
  void ICLWidget::mouseMoveEvent(QMouseEvent *e){
    // {{{ open
    m_oOSDMutex.lock();
    if(m_poCurrOSD && m_poCurrOSD->mouseOver(e->x(),e->y())){
      m_iMouseX = e->x();
      m_iMouseY = e->y();
      m_poCurrOSD->_mouseMoved(e->x(),e->y(),aiDown);
    }
    m_oOSDMutex.unlock();

    up();
  }
  // }}}
  void ICLWidget::enterEvent(QEvent *e){
    // {{{ open
    (void)e;
    m_oOSDMutex.lock();
    if(!m_poOSD){
      m_poOSD = new OSD(0,QRect(3,3,w()-10,h()-10),this,0);
      m_poShowOSD = new OSDButton(SHOW_OSD_ID,QRect(w()-52,h()-16,50,14),this,0,"options");
    }
    if(m_poCurrOSD == 0){
      m_poCurrOSD = m_poShowOSD;
    }
    m_oOSDMutex.unlock();
    up();
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
      delete m_poOSD;
      delete m_poShowOSD;
      m_poOSD = new OSD(0,QRect(3,3,s.width()-10,s.height()-10),this,0);
      m_poShowOSD = new OSDButton(SHOW_OSD_ID,QRect(s.width()-52,s.height()-16,50,14),this,0,"options");

      if(iLastOSD == 1) m_poCurrOSD = m_poOSD;
      else if(iLastOSD == 2) m_poCurrOSD = m_poShowOSD;
    }
    m_oOSDMutex.unlock();
  }

  // }}}

  void ICLWidget::childChanged(int id, void *val){  
    // {{{ open
    (void)val;
    switch(id){
      case SHOW_OSD_ID:
        m_poCurrOSD = m_poOSD;
#ifndef USE_OPENGL_ACCELERATION   
        setAttribute(Qt::WA_PaintOnScreen,false); 
        setAttribute(Qt::WA_NoBackground,false);
#endif
        break;
      case OSD::BOTTOM_EXIT_ID:
        m_poCurrOSD = m_poShowOSD;
#ifndef USE_OPENGL_ACCELERATION   
        setAttribute(Qt::WA_PaintOnScreen,true); 
        setAttribute(Qt::WA_NoBackground,true);
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
      default:
        break;
    }
    up();
  }
  // }}}
  void ICLWidget::setImage(ImgI *input){
    // {{{ open
    if(!op.on){
      return;
    }
    if(!input){
      m_oMutex.lock();    
      m_oQImage = QImage();
      m_oMutex.unlock();
      return;
    }
    
    m_oMutex.lock();    
    Size s = input->getSize();
        
    Img8u *in8u = input->getDepth()==depth8u ? input->asImg<icl8u>() : 0;
    Img32f *in32f = input->getDepth()==depth32f ? input->asImg<icl32f>() : 0;
    
    if(op.c<0){
      if(in8u){
        switch(input->getChannels()){
          case 2: // use r and b
            m_poImage->resize(s);
            m_poImage->replaceChannel(0,in8u,1);
            m_poImage->clear(1,0);               // green = 0
            m_poImage->replaceChannel(2,in8u,0);
            m_poImage->clear(3,255);
            break;
          case 3: // flip r and b
            m_poImage->resize(s); 
            m_poImage->replaceChannel(0,in8u,2);
            m_poImage->replaceChannel(1,in8u,1);
            m_poImage->replaceChannel(2,in8u,0);
            m_poImage->clear(3,255);
            break;
          case 4: // using rgba for visualization
            m_poImage->resize(s); 
            m_poImage->replaceChannel(0,in8u,2);
            m_poImage->replaceChannel(1,in8u,1);
            m_poImage->replaceChannel(2,in8u,0);
            m_poImage->replaceChannel(3,in8u,3);
            break;
          default: // 1 or 5+ channels showin channel 0 as gray image
            op.c=0;
            break;
        }
      }else{
        switch(input->getChannels()){
          case 2: // use r and b
            m_poImage->resize(s);
            deepCopyChannel(in32f,1,m_poImage,0); // blue = green(1)
            m_poImage->clear(1,0);               // green = 0
            deepCopyChannel(in32f,0,m_poImage,2); // red = red(0)
            m_poImage->clear(3,255);
            break;
          case 3: // flip r and b
            m_poImage->resize(s); 
            deepCopyChannel(in32f,2,m_poImage,0); 
            deepCopyChannel(in32f,1,m_poImage,1); 
            deepCopyChannel(in32f,0,m_poImage,2); 
            m_poImage->clear(3,255);
            break;
          case 4: // using rgba for visualization
            m_poImage->resize(s); 
            deepCopyChannel(in32f,2,m_poImage,0); 
            deepCopyChannel(in32f,1,m_poImage,1); 
            deepCopyChannel(in32f,0,m_poImage,2); 
            deepCopyChannel(in32f,3,m_poImage,3); 
            break;
          default: // 1 or 5+ channels showin channel 0 as gray image
            op.c=0;
            break;
         }      
      }
    }
    if(op.c<0){ //creating rgba QImage
      if(m_oQImage.width() != s.width || m_oQImage.height() != s.height || m_oQImage.format() != QImage::Format_ARGB32){
        m_oQImage = QImage(QSize(s.width,s.height),QImage::Format_ARGB32);
      }
      convertToARGB32Interleaved(m_oQImage.bits(),m_poImage);
    }else{
      if(m_oQImage.width() != s.width || m_oQImage.height() != s.height || m_oQImage.format() != QImage::Format_Indexed8){
        m_oQImage = QImage(QSize(s.width,s.height),QImage::Format_Indexed8);
        m_oQImage.setColorTable(m_oColorTable);
      }
      if(in8u){
        copy(in8u->getData(op.c),in8u->getData(op.c)+in8u->getDim(),m_oQImage.bits());
      }else{
        copy(in32f->getData(op.c),in32f->getData(op.c)+in32f->getDim(),m_oQImage.bits());
      }
    }
    m_oMutex.unlock();
  }
  
  // }}}
  void ICLWidget::up(){
    // {{{ open
    update();
  }

  // }}}
  int ICLWidget::w(){
    // {{{ open
    return width();
  }

  // }}}
  int ICLWidget::h(){
    // {{{ open
    return height();
  }

  // }}}
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
  
  void ICLWidget::drawOSD(QPainter *poPainter){
    // {{{ open

    m_oOSDMutex.lock();
    if(m_poCurrOSD){
      poPainter->setFont(QFont("Arial",11));
      m_poCurrOSD->_drawSelf(poPainter,m_iMouseX,m_iMouseY,aiDown);
    }
    m_oOSDMutex.unlock();
  }

  // }}}
  void ICLWidget::paintEvent(QPaintEvent *poEvent){
    // {{{ open
    (void)poEvent;
#ifdef USE_OPENGL_ACCELERATION
    makeCurrent();
#endif
    QPainter oPainter;
    oPainter.begin(this);
    
    drawImage(&oPainter);
    
    customPaintEvent(&oPainter);
    
    drawOSD(&oPainter);
      
    oPainter.end();
  }

  // }}}
  void ICLWidget::drawImage(QPainter *poPainter){
    // {{{ open
    int _w = w();
    int _h = h();
    QRect r(0,0,_w,_h);

    m_oMutex.lock();
    if(!op.on){
      drawRect(poPainter,r,QColor(0,0,0),QColor(0,0,0));
      drawStr(poPainter,"disabled",r);    
    }else if(!m_oQImage.isNull()){
      if(op.rm == rmOff || op.c < 0){
        setBiasAndScale(0.0,1.0);
      }
      Rect r = computeImageRect(m_poImage->getSize(),Size(w(),h()),op.fm);
      poPainter->drawImage(QRect(r.x,r.y,r.width,r.height),m_oQImage);
    }else{
      drawRect(poPainter,r,QColor(0,0,0),QColor(0,0,0));
      drawStr(poPainter,"no image",r);      
    }
    m_oMutex.unlock();
  }

  // }}}
  void ICLWidget::setBiasAndScale(float fBiasRGB, float fScaleRGB){
    // {{{ open
    fBiasRGB+=(float)(op.brightness)/255.0;
    if(op.contrast){
      // not yet implemented
    }else{
      // not yet implemented
    }
    glPixelTransferf(GL_RED_SCALE,fScaleRGB);
    glPixelTransferf(GL_GREEN_SCALE,fScaleRGB);
    glPixelTransferf(GL_BLUE_SCALE,fScaleRGB);
    glPixelTransferf(GL_RED_BIAS,fBiasRGB);
    glPixelTransferf(GL_GREEN_BIAS,fBiasRGB);
    glPixelTransferf(GL_BLUE_BIAS,fBiasRGB);

  }

  // }}}
  void ICLWidget::drawStr(QPainter *poPainter,QString s, QRect r,int iFontSize, QColor c, QFont f){
    // {{{ open
    poPainter->setPen(c);
    f.setPointSize(iFontSize);
    poPainter->setFont(f);
    poPainter->drawText(r,Qt::AlignCenter,s);
  }

  // }}}
  void ICLWidget::drawRect(QPainter *poPainter,QRect r,QColor cBorder, QColor cFill){
    // {{{ open

    poPainter->setBrush(cFill);
    poPainter->setPen(cBorder);
    poPainter->drawRect(r);
  }

  // }}}
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
  std::vector<QString> ICLWidget::getImageInfo(){
    // {{{ open

    std::vector<QString> info;
    ImgI* i = m_poImage;
    if(!i){
      info.push_back("Image is NULL");
      return info;
    }
    info.push_back(QString("depth:   ")+translateDepth(i->getDepth()).c_str());
    info.push_back(QString("size:    ")+QString::number(i->getSize().width)+" x "+QString::number(i->getSize().height));
    info.push_back(QString("channels:")+QString::number(i->getChannels()));
    info.push_back(QString("format:  ")+translateFormat(i->getFormat()).c_str());
    if(i->hasFullROI()){
      info.push_back("roi:   full");
    }else{
      char ac[200];
      Rect r = i->getROI();
      sprintf(ac,"roi:   ((%d,%d),(%d x %d))",r.x,r.y,r.width,r.height);
      info.push_back(QString(ac));
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

}
