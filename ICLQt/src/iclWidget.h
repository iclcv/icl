#ifndef ICLWIDGET_H
#define ICLWIDGET_H

#include <QGLWidget>
#include <iclImgBase.h>
#include <iclTypes.h>
#include <iclImageStatistics.h>
#include <iclMouseHandler.h>
#include "iclWidgetCaptureMode.h"
#include <iclGUI.h>

namespace icl{

  /** \cond */
  class PaintEngine;
  class OSDGLButton;
  /** \endcond */
  
 
  /// Class for openGL-based image visualization components \ingroup COMMON
  /** The ICLWidget class provide basic abilities for displaying ICL images (ImgBase) on embedded 
      Qt GUI components. Its is fitted out with a responsive OpenGL-Overlay On-Screen-Display (OSD),
      which can be used to adapt some ICLWidget specific settings, and which is partitioned into several
      sub menus:
      - <b>adjust</b> here one can define image adjustment parameters via sliders, like brightness and
        contrast. In addition, there are three different modes: <em>off</em> (no adjustments are 
        preformed), <em>manual</em> (slider values are exploited) and <em>auto</em> (brightness 
        and intensity are adapted automatically to exploit the full image range). <b>Note:</b> This time,
        no intensity adaptions are implemented at all.
      - <b>fitmode</b> Determines how the image is fit into the widget geometry (see enum 
        ICLWidget::fitmode) 
      - <b>channel selection</b> A slider can be used to select a single image channel that should be 
        shown as gray image 
      - <b>capture</b> Yet, just a single button is available to save the currently shown image into the
        local directory 
      - <b>info</b> shows information about the current image
      - <b>menu</b> settings for the menu itself, like the alpha value or the color (very useful!)
      
      The following code (also available in <em>ICLQt/examples/camviewer_lite.cpp</em> demonstrates how
      simple an ICLWidget can be used to create a simple USB Webcam viewer:
      \code
      
\#include <iclWidget.h>
\#include <iclDrawWidget.h>
\#include <iclPWCGrabber.h>
\#include <QApplication>
\#include <QThread>
\#include <iclTestImages.h>
using namespace icl;
using namespace std;


// we need a working thread, which allows us to grab images
// from the webcam asynchronously to Qts event loop

class MyThread : public QThread{               
public:
  MyThread(){
    widget = new ICLWidget(0);                // create the widget
    widget->setGeometry(200,200,640,480);     // set up its geometry
    widget->show();                           // show it
    start();                                  // start the working loop (this will call the
  }                                           //           run() function in an own thread)   
  ~MyThread(){
    exit();                                   // this will destroy the working thread
    msleep(250);                              // wait till it is destroyed   
    delete widget;
  }
  
  virtual void run(){
    PWCGrabber g(Size(320,240));              // create a grabber instance
    while(1){                                 // enter the working loop
      widget->setImage(g.grab());             // grab a new image from the grabber an give it to the widget
      widget->update();                       // force qt to update the widgets user interface
    }
  }
  private:
  ICLWidget *widget;                          // internal widget object
};


int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);           // create a QApplication
  MyThread x;                             // create the widget and thread
  return a.exec();                        // start the QAppliations event loop
}
      
      \endcode

      When other things, like annotations should be drawn to the widget, you can either
      create a new class that inherits the ICLWidget class, and reimplement the function:
      \code
      virtual void customPaintEvent(PaintEngine *e);
      \endcode
      or, you can use the extended ICLDrawWidget class, which enables you to give 2D draw
      commands in image coordinates. (The last method is more recommended, as it should be
      exactly what you want!!!)
      @see ICLDrawWidget
  */
  class ICLWidget : public QGLWidget{
    Q_OBJECT
    public:
    /// just used internally 
    friend class OSDGLButton;

    /// member function callback type for void foo(void) members
    typedef void (ICLWidget::*VoidCallback)();

    /// member function callback type for void foo(bool) members
    typedef void (ICLWidget::*BoolCallback)(bool);

    class Data;   
    class OutputBufferCapturer;
    
    /// determines how the image is fit into the widget geometry
    enum fitmode{ 
      fmNoScale=0,  /**< the image is not scaled it is centered to the image rect */
      fmHoldAR=1,   /**< the image is scaled to fit into the image rect, but its aspect ratio is hold */
      fmFit=2,      /**< the image is fit into the frame ( this may change the images aspect ratio)*/
      fmZoom=3      /**< new mode where an image rect can be specified in the gui ... */
    };
    
    /// determines intensity adaption mode 
    enum rangemode { 
      rmOn = 1 ,  /**< range settings of the sliders are used */ 
      rmOff = 2 , /**< no range adjustment is used */
      rmAuto      /**< automatic range adjustment */ 
    };   
    
    /// creates a new ICLWidget within the parent widget
    ICLWidget(QWidget *parent=0);
    
    /// destructor
    virtual ~ICLWidget();

    /// GLContext initialization
    virtual void initializeGL();
    
    /// called by resizeEvent to adapt the current GL-Viewport
    virtual void resizeGL(int w, int h);
    
    /// draw function
    virtual void paintGL();

    /// drawing function for NO-GL fallback
    virtual void paintEvent(QPaintEvent *e);
    
