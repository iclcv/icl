#include "GLPaintEngine.h"
#include <QGLWidget>
#include <Img.h>
#include <QFontMetrics>
#include <QPainter>



namespace icl{

  GLPaintEngine::GLPaintEngine(QGLWidget *widget):
    // {{{ open

    m_poWidget(widget), m_oFont(QFont("Arial",30)){
    
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
  }

  // }}}
  GLPaintEngine::~GLPaintEngine(){
    // {{{ open

    glFlush();
  } 

  // }}}
  
 void GLPaintEngine::fontsize(int size){
    // {{{ open
    m_oFont.setPointSize(size);
  }

  // }}}
  void  GLPaintEngine::font(string name, int size){
    // {{{ open
    m_oFont.setFamily(name.c_str());
    m_oFont.setPointSize(size);
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
  void GLPaintEngine::line(int x1, int y1, int x2, int y2){
    // {{{ open

    glColor4fv(m_afLineColor);
    glBegin(GL_LINES);
    glVertex2f(x1,y1);
    glVertex2f(x2,y2);
    glEnd();
  }

  // }}}
  void GLPaintEngine::point(int x,int y){
    // {{{ open

    glColor4fv(m_afLineColor);
    glBegin(GL_POINTS);
    glVertex2f((GLfloat)x,(GLfloat)y);
    glEnd();
  }

  // }}}
  void GLPaintEngine::image(const Rect &r,ImgI *image, AlignMode mode){
    // {{{ open
    Size s = image->getSize();
    setupRasterEngine(r,s,mode);
    setPackAlignment(image->getDepth(),s.width);
  
    GLenum datatype = image->getDepth() == depth8u ? GL_UNSIGNED_BYTE : GL_FLOAT;
    static GLenum CHANNELS[4] = {GL_RED,GL_GREEN,GL_BLUE,GL_ALPHA};
    
    if(image->getChannels() > 1){ 
      for(int i=0;i<4 && i<image->getChannels();i++){
        glColorMask(i==0,i==1,i==2,i==3);
        glDrawPixels(s.width,s.height,CHANNELS[i],datatype,image->getDataPtr(i));
      }
      glColorMask(1,1,1,1);
    }else if(image->getChannels() > 0){
      glColorMask(1,1,1,0);
      glDrawPixels(s.width,s.height,GL_LUMINANCE,datatype,image->getDataPtr(0));
    }
  }

  // }}}
  void GLPaintEngine::image(const Rect &r,const QImage &image, AlignMode mode){
    // {{{ open
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
    glBegin(GL_LINE_STRIP);
    glVertex2f((GLfloat)r.x,(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.bottom());
    glVertex2f((GLfloat)r.x,(GLfloat)r.bottom());
    glVertex2f((GLfloat)r.x,(GLfloat)r.y);
    glEnd();
    
    point(r.right(),r.top());
    
  }

  // }}}
  void GLPaintEngine::ellipse(const Rect &r){
    // {{{ open
    printf("drawing ellipses is not supported yet \n");
  }

  // }}}
  void GLPaintEngine::text(const Rect &r, const string text, AlignMode mode){
    // {{{ open
    QFontMetrics m(m_oFont);
    QRect br = m.boundingRect(text.c_str());
    
    QImage img(br.width(),br.height(),QImage::Format_ARGB32);
    img.fill(0);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::TextAntialiasing,true);
    painter.setFont(m_oFont);
    painter.setPen(QColor( (int)(m_afLineColor[2]*255),
                           (int)(m_afLineColor[1]*255),
                           (int)(m_afLineColor[0]*255),
                           (int)(m_afLineColor[3]*255) ));
    
    painter.drawText(QPoint(0,img.height()-1),text.c_str());
    painter.end();
    
    image(r,img,mode);
    
    color(255,255,255);
    fill(0,0,0,0);
    rect(r);
  }

  // }}}
  
  void GLPaintEngine::setupRasterEngine(const Rect& r, const Size &s, AlignMode mode){
    // {{{ open

    switch(mode){
      case NoAlign:
        glPixelZoom(1.0,-1.0);
        glRasterPos2i(r.x,r.y-r.height+s.height);
        break;
      case Centered:
        glRasterPos2i(r.x+(r.width-s.width)/2,r.y+(r.height-s.height)/2);
        glPixelZoom(1.0,-1.0);      
        break;
      case Justify:
        glPixelZoom((GLfloat)r.width/s.width,-(GLfloat)r.height/s.height);
        glRasterPos2i(r.x,(int)(r.y-r.height+(s.height*(GLfloat)r.height/s.height)));
        break;
    }
  }

  // }}}
  void GLPaintEngine::setPackAlignment(depth d, int linewidth){
    // {{{ open
    if(d==depth8u){
      if(linewidth%2) glPixelStorei(GL_UNPACK_ALIGNMENT,1);
      else if(linewidth%4) glPixelStorei(GL_UNPACK_ALIGNMENT,2);
      else if(linewidth%8) glPixelStorei(GL_UNPACK_ALIGNMENT,4);
      else  glPixelStorei(GL_UNPACK_ALIGNMENT,8);
    }else{
      if(linewidth%2) glPixelStorei(GL_UNPACK_ALIGNMENT,4);
      else glPixelStorei(GL_UNPACK_ALIGNMENT,8);
    }
  }

  // }}}
}
