#ifndef QTPAINT_ENGINE_H
#define QTPAINT_ENGINE_H

#include <ICLQt/PaintEngine.h>
#include <QFont>
#include <QPainter>
#include <ICLQt/QImageConverter.h>

// forward declaration of the parent class 
class QWidget;

namespace icl{

  /** \cond */
  class ImgBase;
  /** \endcond */
  
  /// PaintEngine implementation using Qts QPainter \ingroup UNCOMMON
  class QtPaintEngine : public PaintEngine{
    public:
    /// Create a new QtPaintEngine which paints on a given QWidget
    QtPaintEngine(QWidget *widget);

    /// Destructor
    ~QtPaintEngine();
    
    virtual void color(int r, int g, int b, int a=255);
    virtual void fill(int r, int g, int b, int a=255);
    virtual void fontsize(int size);
    virtual void font(std::string name, 
                      int size = -1, 
                      PaintEngine::TextWeight weight = PaintEngine::Normal, 
                      PaintEngine::TextStyle style = PaintEngine::StyleNormal);

    virtual void line(const Point &a, const Point &b);
    virtual void point(const Point &p);
    virtual void image(const Rect &r,ImgBase *image, PaintEngine::AlignMode mode = PaintEngine::Justify, scalemode sm=interpolateNN);
    virtual void image(const Rect &r,const QImage &image, PaintEngine::AlignMode mode = PaintEngine::Justify, scalemode sm=interpolateNN);
    virtual void rect(const Rect &r);
    virtual void triangle(const Point &a, const Point &b, const Point &c);
    virtual void quad(const Point &a, const Point &b, const Point &c, const Point &d);
    virtual void ellipse(const Rect &r);
    virtual void text(const Rect &r, const std::string text, PaintEngine::AlignMode mode = PaintEngine::Centered);

    /// brightness-constrast intensity adjustment (for images only)
    virtual void bci(int brightness=0, int contrast=0, int intensity=0);
    virtual void bciAuto();
    
    virtual void getColor(int *piColor);
    virtual void getFill(int *piColor);

    virtual int getFontSize() const{
      return m_oFont.pixelSize();
    }


    protected:
      
    QWidget *m_poWidget;
    QPainter m_oPainter;
    
    QFont m_oFont;
    QImageConverter m_oQImageConverter;
    ImgBase *m_poScaledImage;
  };
}// namespace
#endif

