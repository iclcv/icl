/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/GLPaintEngine.cpp                            **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter, Robert Haschke                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQt/GLPaintEngine.h>
#include <QGLWidget>
#include <ICLCore/Img.h>
#include <QFontMetrics>
#include <QPainter>
#include <ICLCore/Mathematics.h>
#include <ICLCC/CCFunctions.h>
#include <ICLQt/GLTextureMapBaseImage.h>

using std::string; using std::min;

namespace icl{

  namespace{
    Rect32f computeRect(const Rect32f &rect, const Size &imageSize, PaintEngine::AlignMode mode){
      // {{{ open

      switch(mode){
        case PaintEngine::NoAlign: return Rect32f(rect.x, rect.y, imageSize.width, imageSize.height);
        case PaintEngine::Centered: {
          float cx  = rect.x+rect.width/2;
          float cy  = rect.y+rect.height/2;
          return Rect32f(cx-imageSize.width/2,cy-imageSize.height/2,imageSize.width,imageSize.height);
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
    m_widget(widget),m_bciauto(false), m_font(QFont("Arial",30)),
    m_incompDepthBuf(0){
    
    m_linewidth = 1;
    m_pointsize = 1;

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

    
    std::fill(m_fillcolor,m_fillcolor+4,0);
    std::fill(m_linecolor,m_linecolor+4,255);
    std::fill(m_bci,m_bci+3,0);
  }

  // }}}
  GLPaintEngine::~GLPaintEngine(){
    // {{{ open
    ICL_DELETE(m_incompDepthBuf);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

  } 

  // }}}
  
 void GLPaintEngine::fontsize(float size){
    // {{{ open
    m_font.setPointSize(size);
  }

  // }}}
  void  GLPaintEngine::font(string name, float size, PaintEngine::TextWeight weight, PaintEngine::TextStyle style){
    // {{{ open
    m_font.setFamily(name.c_str());
    m_font.setPointSize(size);
    m_font.setStyle(style == PaintEngine::StyleNormal ? QFont::StyleNormal :
                     style == PaintEngine::StyleItalic ? QFont::StyleItalic : QFont::StyleOblique);
    m_font.setWeight(weight == PaintEngine::Light ? QFont::Light :
                     weight == PaintEngine::Normal ? QFont::Normal :
                     weight == PaintEngine::DemiBold ? QFont::DemiBold :
                     weight == PaintEngine::Bold ? QFont::Bold : QFont::Black);
  }

  // }}}
  void GLPaintEngine::color(float r, float g, float b, float a){
    // {{{ open

    m_linecolor[0] = (float)r/255.0;
    m_linecolor[1] = (float)g/255.0;
    m_linecolor[2] = (float)b/255.0;
    m_linecolor[3] = (float)a/255.0;
  }

  // }}}
  void GLPaintEngine::fill(float r, float g, float b, float a){
    // {{{ open

    m_fillcolor[0] = (float)r/255.0;
    m_fillcolor[1] = (float)g/255.0;
    m_fillcolor[2] = (float)b/255.0;
    m_fillcolor[3] = (float)a/255.0;
  }

  void GLPaintEngine::linewidth(float w){
    m_linewidth = w;
  }

  void GLPaintEngine::pointsize(float s){
    m_pointsize = s;
  }


  // }}}
  void GLPaintEngine::line(const Point32f &a, const Point32f &b){
    // {{{ open
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(m_linewidth);
    glColor4fv(m_linecolor);
    glBegin(GL_LINES);
    glVertex2f(a.x,a.y);
    glVertex2f(b.x,b.y);
    glEnd();
    glDisable(GL_LINE_SMOOTH);
  }

  // }}}
  void GLPaintEngine::point(const Point32f &p){
    // {{{ open

    glColor4fv(m_linecolor);
    glPointSize(m_pointsize);
    glBegin(GL_POINTS);
    glVertex2f((GLfloat)p.x,(GLfloat)p.y);
    glEnd();
  }

  // }}}

  void GLPaintEngine::image(const Rect32f &r,const QImage &image, PaintEngine::AlignMode mode, scalemode sm){
    // {{{ open

    Img8u buf;    
    if(image.format()==QImage::Format_Indexed8){
      buf = Img8u(Size(image.width(),image.height()),formatGray);
    }else{
      buf = Img8u(Size(image.width(),image.height()),4);
    }
    interleavedToPlanar(image.bits(),&buf);
    this->image(r,&buf,mode,sm);
  }

  // }}}

  void GLPaintEngine::image(const Rect32f &r,ImgBase *image, PaintEngine::AlignMode mode, scalemode sm){
    // {{{ open
    
    ICLASSERT_RETURN(image);
    glColor4f(1,1,1,1);
    GLTextureMapBaseImage texmapImage;
    texmapImage.bci(m_bci[0],m_bci[1],m_bci[2]);
    texmapImage.updateTextures(image);
    texmapImage.drawTo(computeRect(r,image->getSize(),mode), Size(m_widget->width(),m_widget->height()),sm);
  }

  // }}}

  void GLPaintEngine::rect(const Rect32f &r){
    // {{{ open

    glLineWidth(m_linewidth);
    glColor4fv(m_fillcolor);
    glBegin(GL_QUADS);
    glVertex2f((GLfloat)r.x,(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.bottom());
    glVertex2f((GLfloat)r.x,(GLfloat)r.bottom());
    glEnd();
    
    glColor4fv(m_linecolor);
    glBegin(GL_LINE_LOOP);
    glVertex2f((GLfloat)r.x,(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.y);
    glVertex2f((GLfloat)r.right(),(GLfloat)r.bottom());
    glVertex2f((GLfloat)r.x,(GLfloat)r.bottom());
    glEnd();
    
    
    
  }
  void GLPaintEngine::triangle(const Point32f &a, const Point32f &b, const Point32f &c){

    glColor4fv(m_fillcolor);
    glBegin(GL_TRIANGLES);
    glVertex2f((GLfloat)a.x,(GLfloat)a.y);
    glVertex2f((GLfloat)b.x,(GLfloat)b.y);
    glVertex2f((GLfloat)c.x,(GLfloat)c.y);
    glEnd();

    glLineWidth(m_linewidth);
    glEnable(GL_LINE_SMOOTH);    
    glColor4fv(m_linecolor);
    glBegin(GL_LINE_LOOP);
    glVertex2f((GLfloat)a.x,(GLfloat)a.y);
    glVertex2f((GLfloat)b.x,(GLfloat)b.y);
    glVertex2f((GLfloat)c.x,(GLfloat)c.y);
    glEnd();
    glDisable(GL_LINE_SMOOTH);
  }
  // }}}
 
  void GLPaintEngine::quad(const Point32f &a, const Point32f &b, const Point32f &c, const Point32f &d){

    glEnable(GL_LINE_SMOOTH);    
    glColor4fv(m_fillcolor);
    glBegin(GL_QUADS);
    glVertex2f((GLfloat)a.x,(GLfloat)a.y);
    glVertex2f((GLfloat)b.x,(GLfloat)b.y);
    glVertex2f((GLfloat)c.x,(GLfloat)c.y);
    glVertex2f((GLfloat)d.x,(GLfloat)d.y);
    glEnd();

    glColor4fv(m_linecolor);
    glBegin(GL_LINE_LOOP);
    glVertex2f((GLfloat)a.x,(GLfloat)a.y);
    glVertex2f((GLfloat)b.x,(GLfloat)b.y);
    glVertex2f((GLfloat)c.x,(GLfloat)c.y);
    glVertex2f((GLfloat)d.x,(GLfloat)d.y);
    glEnd();
    glDisable(GL_LINE_SMOOTH);
  }
  // }}}
  

  void GLPaintEngine::ellipse(const Rect32f &r){
    // {{{ open
    glLineWidth(m_linewidth);
    glEnable(GL_LINE_SMOOTH);
    glColor4fv(m_fillcolor);
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
    
    glColor4fv(m_linecolor);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<NSTEPS;i++){
      float arc = i*D_ARC;
      glVertex2f(cx+std::cos(arc)*w2,cy+std::sin(arc)*h2);
    }
    glVertex2f(cx+std::cos(float(0))*w2,cy+std::sin(float(0))*h2);
    glEnd();
    glDisable(GL_LINE_SMOOTH);
  }

  // }}}
  void GLPaintEngine::text(const Rect32f &r, const string text, PaintEngine::AlignMode mode){
    // {{{ open
    QFontMetrics m(m_font);
    QRectF br = m.boundingRect(text.c_str());
    QImage img(br.width()+2,br.height()+2,QImage::Format_ARGB32);
    img.fill(0);
    QPainter painter(&img);
    painter.setFont(m_font);
    painter.setPen(QColor( (int)(m_linecolor[2]*255),
                           (int)(m_linecolor[1]*255),
                           (int)(m_linecolor[0]*255),
                           min (254, (int)(m_linecolor[3]*255)) ));
   
    painter.drawText(QPointF(1,img.height()-m.descent()-1),text.c_str());
    painter.end();
    
    image(r,img,mode,interpolateLIN);
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
  
  void GLPaintEngine::bci(float brightness, float contrast, float intensity){
    // {{{ open
    m_bci[0]=brightness;
    m_bci[1]=contrast;
    m_bci[2]=intensity;
    m_bciauto = false;
  }

  // }}}

  void GLPaintEngine::bciAuto(){
    // {{{ open

    m_bciauto = true;
  }

  // }}}

  void GLPaintEngine::getColor(float *piColor){
    // {{{ open

    for(int i=0;i<4;i++){
      piColor[i]=(int)(m_linecolor[i]*255.0);
    }
  }

  // }}}
  
  void GLPaintEngine::getFill(float *piColor){
    // {{{ open

    for(int i=0;i<4;i++){
      piColor[i]=(int)(m_fillcolor[i]*255.0);
    }
  }

  // }}}
  

  void GLPaintEngine::setupRasterEngine(const Rect32f& r, const Size32f &s, PaintEngine::AlignMode mode){
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
  void GLPaintEngine::setupPixelTransfer(depth d, float brightness, float contrast, float intensity){
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
