#include "QtPaintEngine.h"
#include <QWidget>
#include <Img.h>
#include <QFontMetrics>
#include <QPainter>
#include <Macros.h>
#include <ICLcc.h>

using std::string;

namespace icl{

  QtPaintEngine::QtPaintEngine(QWidget *widget):
    // {{{ open
      
    m_poWidget(widget), 
    m_oFont(QFont("Arial",30)),
    m_poScaledImage(0){
    m_oPainter.begin(widget);
  }

  // }}}
  QtPaintEngine::~QtPaintEngine(){
    // {{{ open

    m_oPainter.end();
  } 

  // }}}
  
 void QtPaintEngine::fontsize(int size){
    // {{{ open
    m_oFont.setPointSize(size);
  }

  // }}}
  void  QtPaintEngine::font(string name, int size, TextWeight weight, TextStyle style){
    // {{{ open
    m_oFont.setFamily(name.c_str());
    m_oFont.setPointSize(size);
    m_oFont.setStyle(style == StyleNormal ? QFont::StyleNormal :
                     style == StyleItalic ? QFont::StyleItalic : QFont::StyleOblique);
    m_oFont.setWeight(weight == Light ? QFont::Light :
                      weight == Normal ? QFont::Normal :
                      weight == DemiBold ? QFont::DemiBold :
                      weight == Bold ? QFont::Bold : QFont::Black);
  }

  // }}}
  void QtPaintEngine::color(int r, int g, int b, int a){
    // {{{ open
    m_oPainter.setPen(QColor(r,g,b,a));
  }

  // }}}
  void QtPaintEngine::fill(int r, int g, int b, int a){
    // {{{ open
    m_oPainter.setBrush(QColor(r,g,b,a));
  }

  // }}}
  void QtPaintEngine::line(const Point &a, const Point &b){
    // {{{ open
    m_oPainter.drawLine(a.x,a.y,b.x,b.y);
  }

  // }}}
  void QtPaintEngine::point(const Point &p){
    // {{{ open
    m_oPainter.drawPoint(p.x,p.y);
  }

  // }}}
  void QtPaintEngine::image(const Rect &r,ImgI *image, AlignMode mode){
    // {{{ open
    if(!image)return;
    ensureCompatible(&m_poScaledImage,image->getDepth(),r.size(),image->getChannels());
    image->scaledCopy(m_poScaledImage);
    m_oQImageConverter.setImage(m_poScaledImage);
    this->image(r,*(m_oQImageConverter.getQImage()),mode);
  }

  // }}}
  void QtPaintEngine::image(const Rect &r,const QImage &image, AlignMode mode){
    // {{{ open
    switch(mode){
      case NoAlign:
        m_oPainter.drawImage(QRect(r.x,r.y,r.width,r.height),image);
        break;
      case Centered:
        m_oPainter.drawImage(QRect(r.x,r.y,r.width,r.height),image);
        break;
      case Justify:
        m_oPainter.drawImage(QRect(r.x,r.y,r.width,r.height),image);
        break;
    }
    
  }

  // }}}
  void QtPaintEngine::rect(const Rect &r){
    // {{{ open
    m_oPainter.drawRect(QRect(r.x,r.y,r.width,r.height));
  }

  // }}}
  void QtPaintEngine::ellipse(const Rect &r){
    // {{{ open
    m_oPainter.drawEllipse(QRect(r.x,r.y,r.width,r.height));
  }

  // }}}
  void QtPaintEngine::text(const Rect &r, const string text, AlignMode mode){
    // {{{ open
    
    switch(mode){
      case NoAlign:
        m_oPainter.drawText(QPoint(r.x,r.y),text.c_str());
        break;
      case Centered:
        m_oPainter.drawText(QRect(r.x,r.y,r.width,r.height),Qt::AlignCenter,text.c_str());
        break;
      case Justify:
        m_oPainter.drawText(QRect(r.x,r.y,r.width,r.height),Qt::AlignJustify,text.c_str());
        break;
    }

  }

  // }}}
  
  void QtPaintEngine::bci(int brightness, int contrast, int intensity){
    // {{{ open
    (void)brightness; (void)contrast; (void)intensity;
  }

  // }}}

  void QtPaintEngine::bciAuto(){
    // {{{ open
  
  }

  // }}}

  void QtPaintEngine::getColor(int *piColor){
    // {{{ open
    QColor c = m_oPainter.pen().color();
    piColor[0] = c.red();
    piColor[1] = c.green();
    piColor[2] = c.blue();
    piColor[3] = c.alpha();
  }

  // }}}
  
  void QtPaintEngine::getFill(int *piColor){
    // {{{ open
    QColor c = m_oPainter.brush().color();
    piColor[0] = c.red();
    piColor[1] = c.green();
    piColor[2] = c.blue();
    piColor[3] = c.alpha();
  }

  // }}}

}
