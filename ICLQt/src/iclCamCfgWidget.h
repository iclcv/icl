#ifndef CAM_CFG_WIDGET_H
#define CAM_CFG_WIDGET_H

#include <string>
#include <vector>
#include <QString>
#include <QWidget>
#include <iclUnicapDevice.h>
#include <iclTypes.h>
#include <QMutex>

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


  class CamCfgWidget : public QWidget{
    Q_OBJECT
    public:
    CamCfgWidget();
    
    public slots:
    void deviceChanged(const QString &text);
    void deviceChanged(int index);
    void formatChanged(const QString &text);
    void sizeChanged(const QString &text);

    void visImageParamChanged(int width, int height, int d, int fmt);

    void propertySliderChanged(const QString &id, double value);
    void propertyComboBoxChanged(const QString &text);
    void propertyButtonClicked(const QString &text);
    
    void startStopCapture(bool on);
    void updateImage();
    
    void createGrabber(const QString &id);
    
    private:
    void updateSizeCombo();
    void updateFormatCombo();
    
    void fillLayout(QLayout *l, Grabber *dev);

    QVBoxLayout *m_poVTopLevelLayout;
    QHBoxLayout *m_poTopLevelLayout;
    ICLWidget *m_poICLWidget;
    QWidget *m_poHBoxWidget;
    QWidget *m_poCenterPanel, *m_poRightPanel;
    QVBoxLayout *m_poCenterPanelLayout, *m_poRightPanelLayout;
    QScrollArea *m_poPropertyScrollArea;
    
    QPushButton *m_poCaptureButton;
    QLabel *m_poFpsLabel;
    QWidget *m_poGrabButtonAndFpsLabelWidget;
    QHBoxLayout *m_poGrabButtonAndFpsLabelLayout;

    QTabWidget *m_poTabWidget;
    
    QComboBox *m_poDeviceCombo;
    QComboBox *m_poFormatCombo;
    QComboBox *m_poSizeCombo;

    QTimer *m_poTimer;
    QMutex m_oGrabberMutex;
    
    
    Grabber *m_poGrabber;
    
    // UnicapDevice m_oUnicapDevice;
    std::vector<UnicapDevice> m_vecDeviceList;
    std::vector<int> m_vecPWCDeviceList;
    
    ImgParamWidget *m_poImgParamWidget;
    
    bool m_bDisableSlots;
    bool m_bCapturing;
  
    Size m_oVideoSize;
    format m_eVideoFormat;
    icl::depth m_eVideoDepth;
  
  };
}

#endif
  
