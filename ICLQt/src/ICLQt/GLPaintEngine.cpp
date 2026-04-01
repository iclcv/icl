// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#include <ICLQt/GLPaintEngine.h>
#include <QOpenGLWidget>
#include <ICLCore/Img.h>
#include <QFontMetrics>
#include <QPainter>
#include <ICLCore/CCFunctions.h>
#include <ICLQt/GLImg.h>

using namespace icl::utils;
using namespace icl::core;


namespace icl{
  namespace qt{

    namespace{
      Rect32f computeRect(const Rect32f &rect, const Size &imageSize, PaintEngine::AlignMode mode){

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


      // inline float winToDraw(float x, float w) { return (2/w) * x -1; }
      // inline float drawToWin(float x, float w) { return (w/2) * x + (w/2); }
    }

    GLPaintEngine::GLPaintEngine(QOpenGLWidget *widget):
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

    GLPaintEngine::~GLPaintEngine(){
      ICL_DELETE(m_incompDepthBuf);
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();

    }


   void GLPaintEngine::fontsize(float size){
      m_font.setPointSize(size);
    }

    void  GLPaintEngine::font(std::string name, float size, PaintEngine::TextWeight weight, PaintEngine::TextStyle style){
      m_font.setFamily(name.c_str());
      m_font.setPointSize(size);
      m_font.setStyle(style == PaintEngine::StyleNormal ? QFont::StyleNormal :
                       style == PaintEngine::StyleItalic ? QFont::StyleItalic : QFont::StyleOblique);
      m_font.setWeight(weight == PaintEngine::Light ? QFont::Light :
                       weight == PaintEngine::Normal ? QFont::Normal :
                       weight == PaintEngine::DemiBold ? QFont::DemiBold :
                       weight == PaintEngine::Bold ? QFont::Bold : QFont::Black);
    }

    void GLPaintEngine::color(float r, float g, float b, float a){

      m_linecolor[0] = static_cast<float>(r)/255.0;
      m_linecolor[1] = static_cast<float>(g)/255.0;
      m_linecolor[2] = static_cast<float>(b)/255.0;
      m_linecolor[3] = static_cast<float>(a)/255.0;
    }

    void GLPaintEngine::fill(float r, float g, float b, float a){

      m_fillcolor[0] = static_cast<float>(r)/255.0;
      m_fillcolor[1] = static_cast<float>(g)/255.0;
      m_fillcolor[2] = static_cast<float>(b)/255.0;
      m_fillcolor[3] = static_cast<float>(a)/255.0;
    }

    void GLPaintEngine::linewidth(float w){
      m_linewidth = w;
    }

    void GLPaintEngine::pointsize(float s){
      m_pointsize = s;
    }


    void GLPaintEngine::line(const Point32f &a, const Point32f &b){
      glEnable(GL_LINE_SMOOTH);
      glLineWidth(m_linewidth);
      glColor4fv(m_linecolor);
      glBegin(GL_LINES);
      glVertex2f(a.x,a.y);
      glVertex2f(b.x,b.y);
      glEnd();
      glDisable(GL_LINE_SMOOTH);
    }

    void GLPaintEngine::point(const Point32f &p){

      glColor4fv(m_linecolor);
      glPointSize(m_pointsize);
      glBegin(GL_POINTS);
      glVertex2f(static_cast<GLfloat>(p.x),static_cast<GLfloat>(p.y));
      glEnd();
    }


    void GLPaintEngine::image(const Rect32f &r,const QImage &image, PaintEngine::AlignMode mode, scalemode sm){

      Img8u buf;
      if(image.format()==QImage::Format_Indexed8){
        buf = Img8u(Size(image.width(),image.height()),formatGray);
      }else{
        buf = Img8u(Size(image.width(),image.height()),4);
      }

      interleavedToPlanar(image.bits(),&buf);
      this->image(r,&buf,mode,sm);
    }


    void GLPaintEngine::image(const Rect32f &r,ImgBase *image, PaintEngine::AlignMode mode, scalemode sm){

      ICLASSERT_RETURN(image);
      glColor4f(1,1,1,1);
      GLImg gli(image,sm);
      gli.setBCI(m_bci[0],m_bci[1],m_bci[2]);
      gli.draw2D(computeRect(r,image->getSize(),mode), Size(m_widget->width(),m_widget->height()));
    }


