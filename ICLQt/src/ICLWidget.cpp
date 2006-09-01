#include "ICLWidget.h"
#include <QImage>
#include <QPainter>
#include <QResizeEvent>

#include "OSD.h"

#include "Img.h"
#include "Timer.h"


namespace icl{  
  const int ICLWidget::SHOW_OSD_ID;
  
  ICLWidget::ICLWidget(QWidget *poParent):ParentWidgetClass(poParent),
    // {{{ open Constructor definition
    m_poImage(imgNew()),
    m_poOSD(0),m_poCurrOSD(0),m_poShowOSD(0),m_iMouseX(-1), m_iMouseY(-1){
   
    setParent(poParent);
    setMouseTracking(true);

#ifndef USE_OPENGL_ACCELERATION 
    for(int i=0;i<256;i++){
      m_oColorTable.push_back(qRgb(i,i,i));
    }
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

  void ICLWidget::setImage(ImgI *poInputImage){
    // {{{ open
    if(!op.on){
      up();
      return;
    }
 
    if(!poInputImage){
      return;
    }

    m_oMutex.lock();    
    bufferImg(poInputImage);
#ifndef USE_OPENGL_ACCELERATION   
    convertImg(m_poImage,m_oQImage);
#endif

    m_oMutex.unlock();
    up();
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

  void ICLWidget::paint2D(QPainter *poPainter){
    // {{{ open

    int _w = w();
    int _h = h();
    QRect r(0,0,_w,_h);
    if(!op.on){
      drawRect(poPainter,r,QColor(0,0,0),QColor(0,0,0));
      drawStr(poPainter,"disabled",r);      
    }else if(!m_poImage){  
      drawRect(poPainter,r,QColor(0,0,0),QColor(0,0,0));
      drawStr(poPainter,"no image",r);      
    }    
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
    // make this images gl context to the current one
    makeCurrent();
    
    // create a QPainter but delete all changings on the GL-Engine
    pushCurrentState();
    QPainter oPainter;
    oPainter.begin((QGLWidget*)this);
    popCurrentState();
    
    // paint the image using openGL
    paintGL();

    // restore the painters changing on the GL-Engine
    restoreQPainterInitialization();
    
    // paint 2D stuff 
    paint2D(&oPainter);
   
    
    // undo the changings on the PROJECTION_MATRIX, done by restoreQPainterInitialization()
    glPopMatrix();
    
    // finish drawing (this will call glFlush(),...)
    oPainter.end();
#else
    QPainter oPainter;
    oPainter.begin(this);

    // draw the image
    m_oMutex.lock();
    // convert the image to rgba8-interleaved

    if(!m_oQImage.isNull()){
      Rect r = computeImageRect(m_poImage->getSize(),Size(w(),h()),op.fm);
      oPainter.drawImage(QRect(r.x,r.y,r.width,r.height),m_oQImage);
    }

    m_oMutex.unlock();

    // draw the OSD
    paint2D(&oPainter);
    oPainter.end();
#endif
  }

  // }}}
 
#ifdef USE_OPENGL_ACCELERATION
  void ICLWidget::initializeGL (){
    // {{{ open

    // this initializations should accelerate 
    // the GL-Pixel engine
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    glDisable(GL_LOGIC_OP);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
    
  }    

  // }}}
  void ICLWidget::paintGL (){
    // {{{ open
    m_oMutex.lock();
    if(!op.on || !m_poImage){
      m_oMutex.unlock();
      return;
    }
    drawImgGL(m_poImage,op.fm,-1);
    m_oMutex.unlock();
  }

  // }}}
  void ICLWidget::pushCurrentState(){
    // {{{ open

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
  }

  // }}}
  void ICLWidget::popCurrentState(){
  // {{{ open

    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
  }

  // }}}
  void ICLWidget::restoreQPainterInitialization(){
    // {{{ open

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glOrtho(0, QGLWidget::width(),QGLWidget::height(), 0, -999999, 999999);
    glViewport(0,0,QGLWidget::width(),QGLWidget::height());
    glColorMask(1,1,1,1); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  }

  // }}}
  void ICLWidget::setPackAllignment(int iImageW, icl::depth eDepth){
    // {{{ open

    GLenum E = GL_UNPACK_ALIGNMENT;
    if(eDepth == depth32f){
      if(!(iImageW % 2)) glPixelStorei(E,8);
      else glPixelStorei(E,4);
    }else{
      if(!(iImageW % 8)) glPixelStorei(E,8);
      else if(!(iImageW % 4)) glPixelStorei(E,4);
      else if(!(iImageW % 2)) glPixelStorei(E,2);
      else glPixelStorei(E,1);
    }    
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
  void ICLWidget::setupPixelEngine(int iImageW, int iImageH, fitmode eFitMode){
    // {{{ open
    Rect r = computeImageRect(Size(iImageW,iImageH),Size(w(),h()),eFitMode);
    glPixelZoom((float)r.width/iImageW,(float)r.height/iImageH);
    glRasterPos2f((2*(float)r.x/w())-1.0,(2*(float)r.y/h())-1.0);
  }

  // }}}
  void ICLWidget::setBiasAndScaleForChannel(int iDepth, int iChannel){
    // {{{ open

    float fMin, fMax;
    unsigned char ucMin, ucMax;
    if(iChannel == -1){
      int aiBiases[3] = {GL_RED_BIAS,GL_GREEN_BIAS,GL_BLUE_BIAS};
      int aiScales[3] = {GL_RED_SCALE,GL_GREEN_SCALE,GL_BLUE_SCALE};
      if(iDepth == 8){
        for(int i=0;i<3;i++){

          m_poImage->asImg<icl8u>()->getMinMax(ucMin,ucMax,i);

          float fScale = 255.0/(float)(ucMax-ucMin);
          glPixelTransferf(aiBiases[i],-fScale*ucMin);
          glPixelTransferf(aiScales[i],fScale);
        }
      }else{
        for(int i=0;i<3;i++){

          m_poImage->asImg<icl32f>()->getMinMax(fMin,fMax,i);

          float fScale = 1.0/(fMax-fMin);
          glPixelTransferf(aiBiases[i],-fScale*fMin);
          glPixelTransferf(aiScales[i],fScale);
        }        
      }
    }else if(iChannel >= 0){
      if(iDepth == 8){
        m_poImage->asImg<icl8u>()->getMinMax(ucMin,ucMax,iChannel);

        float fScale = 255.0/(float)(ucMax-ucMin);
        setBiasAndScale(-fScale*ucMin,fScale);
      }else{

        m_poImage->asImg<icl32f>()->getMinMax(fMin,fMax,iChannel);
        
        float fScale = 1.0/(fMax-fMin);
        setBiasAndScale(-fScale*fMin,fScale);
      }
    }   
  }

  // }}}
  void ICLWidget::drawImgGL(ImgI *poImage, fitmode eFitMode, int iChannel){
    // {{{ open
  
    glColorMask(1,1,1,1);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if(!poImage)return;
    Size s = poImage->getSize();
    icl::depth d = poImage->getDepth();

    setupPixelEngine(s.width,s.height,eFitMode);

    setPackAllignment(s.width, poImage->getDepth());
    

    if(op.rm == rmOff || iChannel < 0){
      setBiasAndScale(0.0,d==depth32f  ?  1.0/255.0 :  1.0);
    }else{
      setBiasAndScaleForChannel(d==depth8u ? 8 : 32, iChannel);
    }

    // drawing of the image using openGL's glDrawPixels
    GLenum eGlFormat = d == depth8u ? GL_UNSIGNED_BYTE : GL_FLOAT;

    if(iChannel < 0){
      if(poImage->getChannels()>=3){
        glColorMask(1,0,0,1);
        glDrawPixels(s.width,s.height,GL_RED,eGlFormat,poImage->getDataPtr(0));
        glColorMask(0,1,0,1);
        glDrawPixels(s.width,s.height,GL_GREEN,eGlFormat,poImage->getDataPtr(1));
        glColorMask(0,0,1,1);
        glDrawPixels(s.width,s.height,GL_BLUE,eGlFormat,poImage->getDataPtr(2));
      }else if (poImage->getChannels()==1){
        // gray-images can be draw, using the GL_LUMINANCE parameter
        glColorMask(1,1,1,1);
        glDrawPixels(s.width,s.height,GL_LUMINANCE,eGlFormat,poImage->getDataPtr(0));
      }
    }else{
      // gray-images from sigle channels can be draw, using the GL_LUMINANCE parameter
      glColorMask(1,1,1,1);
      glDrawPixels(s.width,s.height,GL_LUMINANCE,eGlFormat,poImage->getDataPtr(iChannel/*todo check*/));
    }
  }

  // }}}
#else
 
  void ICLWidget::convertImg(ImgI* src,QImage &dst){
    // {{{ open
    if(src->getDepth() == depth8u){
      if(src->getChannels()>=3){
        if(src->getSize() != Size(dst.width(),dst.height()) || dst.format() != QImage::Format_ARGB32){
          dst = QImage(QSize(src->getSize().width,src->getSize().height),QImage::Format_ARGB32);
        }
        
        uchar *dstData = dst.bits();
        icl8u *srcData0 = src->asImg<icl8u>()->getData(0);
        icl8u *srcData1 = src->asImg<icl8u>()->getData(1);
        icl8u *srcData2 = src->asImg<icl8u>()->getData(2);
        uchar *dstDataEnd = dstData+src->getDim()*4;
        for(; dstData != dstDataEnd; ++srcData0, ++srcData1, ++srcData2){
        
          *dstData++ = *srcData0;
          *dstData++ = *srcData1;
          *dstData++ = *srcData2;
          *dstData++ = 255;
        }
      }else{
        if(src->getSize() != Size(dst.width(),dst.height()) || dst.format() != QImage::Format_Indexed8){
          dst = QImage(QSize(src->getSize().width,src->getSize().height),QImage::Format_Indexed8);
          dst.setColorTable(m_oColorTable);
        }        
        uchar *dstData = dst.bits();
        icl8u *srcData0 = src->asImg<icl8u>()->getData(0);
        uchar *dstDataEnd = dstData+src->getDim();
        for(; dstData != dstDataEnd; ++srcData0,++dstData){
          *dstData = *srcData0;
        }
      }
    }else{
        if(src->getChannels()>=3){
        if(src->getSize() != Size(dst.width(),dst.height()) || dst.format() != QImage::Format_ARGB32){
          dst = QImage(QSize(src->getSize().width,src->getSize().height),QImage::Format_ARGB32);
        }
        
        uchar *dstData = dst.bits();
        icl32f *srcData0 = src->asImg<icl32f>()->getData(0);
        icl32f *srcData1 = src->asImg<icl32f>()->getData(1);
        icl32f *srcData2 = src->asImg<icl32f>()->getData(2);
        uchar *dstDataEnd = dstData+src->getDim()*4;
        for(; dstData != dstDataEnd; ++srcData0, ++srcData1, ++srcData2){
          *dstData++ = 255;
          *dstData++ = Cast<icl32f,uchar>::cast(*srcData0);
          *dstData++ = Cast<icl32f,uchar>::cast(*srcData1);
          *dstData++ = Cast<icl32f,uchar>::cast(*srcData2);
        }
      }else{
        if(src->getSize() != Size(dst.width(),dst.height()) || dst.format() != QImage::Format_Indexed8){
          dst = QImage(QSize(src->getSize().width,src->getSize().height),QImage::Format_Indexed8);
          dst.setColorTable(m_oColorTable);
        }        
        uchar *dstData = dst.bits();
        icl32f *srcData0 = src->asImg<icl32f>()->getData(0);
        uchar *dstDataEnd = dstData+src->getDim();
        for(; dstData != dstDataEnd; ++srcData0,++dstData){
          *dstData = Cast<icl32f,uchar>::cast(*srcData0);
        }
      }
    }
  }
  // }}}
#endif

  void ICLWidget::bufferImg(ImgI* poSrc){
    // {{{ open
    ImgI** ppoDst = &m_poImage;
    
    ensureCompatible(ppoDst,poSrc);
    
    Rect aoR[2] = { poSrc->getROI(), (*ppoDst)->getROI() };
    poSrc->setFullROI();
    (*ppoDst)->setFullROI();
    poSrc->flippedCopyROI(*ppoDst,axisHorz);
    poSrc->setROI(aoR[0]);
    (*ppoDst)->setROI(aoR[1]);
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
