#include <iclGLPaintEngine.h>
#include <QGLWidget>
#include <iclImg.h>
#include <QFontMetrics>
#include <QPainter>
#include <iclMathematics.h>
#include <iclCC.h>
#include <iclGLTextureMapBaseImage.h>

using std::string; using std::min;

namespace icl{

  namespace{
    Rect computeRect(const Rect &rect, const Size &imageSize, PaintEngine::AlignMode mode){
      // {{{ open

      switch(mode){
        case PaintEngine::NoAlign: return Rect(rect.x, rect.y, imageSize.width, imageSize.height);
        case PaintEngine::Centered: {
          int cx  = rect.x+rect.width/2;
          int cy  = rect.y+rect.height/2;
          return Rect(cx-imageSize.width/2,cy-imageSize.height/2,imageSize.width,imageSize.height);
        }
        default:  return rect;
      }
    }

    // }}}
 
    inline float winToDraw(float x, float w) { return (2/w) * x -1; }  
    inline float drawToWin(float x, float w) { return (w/2) * x + (w/2); } 
  }

  GLPaintEngine::GLPaintEngine(QGLWidget *widget):
    // {{{ open
    m_poWidget(widget),m_bBCIAutoFlag(false), m_oFont(QFont("Arial",30)),
    m_poImageBufferForIncompatibleDepth(0){
    
    // widget->makeCurrent();
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
   
    glOrtho(0, widget->width(), widget->height(), 0, -999999, 999999);
   

    glPixelTransferf(GL_ALPHA_SCALE,1);
    glPixelTransferf(GL_RED_SCALE,1);
    glPixelTransferf(GL_GREEN_SCALE,1);
    glPixelTransferf(GL_BLUE_SCALE,1);
    glPixelTransferf(GL_ALPHA_BIAS,0);
    glPixelTransferf(GL_RED_BIAS,0);
    glPixelTransferf(GL_GREEN_BIAS,0);
    glPixelTransferf(GL_BLUE_BIAS,0);

    
    memset(m_afFillColor,0,4*sizeof(float));
    for(int i=0;i<4;m_afLineColor[i++]=255);
    memset(m_aiBCI,0,3*sizeof(int));
  }

  // }}}
  GLPaintEngine::~GLPaintEngine(){
    // {{{ open
    if(m_poImageBufferForIncompatibleDepth) delete m_poImageBufferForIncompatibleDepth;
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

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

  void GLPaintEngine::image(const Rect &r,const QImage &image, PaintEngine::AlignMode mode){
    // {{{ open

    Img8u buf;    
    if(image.format()==QImage::Format_Indexed8){
      buf = Img8u(Size(image.width(),image.height()),formatGray);
    }else{
      buf = Img8u(Size(image.width(),image.height()),4);
    }
    interleavedToPlanar(image.bits(),&buf);
    this->image(r,&buf,mode);
  }

  // }}}

  void GLPaintEngine::image(const Rect &r,ImgBase *image, PaintEngine::AlignMode mode){
    // {{{ open
    
    ICLASSERT_RETURN(image);
    glColor4f(1,1,1,1);
    GLTextureMapBaseImage texmapImage;
    texmapImage.bci(m_aiBCI[0],m_aiBCI[1],m_aiBCI[2]);
    texmapImage.updateTextures(image);
    texmapImage.drawTo(computeRect(r,image->getSize(),mode), Size(m_poWidget->width(),m_poWidget->height()));
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
    
    point(Point(r.right(),r.y));
    
  }
  void GLPaintEngine::triangle(const Point &a, const Point &b, const Point &c){
    glColor4fv(m_afFillColor);
    glBegin(GL_TRIANGLES);
    glVertex2f((GLfloat)a.x,(GLfloat)a.y);
    glVertex2f((GLfloat)b.x,(GLfloat)b.y);
    glVertex2f((GLfloat)c.x,(GLfloat)c.y);
    glEnd();
    
    glColor4fv(m_afLineColor);
    glBegin(GL_LINE_LOOP);
    glVertex2f((GLfloat)a.x,(GLfloat)a.y);
    glVertex2f((GLfloat)b.x,(GLfloat)b.y);
    glVertex2f((GLfloat)c.x,(GLfloat)c.y);
    glEnd();
    
  }
  // }}}
 
  void GLPaintEngine::quad(const Point &a, const Point &b, const Point &c, const Point &d){
    glColor4fv(m_afFillColor);
    glBegin(GL_QUADS);
    glVertex2f((GLfloat)a.x,(GLfloat)a.y);
    glVertex2f((GLfloat)b.x,(GLfloat)b.y);
    glVertex2f((GLfloat)c.x,(GLfloat)c.y);
    glVertex2f((GLfloat)d.x,(GLfloat)d.y);
    glEnd();
    
    glColor4fv(m_afLineColor);
    glBegin(GL_LINE_LOOP);
    glVertex2f((GLfloat)a.x,(GLfloat)a.y);
    glVertex2f((GLfloat)b.x,(GLfloat)b.y);
    glVertex2f((GLfloat)c.x,(GLfloat)c.y);
    glVertex2f((GLfloat)d.x,(GLfloat)d.y);
    glEnd();
    
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
    QImage img(br.width()+2,br.height()+2,QImage::Format_ARGB32);
    img.fill(0);
    QPainter painter(&img);
    painter.setFont(m_oFont);
    painter.setPen(QColor( (int)(m_afLineColor[2]*255),
                           (int)(m_afLineColor[1]*255),
                           (int)(m_afLineColor[0]*255),
                           min (254, (int)(m_afLineColor[3]*255)) ));
   
    painter.drawText(QPoint(1,img.height()-m.descent()-1),text.c_str());
    painter.end();
    
    image(r,img,mode);
    /*
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
    */
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
    float fScaleRGB(1);
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