    void GLPaintEngine::rect(const Rect32f &r){

      glLineWidth(m_linewidth);
      glColor4fv(m_fillcolor);
      glBegin(GL_QUADS);
      glVertex2f(static_cast<GLfloat>(r.x),static_cast<GLfloat>(r.y));
      glVertex2f(static_cast<GLfloat>(r.right()),static_cast<GLfloat>(r.y));
      glVertex2f(static_cast<GLfloat>(r.right()),static_cast<GLfloat>(r.bottom()));
      glVertex2f(static_cast<GLfloat>(r.x),static_cast<GLfloat>(r.bottom()));
      glEnd();

      glColor4fv(m_linecolor);
      glBegin(GL_LINE_STRIP);
      glVertex2f(static_cast<GLfloat>(r.x),static_cast<GLfloat>(r.y));
      glVertex2f(static_cast<GLfloat>(r.right()),static_cast<GLfloat>(r.y));
      glVertex2f(static_cast<GLfloat>(r.right()),static_cast<GLfloat>(r.bottom()));
      glVertex2f(static_cast<GLfloat>(r.x),static_cast<GLfloat>(r.bottom()));
      glVertex2f(static_cast<GLfloat>(r.x),static_cast<GLfloat>(r.y));
      glEnd();



    }
    void GLPaintEngine::triangle(const Point32f &a, const Point32f &b, const Point32f &c){

      glColor4fv(m_fillcolor);
      glBegin(GL_TRIANGLES);
      glVertex2f(static_cast<GLfloat>(a.x),static_cast<GLfloat>(a.y));
      glVertex2f(static_cast<GLfloat>(b.x),static_cast<GLfloat>(b.y));
      glVertex2f(static_cast<GLfloat>(c.x),static_cast<GLfloat>(c.y));
      glEnd();

      glLineWidth(m_linewidth);
      glEnable(GL_LINE_SMOOTH);
      glColor4fv(m_linecolor);
      glBegin(GL_LINE_LOOP);
      glVertex2f(static_cast<GLfloat>(a.x),static_cast<GLfloat>(a.y));
      glVertex2f(static_cast<GLfloat>(b.x),static_cast<GLfloat>(b.y));
      glVertex2f(static_cast<GLfloat>(c.x),static_cast<GLfloat>(c.y));
      glEnd();
      glDisable(GL_LINE_SMOOTH);
    }

    void GLPaintEngine::quad(const Point32f &a, const Point32f &b, const Point32f &c, const Point32f &d){

      glEnable(GL_LINE_SMOOTH);
      glColor4fv(m_fillcolor);
      glBegin(GL_QUADS);
      glVertex2f(static_cast<GLfloat>(a.x),static_cast<GLfloat>(a.y));
      glVertex2f(static_cast<GLfloat>(b.x),static_cast<GLfloat>(b.y));
      glVertex2f(static_cast<GLfloat>(c.x),static_cast<GLfloat>(c.y));
      glVertex2f(static_cast<GLfloat>(d.x),static_cast<GLfloat>(d.y));
      glEnd();

      glColor4fv(m_linecolor);
      glBegin(GL_LINE_LOOP);
      glVertex2f(static_cast<GLfloat>(a.x),static_cast<GLfloat>(a.y));
      glVertex2f(static_cast<GLfloat>(b.x),static_cast<GLfloat>(b.y));
      glVertex2f(static_cast<GLfloat>(c.x),static_cast<GLfloat>(c.y));
      glVertex2f(static_cast<GLfloat>(d.x),static_cast<GLfloat>(d.y));
      glEnd();
      glDisable(GL_LINE_SMOOTH);
    }


