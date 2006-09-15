#ifndef ICLWIDGET_H
#define ICLWIDGET_H

// comment out only if No acceleration is available
// fall back is not yet implemented
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
#include "GLPaintEngine.h"
#include "MouseInteractionInfo.h"
#include "MouseInteractionReceiver.h"

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
    Q_OBJECT
    public slots:
    void setImage(ImgI *poImage);
    
    signals:
    void mouseEvent(MouseInteractionInfo *info);

    public:
    enum fitmode   { fmFit = 0, 
                     fmHoldAR = 1,
                     fmNoScale = 2    };
    
    enum rangemode { rmOn = 1 ,  /**< range settings of the sliders are used */ 
                     rmOff = 2 , /**< no range adjustment is used */
                     rmAuto };   /**< automatic range adjustment */
                    
    
    
    
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
    
    /// drawing using openGL
    virtual void paintGL();
    /// additiona custom drawings (between image and osd)
    virtual void customPaintEvent(GLPaintEngine *e){(void)e;}
    /// final drawing of the osd
    void drawOSD(GLPaintEngine *e);
  
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
    std::vector<string> getImageInfo();
    
    protected:
    /// sets up all 3 gl channels to given bias and scale
    Rect computeImageRect(Size oImageSize, Size oWidgetSize, fitmode eFitMode);
    void drawImage(GLPaintEngine *e);
       
    
    private:

    // help function for creating the current mouse interactio info
    MouseInteractionInfo *updateMouseInfo(MouseInteractionInfo::Type type);
    MouseInteractionInfo m_oMouseInfo;
   
    Options op;
    QMutex m_oMutex, m_oOSDMutex;

    ImgI *m_poImage;
    
    OSDWidget *m_poOSD;
    OSDWidget *m_poCurrOSD;
    OSDButton *m_poShowOSD;

    int aiDown[3];
    int m_iMouseX, m_iMouseY;

    static const int SHOW_OSD_ID = 123456;

  };
}
#endif
