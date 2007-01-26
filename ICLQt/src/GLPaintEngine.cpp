#include <GLPaintEngine.h>
#include <QGLWidget>
#include <Img.h>
#include <QFontMetrics>
#include <QPainter>

#include <Mathematics.h>

using std::string; using std::min;

namespace icl{

  GLPaintEngine::GLPaintEngine(QGLWidget *widget):
    // {{{ open

    m_poWidget(widget),m_bBCIAutoFlag(false), m_oFont(QFont("Arial",30)),
    m_poImageBufferForIncompatibleDepth(0){
    
    widget->makeCurrent();
    
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
    QSize sz(widget->size());
    glViewport(0, 0, sz.width(), sz.height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    memset(m_afFillColor,0,3*sizeof(float));
    for(int i=0;i<4;m_afLineColor[i++]=255);
    memset(m_aiBCI,0,3*sizeof(int));
  }

  // }}}
  GLPaintEngine::~GLPaintEngine(){
    // {{{ open
    if(m_poImageBufferForIncompatibleDepth) delete m_poImageBufferForIncompatibleDepth;
    glFlush();
  } 

  // }}}
  
 void GLPaintEngine::fontsize(int size){
    // {{{ open
    m_oFont.setPointSize(size);
  }

  // }}}
  void  GLPaintEngine::font(string name, int size, PaintEngine::TextWeight weight, PaintEngine::TextStyle style){
    // {{{ open
    m_oFont.setFamily(name.c_str());
    m_oFont.setPointSize(size);
    m_oFont.setStyle(style == PaintEngine::StyleNormal ? QFont::StyleNormal :
                     style == PaintEngine::StyleItalic ? QFont::StyleItalic : QFont::StyleOblique);
    m_oFont.setWeight(weight == PaintEngine::Light ? QFont::Light :
                      weight == PaintEngine::Normal ? QFont::Normal :
                      weight == PaintEngine::DemiBold ? QFont::DemiBold :
                      weight == PaintEngine::Bold ? QFont::Bold : QFont::Black);
  }

  // }}}
  void GLPaintEngine::color(int r, int g, int b, int a){
    // {{{ open

    m_afLineColor[0] = (float)r/255.0;
    m_afLineColor[1] = (float)g/255.0;
    m_afLineColor[2] = (float)b/255.0;
    m_afLineColor[3] = (float)a/255.0;
  }

  // }}}
  void GLPaintEngine::fill(int r, int g, int b, int a){
    // {{{ open

    m_afFillColor[0] = (float)r/255.0;
    m_afFillColor[1] = (float)g/255.0;
    m_afFillColor[2] = (float)b/255.0;
    m_afFillColor[3] = (float)a/255.0;
  }

  // }}}
  void GLPaintEngine::line(const Point &a, const Point &b){
    // {{{ open

    glColor4fv(m_afLineColor);
    glBegin(GL_LINES);
    glVertex2f(a.x,a.y);
    glVertex2f(b.x,b.y);
    glEnd();
  }

  // }}}
  void GLPaintEngine::point(const Point &p){
    // {{{ open

    glColor4fv(m_afLineColor);
    glBegin(GL_POINTS);
    glVertex2f((GLfloat)p.x,(GLfloat)p.y);
    glEnd();
  }

  // }}}
  void GLPaintEngine::image(const Rect &r,ImgBase *image, PaintEngine::AlignMode mode){
    // {{{ open
    Size s = image->getSize();
    setupRasterEngine(r,s,mode);
    setPackAlignment(image->getDepth(),s.width);
   
    if(!m_bBCIAutoFlag){
      setupPixelTransfer(image->getDepth(),m_aiBCI[0],m_aiBCI[1],m_aiBCI[2]);
    }else{
      // automatic adjustment of brightness and contrast
      float fScaleRGB,fBiasRGB;
      // auto adaption
      switch (image->getDepth()){
        case depth8u:{
          icl8u tMin,tMax;
          image->asImg<icl8u>()->getMinMax(tMin,tMax);
          fScaleRGB  = (tMax == tMin) ? 255 : 255.0/(tMax-tMin);
          fBiasRGB = (- fScaleRGB * tMin)/255.0;
          break;
        }
        case depth16s:{
          static const icl16s _max = (65536/2-1);
          icl16s tMin,tMax;
          image->asImg<icl16s>()->getMinMax(tMin,tMax);
          fScaleRGB  = (tMax == tMin) ? _max : _max/(tMax-tMin);
          fBiasRGB = (- fScaleRGB * tMin)/255.0;
          break;
        }
        case depth32s:{ // drawn as float
          icl32s tMin,tMax;
          image->asImg<icl32s>()->getMinMax(tMin,tMax);
          fScaleRGB  = (tMax == tMin) ? 255 : 255/(tMax-tMin);
          fBiasRGB = (- fScaleRGB * tMin)/255.0;
          break;
        }
        case depth32f:{
          icl32f tMin,tMax;
          image->asImg<icl32f>()->getMinMax(tMin,tMax);
          fScaleRGB  = (tMax == tMin) ? 255 : 255.0/(tMax-tMin);
          fBiasRGB = (- fScaleRGB * tMin)/255.0;
          fScaleRGB /= 255.0;
          break;
        }
        case depth64f:{ // drawn as float
          icl64f tMin,tMax;
          image->asImg<icl64f>()->getMinMax(tMin,tMax);
          fScaleRGB  = (tMax == tMin) ? 255 : 255.0/(tMax-tMin);
          fBiasRGB = (- fScaleRGB * tMin)/255.0;
          fScaleRGB /= 255.0;
          break;
        }
        default:
          ICL_INVALID_FORMAT;
          break;
      }
      glPixelTransferf(GL_RED_SCALE,fScaleRGB);
      glPixelTransferf(GL_GREEN_SCALE,fScaleRGB);
      glPixelTransferf(GL_BLUE_SCALE,fScaleRGB);
      glPixelTransferf(GL_RED_BIAS,fBiasRGB);
      glPixelTransferf(GL_GREEN_BIAS,fBiasRGB);
      glPixelTransferf(GL_BLUE_BIAS,fBiasRGB);
    }
  
    // old
    // GLenum datatype = image->getDepth() == depth8u ? GL_UNSIGNED_BYTE : GL_FLOAT; // TODO_depth
    // end old
    GLenum datatype;
    switch(image->getDepth()){
      case depth8u: datatype = GL_UNSIGNED_BYTE; break;
      case depth16s: datatype = GL_SHORT; break;
      case depth32s:
      case depth32f:
      case depth64f: datatype = GL_FLOAT; break;
    }
    static GLenum CHANNELS[4] = {GL_RED,GL_GREEN,GL_BLUE,GL_ALPHA};
    
    ImgBase *drawImage=image;

    if(image->getDepth() == depth32s || image->getDepth() == depth64f){
      // use fallback image conversion before drawing, as "int" and "double" are not supported yet
      ensureCompatible((ImgBase**)&m_poImageBufferForIncompatibleDepth,depth32f,image->getParams());
      drawImage = m_poImageBufferForIncompatibleDepth;
      image->deepCopy(drawImage);
    }
    
    if(drawImage->getChannels() > 1){ 
      for(int i=0;i<4 && i<drawImage->getChannels();i++){
        glColorMask(i==0,i==1,i==2,i==3);
        glDrawPixels(s.width,s.height,CHANNELS[i],datatype,drawImage->getDataPtr(i));
      }
      glColorMask(1,1,1,1);
    }else if(drawImage->getChannels() > 0){
      glColorMask(1,1,1,0);
      glDrawPixels(s.width,s.height,GL_LUMINANCE,datatype,drawImage->getDataPtr(0));
    }
  }

  // }}}
  void GLPaintEngine::image(const Rect &r,const QImage &image, PaintEngine::AlignMode mode){
    // {{{ open
    setupPixelTransfer(depth8u,m_aiBCI[0],m_aiBCI[1],m_aiBCI[2]);
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    setupRasterEngine(r, Size(image.width(),image.height()),mode);
    glDrawPixels(image.width(),image.height(),GL_RGBA,GL_UNSIGNED_BYTE,image.bits());
  }

  // }}}
  void GLPaintEngine::rect(const Rect &r){
    // {{{ open

    glColor4fv(m_afFillColor);
    glBegin(GL_QUADS);
    glVertex2f((GLfloat)r.x,(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.bottom());
    glVertex2f((GLfloat)r.x,(GLfloat)r.bottom());
    glEnd();
    
    glColor4fv(m_afLineColor);
    glBegin(GL_LINE_LOOP);
    glVertex2f((GLfloat)r.x,(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.bottom());
    glVertex2f((GLfloat)r.x,(GLfloat)r.bottom());
    glEnd();
    
    point(Point(r.right(),r.top()));
    
  }

  // }}}
  void GLPaintEngine::ellipse(const Rect &r){
    // {{{ open
    glColor4fv(m_afFillColor);
    GLfloat w2 = 0.5*(r.width);
    GLfloat h2= 0.5*(r.height);
    GLfloat cx = r.x+w2;
    GLfloat cy = r.y+h2;
    static const GLint NSTEPS = 32;
    static const GLfloat D_ARC = (2*M_PI)/NSTEPS;
    glBegin(GL_POLYGON);
    for(int i=0;i<NSTEPS;i++){
      float arc = i*D_ARC;
      glVertex2f(cx+std::cos(arc)*w2,cy+std::sin(arc)*h2);
    }
    glEnd();
    
    glColor4fv(m_afLineColor);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<NSTEPS;i++){
      float arc = i*D_ARC;
      glVertex2f(cx+std::cos(arc)*w2,cy+std::sin(arc)*h2);
    }
    glVertex2f(cx+std::cos(float(0))*w2,cy+std::sin(float(0))*h2);
    glEnd();
  }

  // }}}
  void GLPaintEngine::text(const Rect &r, const string text, PaintEngine::AlignMode mode){
    // {{{ open
    QFontMetrics m(m_oFont);
    QRect br = m.boundingRect(text.c_str());
    QImage img;
    
    img = QImage(br.width()+2,br.height()+2,QImage::Format_ARGB32);
    img.fill(0);
    QPainter painter(&img);
    painter.setFont(m_oFont);
    painter.setPen(QColor( (int)(m_afLineColor[2]*255),
                           (int)(m_afLineColor[1]*255),
                           (int)(m_afLineColor[0]*255),
						   min (254, (int)(m_afLineColor[3]*255)) ));
   
    painter.drawText(QPoint(1,img.height()-m.descent()-1),text.c_str());
    painter.end();
    
    setupPixelTransfer(depth8u,0,0,0);
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    

    if(mode == PaintEngine::NoAlign){
      // specialized for no alligned text rendering: 2*img.height() makes the text origin be
      // lower left and not upper left [??]
      setupRasterEngine(Rect(r.x,r.y,img.width(),2*img.height()), Size(img.width(),img.height()),mode);
    }else{
      setupRasterEngine(r, Size(img.width(),img.height()),mode);
    }
    
    glDrawPixels(img.width(),img.height(),GL_RGBA,GL_UNSIGNED_BYTE,img.bits());
  }

  // }}}
  
  void GLPaintEngine::bci(int brightness, int contrast, int intensity){
    // {{{ open
    m_aiBCI[0]=brightness;
    m_aiBCI[1]=contrast;
    m_aiBCI[2]=intensity;
    m_bBCIAutoFlag = false;
  }

  // }}}

  void GLPaintEngine::bciAuto(){
    // {{{ open

    m_bBCIAutoFlag = true;
  }

  // }}}

  void GLPaintEngine::getColor(int *piColor){
    // {{{ open

    for(int i=0;i<4;piColor[i]=(int)m_afLineColor[i],i++);
  }

  // }}}
  
  void GLPaintEngine::getFill(int *piColor){
    // {{{ open

    for(int i=0;i<4;piColor[i]=(int)m_afFillColor[i],i++);
  }

  // }}}
  

  void GLPaintEngine::setupRasterEngine(const Rect& r, const Size &s, PaintEngine::AlignMode mode){
    // {{{ open

    switch(mode){
      case PaintEngine::NoAlign:
        glPixelZoom(1.0,-1.0);
        glRasterPos2i(r.x,r.y-r.height+s.height);
        break;
      case PaintEngine::Centered:
        glRasterPos2i(r.x+(r.width-s.width)/2,r.y+(r.height-s.height)/2);
        glPixelZoom(1.0,-1.0);      
        break;
      case PaintEngine::Justify:
        glPixelZoom((GLfloat)r.width/s.width,-(GLfloat)r.height/s.height);
        glRasterPos2i(r.x,(int)(r.y-r.height+(s.height*(GLfloat)r.height/s.height)));
        break;
    }
  }

  // }}}
  void GLPaintEngine::setPackAlignment(depth d, int linewidth){
    // {{{ open
    switch (d){
      case depth8u:{
        if(linewidth%2) glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        else if(linewidth%4) glPixelStorei(GL_UNPACK_ALIGNMENT,2);
        else if(linewidth%8) glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        else  glPixelStorei(GL_UNPACK_ALIGNMENT,8);
        break;
      }
      case depth16s:{
        if(linewidth%2) glPixelStorei(GL_UNPACK_ALIGNMENT,2);
        else if(linewidth%4) glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        else  glPixelStorei(GL_UNPACK_ALIGNMENT,8);
        break;
      }        
      case depth32s:
      case depth32f:{
        if(linewidth%2) glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        else glPixelStorei(GL_UNPACK_ALIGNMENT,8);
        break;
      }
      default:
        ICL_INVALID_FORMAT;
        break;
    }
  }

  // }}}
  void GLPaintEngine::setupPixelTransfer(depth d, int brightness, int contrast, int intensity){
    // {{{ open
    (void)intensity;
    float fBiasRGB = (float)brightness/255.0;
    
    // old
    // float fScaleRGB = d == depth8u ? 1.0 : 1.0/255;  //TODO_depth
    // end old
    float fScaleRGB;
    switch(d){
      case depth8u:
        fScaleRGB = 1; break;
      case depth16s:
        fScaleRGB = 127; break; // or 255 ?
      case depth32s:
      case depth32f:
      case depth64f:
        fScaleRGB = 1.0/255; break;
    }
        
    float c = (float)contrast/255;
    if(c>0) c*=10;
    fScaleRGB*=(1.0+c);
    fBiasRGB-=c/2;

    glPixelTransferf(GL_RED_SCALE,fScaleRGB);
    glPixelTransferf(GL_GREEN_SCALE,fScaleRGB);
    glPixelTransferf(GL_BLUE_SCALE,fScaleRGB);
    glPixelTransferf(GL_RED_BIAS,fBiasRGB);
    glPixelTransferf(GL_GREEN_BIAS,fBiasRGB);
    glPixelTransferf(GL_BLUE_BIAS,fBiasRGB);
  }

  // }}}
}