    void GLPaintEngine::ellipse(const Rect32f &r){
      glLineWidth(m_linewidth);
      glEnable(GL_LINE_SMOOTH);
      glColor4fv(m_fillcolor);
      GLfloat w2 = 0.5*(r.width);
      GLfloat h2= 0.5*(r.height);
      GLfloat cx = r.x+w2;
      GLfloat cy = r.y+h2;

      GLint NSTEPS = 32;
      float circumference = 0;
      if(w2 == h2){
        circumference = 2* M_PI * w2;
      }else{
        if(!(w2+h2)){
          circumference = 0;
        }else{
          float l = 3 * (w2-h2) / (w2+h2), l3s = 3*l*l;
          circumference = (w2+h2) * M_PI * ( 1 + l3s / 10 + ::sqrt(4-l3s));
        }
        // compute circumference of ellipse using Ramanujan's approximation
      }
      if(circumference > 50000) NSTEPS = 4000;
      else if(circumference > 10000) NSTEPS = 2000;
      else if(circumference > 5000) NSTEPS = 1000;
      else if(circumference > 1000) NSTEPS = 256;
      else if(circumference > 100) NSTEPS = 64;

      //DEBUG_LOG("circumference: " << circumference << " using nSteps:" << NSTEPS << " Rect was: " << r);

      const GLfloat D_ARC = (2*M_PI)/static_cast<double>(NSTEPS);
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


    Size GLPaintEngine::estimateTextBounds(const std::string &text) const{
      QFontMetrics m(m_font);
      QRectF br = m.boundingRect(text.c_str());
      return Size(br.width(), br.height());
    }


    void GLPaintEngine::text(const Rect32f &r, const std::string text, PaintEngine::AlignMode mode, float angle){
      const qreal dpr = m_widget->devicePixelRatio();
      QFontMetrics m(m_font);
      QRectF br = m.boundingRect(text.c_str());
      int logW = static_cast<int>(br.width() + 4);
      int logH = static_cast<int>(br.height());
      // Render text at native resolution for crisp HiDPI, but keep
      // logical dimensions for GL placement via computeRect.
      QImage img(static_cast<int>(logW * dpr), static_cast<int>(logH * dpr), QImage::Format_ARGB32);
      img.setDevicePixelRatio(dpr);
      img.fill(0);
      QPainter painter(&img);
      painter.setFont(m_font);
      painter.setPen(QColor( static_cast<int>(m_linecolor[2]*255),
                             static_cast<int>(m_linecolor[1]*255),
                             static_cast<int>(m_linecolor[0]*255),
                             std::min (254, static_cast<int>(m_linecolor[3]*255)) ));

      painter.drawText(QRect(0, 0, logW, logH), Qt::AlignHCenter, text.c_str());
      painter.end();

      if(angle){
        QTransform R;
        R.rotate(angle);
        img = img.transformed(R);
        logW = static_cast<int>(img.width() / dpr);
        logH = static_cast<int>(img.height() / dpr);
      }
      // Convert to Img8u at full physical resolution for texture quality
      Img8u buf(Size(img.width(), img.height()), 4);
      interleavedToPlanar(img.bits(), &buf);
      // Draw using logical size so computeRect positions correctly
      glColor4f(1,1,1,1);
      GLImg gli(&buf, interpolateLIN);
      gli.setBCI(m_bci[0], m_bci[1], m_bci[2]);
      gli.draw2D(computeRect(r, Size(logW, logH), mode), Size(m_widget->width(), m_widget->height()));
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


    void GLPaintEngine::bci(float brightness, float contrast, float intensity){
      m_bci[0]=brightness;
      m_bci[1]=contrast;
      m_bci[2]=intensity;
      m_bciauto = false;
    }


    void GLPaintEngine::bciAuto(){

      m_bciauto = true;
    }


    void GLPaintEngine::getColor(float *piColor){

      for(int i=0;i<4;i++){
        piColor[i]=static_cast<int>(m_linecolor[i]*255.0);
      }
    }


    void GLPaintEngine::getFill(float *piColor){

      for(int i=0;i<4;i++){
        piColor[i]=static_cast<int>(m_fillcolor[i]*255.0);
      }
    }



    void GLPaintEngine::setupRasterEngine(const Rect32f& r, const Size32f &s, PaintEngine::AlignMode mode){

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
          glPixelZoom(static_cast<GLfloat>(r.width)/s.width,-static_cast<GLfloat>(r.height)/s.height);
          glRasterPos2i(r.x,static_cast<int>(r.y-r.height+(s.height*static_cast<GLfloat>(r.height)/s.height)));
          break;
      }
    }

    void GLPaintEngine::setPackAlignment(depth d, int linewidth){
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

    void GLPaintEngine::setupPixelTransfer(depth d, float brightness, float contrast, float intensity){
      static_cast<void>(intensity);
      float fBiasRGB = static_cast<float>(brightness)/255.0;

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

      float c = static_cast<float>(contrast)/255;
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

  } // namespace qt
}
