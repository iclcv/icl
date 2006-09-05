#ifndef ICLWIDGET_H
#define ICLWIDGET_H

// comment out only if No acceleration is available
#define USE_OPENGL_ACCELERATION


#ifdef USE_OPENGL_ACCELERATION
#include <QGLWidget>
#endif
#include <QImage>
#include <QVector>
#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QFont>
#include <QRect>
#include <QColor>
#include <QMutex>

#include "Converter.h"

using namespace icl;

namespace icl{
  class OSDWidget;
  class OSD;
  class OSDButton;
  class icl::ImgI;

#ifdef USE_OPENGL_ACCELERATION
  typedef QGLWidget ParentWidgetClass;
#else
  typedef QWidget ParentWidgetClass;
#endif  
  
  
  /// Intern used class for openGL-based image visualisation componets, embedded into an ICLGuiModule
  class ICLWidget : public ParentWidgetClass{
    public:
    enum fitmode   { fmFit = 0, 
                     fmHoldAR = 1,
                     fmNoScale = 2    };
    
    enum rangemode { rmOn = 1 , 
                     rmOff = 2        };
    
    /// Constructor
    ICLWidget(QWidget *poParent);
    
    /// Mouse-Event handling
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void enterEvent(QEvent *e);
    virtual void leaveEvent(QEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void childChanged(int id, void *val);
    
    void setImage(ImgI *poImage);
   
 
    /// drawing
    virtual void paintEvent(QPaintEvent *poEvent);
    /// additiona custom drawings (between image and osd)
    virtual void customPaintEvent(QPainter *poPainter){}
    /// final drawing of the osd
    void drawOSD(QPainter *poPainter);
  
  

    struct Options{
      fitmode fm;
      rangemode rm;
      bool on;
      int c;
      int brightness;
      int contrast;
      int intensity;
    };

    // calls update
    void up();
    
    // returns the width of the widget
    int w();

    // returns the height of the widget
    int h();

    // sets the current fitmode
    void setFitMode(fitmode fm){ op.fm = fm; }
    // sets the current rangemode
    void setRangeMode(rangemode rm){ op.rm = rm; }
    /// returns the current image size of the widget size if the image is null
    Size getImageSize();
    /// returns the current image rect
    Rect getImageRect();
    fitmode getFitMode(){return op.fm;}
    std::vector<QString> getImageInfo();
    
    protected:
    /// sets up all 3 gl channels to given bias and scale
    void setBiasAndScale(float fBiasRGB, float fScaleRGB);
    Rect computeImageRect(Size oImageSize, Size oWidgetSize, fitmode eFitMode);

    void drawImage(QPainter *poPainter);
    void drawStr(QPainter *poPainter,QString s, QRect r, int iFontSize = 18, QColor c=QColor(255,255,255),QFont f=QFont("Arial",18));
    void drawRect(QPainter *poPainter,QRect r,QColor cBorder, QColor cFill);

    
    private:
    Options op;
    QMutex m_oMutex, m_oOSDMutex;
    Img8u *m_poImage;
    QImage m_oQImage;
    QVector<QRgb> m_oColorTable;
    Converter m_oConverter;

    OSDWidget *m_poOSD;
    OSDWidget *m_poCurrOSD;
    OSDButton *m_poShowOSD;
    int aiDown[3];
    int m_iMouseX, m_iMouseY;

    static const int SHOW_OSD_ID = 123456;

  };
}
#endif
