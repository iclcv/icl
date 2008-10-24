#ifndef CAM_CFG_WIDGET_H
#define CAM_CFG_WIDGET_H

#include <string>
#include <vector>
#include <QString>
#include <QWidget>
#include <iclUnicapDevice.h>
#include <iclDCDevice.h>
#include <iclTypes.h>
#include <QMutex>
#include <QSplitter>
#include <iclCompabilityLabel.h>
#include <iclFPSEstimator.h>


/** \cond **/  
class QHBoxLayout;
class QComboBox;
class QVBoxLayout;
class QLabel;
class QScrollArea;
class QPushButton;
class QTimer;
class QGroupBox;
class QTabWidget;
/** \endcond **/  

namespace icl{
  /** \cond **/
  class ICLWidget;
  class Grabber;
  class DoubleSlider;
  class BorderBox;
  class ImgParamWidget;
  /** \endcond **/  

  /// Complex widget class, which is used in the Camera configuration tool camcfg \ingroup UNCOMMON
  class CamCfgWidget : public QSplitter{
    Q_OBJECT
    public:
    
    struct CreationFlags{
    CreationFlags(int isoMBits=400,
                  bool resetDCBus=false, 
                  bool omitDoubledDCFrames=true,
                  bool disableUnicap=false,
                  bool disableDC=false,
                  bool disablePWC=false):
        isoMBits(isoMBits),resetDCBus(resetDCBus),
        omitDoubledDCFrames(omitDoubledDCFrames),
        disableUnicap(disableUnicap),disableDC(disableDC),
        disablePWC(disablePWC){}
      
      int isoMBits;
      bool resetDCBus;
      bool omitDoubledDCFrames;
      bool disableUnicap;
      bool disableDC;
      bool disablePWC;
    };
    
    /// Constructor
    CamCfgWidget(const CreationFlags &flags,QWidget *parent=0);
    
    /// Destructor
    ~CamCfgWidget();

    /// getter of the current image for "abusing" the CamCfgWidget as a Grabber
    virtual const ImgBase *getCurrentImage();
    
    public slots:
    /// slot for processing events on the device combo box
    void deviceChanged(const QString &text);

    /// slot for processing events on the device combo box
    void deviceChanged(int index);

    /// slot for processing events on the format combo box
    void formatChanged(const QString &text);
    
    /// slot for processing events on the size combo box
    void sizeChanged(const QString &text);

    /// slot for processing events on the image params widget
    void visImageParamChanged(int width, int height, int d, int fmt);

    /// slot for processing events on property sliders (ranged values)
    void propertySliderChanged(const QString &id, double value);

    /// slot for processing events on property combo-boxes (value lists)
    void propertyComboBoxChanged(const QString &text);
    
    /// slot for processing events on property buttons (events)
    void propertyButtonClicked(const QString &text);
    
    /// slot for processing events on the capture button
    void startStopCapture(bool on);

    /// slot that is called to update the currently shown image
    void updateImage();
    
    /// slot to create a new grabber with given id
    void createGrabber(const QString &id);
    
    private:
    /// called when a new grabber is selcted
    void updateSizeCombo();
    
    /// called when a new grabber is selcted
    void updateFormatCombo();
    
    /// called when a new grabber is selcted
    void fillLayout(QLayout *l, Grabber *dev);

    //QVBoxLayout *m_poVTopLevelLayout;    //!< the top level vertical layout
    //QHBoxLayout *m_poTopLevelLayout;     //!< the top level horizontal layout
    
    QSplitter *m_poBottomSplitter;     //!< the vertical lower level horizonntal splitter

    ICLWidget *m_poICLWidget;            //!< image visualizing widget
    
    //QWidget *m_poHBoxWidget;             //!< top level hbox widget
    
    QWidget *m_poCenterPanel;            //!< top level center widget
    QWidget *m_poRightPanel;             //!< top level right widget
    QVBoxLayout *m_poCenterPanelLayout;  //!< internaly layout
    QVBoxLayout *m_poRightPanelLayout;   //!< internaly layout
    QScrollArea *m_poPropertyScrollArea; //!< internaly scroll-area (for the list of properties)
    
    QPushButton *m_poCaptureButton;      //!< button for start/stop capturing (togglebutton)
    CompabilityLabel *m_poFpsLabel;      //!< lable that shows the current fps (inactive)
    QWidget *m_poGrabButtonAndFpsLabelWidget;       //!< container widget
    QHBoxLayout *m_poGrabButtonAndFpsLabelLayout;   //!< container layout

    QTabWidget *m_poTabWidget;           //!< property tab for each camera found
    
    QComboBox *m_poDeviceCombo;          //!< provides a list of all available devices (ieee,pwc and unicap)
    QComboBox *m_poFormatCombo;          //!< provides all formats available on the currently selcted device
    QComboBox *m_poSizeCombo;            //!< provides all sizes available on the currently selcted device

    QTimer *m_poTimer;                   //!< internally used timer (grabbing thread!)
    QMutex m_oGrabberMutex;              //!< mutex to protect the grabbed images
    
    
    Grabber *m_poGrabber;                //!< current grabber instance
    
    // UnicapDevice m_oUnicapDevice;
    std::vector<UnicapDevice> m_vecDeviceList;   //!< unicap device list
    std::vector<DCDevice> m_vecDCDeviceList;     //!< DCDevice list
    std::vector<int> m_vecPWCDeviceList;         //!< PWCDevice list
    
    ImgParamWidget *m_poImgParamWidget;   //!< widget to ajust image params ( grabbers desired params)
    
    bool m_bDisableSlots;                 //!< internal flag for temporarily disable all slots (to disable endless recursions)
    bool m_bCapturing;                    //!< flag that indicates wheter the current grabber thread runs or not
  
    Size m_oVideoSize;                    //!< current video size
    format m_eVideoFormat;                //!< current video format
    icl::depth m_eVideoDepth;             //!< current video depth

    int m_isoMBits;                       //!< mbit count for dc devices

    bool m_omitDoubledDCFrames;           //!< if set, DCGrabbers are set up to omit doubled frames

    FPSEstimator m_oFPSE;                 //!< used to estimate the grabbers fps
  
  };
}

#endif
  