    /// this function can be overwritten do draw additional misc using the given PaintEngine
    virtual void customPaintEvent(PaintEngine *e);

    virtual void setVisible(bool visible);

    /// sets the current fitmode
    void setFitMode(fitmode fm);
    
    /// sets the current rangemode
    void setRangeMode(rangemode rm);
    
    /// set up current brightness, contrast and intensity adaption values
    void setBCI(int brightness, int contrast, int intensity);

    
    /// returns the widgets size as icl::Size
    Size getSize() { return Size(width(),height()); }
    
    /// returns the current images size
    Size getImageSize(bool fromGUIThread=false);

    /// returns the rect, that is currently used to draw the image into
    Rect getImageRect(bool fromGUIThread=false);

    /// returns current fit-mode
    fitmode getFitMode();
        
    /// returns current range mode
    rangemode getRangeMode();

    /// returns a list of image specification string (used by the OSD)
    std::vector<std::string> getImageInfo();

    
    /// adds a new mouse handler via signal-slot connection
    /** Ownership is not passed ! */
    void install(MouseHandler *h);

    /// deletes mouse handler connection 
    /** Ownership was not passed -> h is not deleted  */
    void uninstall(MouseHandler *h);

    /// registeres a simple callback 
    /** @param cb callback functor to use
        @param eventList comma-separated list of events. Supported types are:
               - all (for all events)
               - move (mouse if moved, no button pressed)
               - drag  (mouse is moved, at least one button is pressed)
               - press, (guess what)
               - release (button released)
               - enter  (mouse cursor enters the widget)
               - leave  (mouse cursor leaved the widget)
    */
    void registerCallback(GUI::CallbackPtr cb, const std::string &eventList="drag,press");

    /// removes all callbacks registered using registerCallback
    void removeCallbacks();

    /// this function should be called to update the widget asyncronously from a working thread
    void updateFromOtherThread();
    
    /// overloaded event function processing special thread save update events
    virtual bool event(QEvent *event);
    
    /// returns current ImageStatistics struct (used by OSD)
    const ImageStatistics &getImageStatistics();

    void setMenuEnabled(bool enabled);
    
    void setImageInfoIndicatorEnabled(bool enabled);
    
    void setShowNoImageWarnings(bool showWarnings);

    /// Adds a new toggle-button to the OSD-button bar on the upper widget edge
    /** Special buttons can directly be attached to specific ICLWidget slots, 
        furthermore special button- clicks and toggle events are notified using
        the ICLWidget-signals specialButtonClicked and specialButtonToggled.
        
        @param id handle to reference the button lateron 
        
        @param untoggledIcon optional button icon(recommeded: use buttons from the
               ICLQt::IconFactor class)

        @param toggledIcon optional button icon (recommeded: use buttons from the
               ICLQt::IconFactor class)

        @param ICLWidgetSlot here you can direcly define a slot from ICLWidget
               class, the button is attached to. Note, if toggable is true,
               it has to be a foo(bool)-slot, otherwise you'll need a foo(void)
               slot.
    */
    void addSpecialToggleButton(const std::string &id, 
                                const ImgBase* untoggledIcon = 0, 
                                const ImgBase *toggledIcon = 0, 
                                bool initiallyToggled = 0, 
                                BoolCallback cb = 0);

    /// Adds a new toggle-button to the OSD-button bar on the upper widget edge
    /** @see addSpecialToggleButton */
    void addSpecialButton(const std::string &id, 
                          const ImgBase* icon = 0, 
                          VoidCallback = 0);

    
    /// removes special button with given ID
    void removeSpecialButton(const std::string &id);
    
    public slots:
    /// sets up the current image
    void setImage(const ImgBase *image);

    signals:
    /// invoked when any mouse interaction was performed
    void mouseEvent(const MouseEvent &event);

    
    void specialButtonClicked(const std::string &id);
    void specialButtonToggled(const std::string &id, bool down);


    public:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void enterEvent(QEvent *e);
    virtual void leaveEvent(QEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    
    
    public slots:
    void showHideMenu();
    void setMenuEmbedded(bool embedded);

    void bciModeChanged(int modeIdx);
    void brightnessChanged(int val);
    void contrastChanged(int val);
    void intensityChanged(int val);
    
    void scaleModeChanged(int modeIdx);
    void currentChannelChanged(int modeIdx);

    void captureCurrentImage();
    void captureCurrentFrameBuffer();
    
    void recordButtonToggled(bool checked);
    void pauseButtonToggled(bool checked);
    void stopButtonClicked();
    void skipFramesChanged(int frameSkip);
    void menuTabChanged(int index);
    void histoPanelParamChanged();
    
    void setEmbeddedZoomModeEnabled(bool enabled);
    
    void setLinInterpolationEnabled(bool enabled);
    
    private:
    /// internally used, grabs the current framebuffer as Img8u 
    const Img8u &grabFrameBufferICL();
    
    /// internal utility function
    std::string getImageCaptureFileName();
    
    /// internal utility function
    void updateInfoTab();

    /// just internally used
    void rebufferImageInternal();

    /// Internal data class (large, so it's hidden)
    Data *m_data;

    /// creates internal event instance
    const MouseEvent &createMouseEvent(MouseEventType type);

  };
  
}
#endif
