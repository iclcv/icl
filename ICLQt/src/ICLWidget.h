#ifndef ICLWIDGET_H
#define ICLWIDGET_H

// comment out only if No acceleration is available
#define USE_OPENGL_ACCELERATION


#ifdef USE_OPENGL_ACCELERATION
#include <QGLWidget>
#else
#include <QImage>
#include <QVector>
#include <QWidget>
#endif
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
    void childChanged(int id, void *val);
    
    void setImage(ImgI *poImage);
    
    /// openGL-drawing
    virtual void paintEvent(QPaintEvent *poEvent);
  
     /// returns the current fitmode
    fitmode getFitMode(){return op.fm;}

    std::vector<QString> getImageInfo();

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

    protected:
#ifdef USE_OPENGL_ACCELERATION
    /// openGL-initialisation 
    virtual void initializeGL();

    virtual void paintGL();

    /// -1 -> set up r,g and b channel separately
    void setBiasAndScaleForChannel(int iDepth, int iChannel=-1);

    /// sets up all 3 gl channels to given bias and scale
    void setBiasAndScale(float fBiasRGB, float fScaleRGB);

    /// sets up the gl raster pos and scale-variables depending on current oImageRect
    void setupPixelEngine(int iImageW, int iImageH, fitmode eFitMode);

    void setPackAllignment(int iImageW, icl::depth eDepth);

    void pushCurrentState();
    
    void popCurrentState();
    
    void restoreQPainterInitialization();

    void drawImgGL(ImgI *poImage, fitmode eFitMode, int iChannel);

#else
    
    void convertImg(ImgI* src,QImage &dst);
    
#endif    

    Rect computeImageRect(Size oImageSize, Size oWidgetSize, fitmode eFitMode);

    void bufferImg(ImgI* poSrc);

    void paint2D(QPainter *poPainter);

    void drawStr(QPainter *poPainter,QString s, QRect r, int iFontSize = 18, QColor c=QColor(255,255,255),QFont f=QFont("Arial",18));
    void drawRect(QPainter *poPainter,QRect r,QColor cBorder, QColor cFill);

   
    
    private:
    Options op;
    QMutex m_oMutex;
    ImgI *m_poImage;
#ifndef USE_OPENGL_ACCELERATION
    QImage m_oQImage;
    QVector<QRgb> m_oColorTable;
#endif  
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
