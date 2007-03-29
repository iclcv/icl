#include "iclCamCfgWidget.h"
#include "iclStackTimer.h"

#include <iclPWCGrabber.h>
#include <iclUnicapGrabber.h>
#include <iclWidget.h>
#include <iclDoubleSlider.h>

#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QSizePolicy>
#include <QTimer>
#include <QPushButton>
#include <QGroupBox>
using namespace icl;
using namespace std;

namespace icl{
  
  struct BorderBox : public QWidget{
    BorderBox(const QString &label, QWidget *content, QWidget *parent) : 
      QWidget(parent), m_poContent(content){
      m_poGroupBox = new QGroupBox(label,this);
      m_poLayout = new QVBoxLayout;
      m_poLayout->setMargin(3);
      m_poLayout->addWidget(content);
      m_poGroupBox->setLayout(m_poLayout);
      m_poGroupBox->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
      parent->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    }
    
    QWidget *content() { return m_poContent; }
  private:
    QGroupBox *m_poGroupBox;
    QVBoxLayout *m_poLayout;
    QWidget *m_poContent;
    
  };
  
    



  string sizeToStr(const Size &size){
    // {{{ open

    static char buf[100];
    sprintf(buf,"%dx%d",size.width,size.height);
    return buf;
  }

  // }}}
  Size strToSize(const string &sIn){
    // {{{ open

    QString s = sIn.c_str();
    return Size(s.section('x',0,0).toInt(), s.section('x',1,1).toInt());
  }

  // }}}
  
  CamCfgWidget::CamCfgWidget() : m_bDisableSlots(false), m_bCapturing(false){
    // TOP LEVEL
    m_poTopLevelLayout = new QHBoxLayout(this);
    m_poICLWidget = new ICLWidget(this);
    m_poTopLevelLayout->addWidget(m_poICLWidget);
    
    /// THREE PANELS
    m_poCenterPanel = new QWidget(this);
    m_poRightPanel = new QWidget(this);
    m_poCenterPanelLayout = new QVBoxLayout(m_poCenterPanel);
    m_poRightPanelLayout = new QVBoxLayout(m_poRightPanel);
    m_poCenterPanel->setLayout(m_poCenterPanelLayout);
    m_poRightPanel->setLayout(m_poRightPanelLayout);
    m_poTopLevelLayout->addWidget(m_poCenterPanel);

   
    /// CENTER WIDGETS
    m_poDeviceCombo = new QComboBox(this);
    m_poCenterPanelLayout->addWidget(new BorderBox("device",m_poDeviceCombo,this));
    
    m_poFormatCombo = new QComboBox(this);
    m_poCenterPanelLayout->addWidget(new BorderBox("format",m_poFormatCombo,this));
    
    m_poSizeCombo = new QComboBox(this);
    m_poCenterPanelLayout->addWidget(new BorderBox("size",m_poSizeCombo,this));

    m_poCaptureButton = new QPushButton("capture!",this);
    m_poCaptureButton->setCheckable(true);
    m_poCenterPanelLayout->addWidget(m_poCaptureButton);

    connect(m_poDeviceCombo,SIGNAL(currentIndexChanged(QString)), SLOT(deviceChanged(QString)));
    connect(m_poFormatCombo,SIGNAL(currentIndexChanged(QString)), SLOT(formatChanged(QString)));
    connect(m_poSizeCombo,SIGNAL(currentIndexChanged(QString)), SLOT(sizeChanged(QString)));
    connect(m_poCaptureButton,SIGNAL(toggled(bool)),this,SLOT(startStopCapture(bool)));

    updateDeviceCombo();

    /// RIGHT WIDGETS
    m_poPropertyScrollArea= new QScrollArea(this);
    m_poPropertyScrollArea->setWidget(m_poRightPanel);
    m_poPropertyScrollArea->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    m_poTopLevelLayout->addWidget(m_poPropertyScrollArea);
    
    
    /// FINISHING : FINAL LAYOUTING
    setLayout(m_poTopLevelLayout);
    setGeometry(50,50,1100,400);
    setWindowTitle("ICL Camera Configuration Tool");
    
    m_poTimer = new QTimer(this);
    m_poGrabber = 0;
    connect(m_poTimer,SIGNAL(timeout()),this,SLOT(updateImage()));
    show();
  }
  
  
  void CamCfgWidget::deviceChanged(const QString &text){
    if(m_bDisableSlots) return;
    bool wasCapturing = m_bCapturing;
    if(wasCapturing){
      startStopCapture(false);
    }
    vector<UnicapDevice> devList = UnicapGrabber::getDeviceList();
    bool found = false;
    for(unsigned int i=0;i<devList.size();i++){
      if(devList[i].getID() == text.toLatin1().data()){
        m_oUnicapDevice = devList[i];
        found = true;
        break;
      }
    }
    if(!found){
      ERROR_LOG("failed to switch to unknown device \""<<text.toLatin1().data()<<"\"");
    }
        
    if(wasCapturing){ 
      startStopCapture(true);
    }
    updateFormatCombo();
  }
  void CamCfgWidget::formatChanged(const QString &text){
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    if(m_bDisableSlots) return;
    bool wasCapturing = m_bCapturing;
    if(wasCapturing){
      startStopCapture(false);
    }
    getCurrentDevice().setFormatID(text.toLatin1().data());
    
    if(wasCapturing){ 
      startStopCapture(true);
    }
  }
  void CamCfgWidget::sizeChanged(const QString &text){
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    if(m_bDisableSlots) return;
    bool wasCapturing = m_bCapturing;
    if(wasCapturing){
      startStopCapture(false);
    }

    getCurrentDevice().setFormatSize(strToSize(text.toLatin1().data()));
    
    if(wasCapturing){ 
      startStopCapture(true);
    }
  }
  void CamCfgWidget::propertySliderChanged(const QString &id, double value){
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    UnicapProperty p = getCurrentDevice().getProperty(id.toLatin1().data());
    if(p.isValid()){
      p.setValue(value);
      getCurrentDevice().setProperty(p);
    }else{
      WARNING_LOG("noting known about property \""<< id.toLatin1().data() << "\"\n");
    }
  }
  void CamCfgWidget::propertyComboBoxChanged(const QString &text){
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    QString first = text.section(']',0,0);
    QString sec = text.section(']',1,1);
    string propName = first.toLatin1().data()+1;
    string propValue = sec.toLatin1().data()+1;
    UnicapProperty p = getCurrentDevice().getProperty(propName);
    if(p.isValid()){
      switch(p.getType()){
        case UnicapProperty::valueList:
          p.setValue(atof(propValue.c_str()));
          break;
        case UnicapProperty::menu:
          p.setMenuItem(propValue);
          break;
        default:
          ERROR_LOG("setting up this property type is not yet implemented !");
      }
      getCurrentDevice().setProperty(p);
    }else{
      WARNING_LOG("noting known about property \""<< propName << "\"\n");
    }
  }
  void CamCfgWidget::startStopCapture(bool on){
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    if(on){
      m_bCapturing = true;
      if(m_poGrabber) delete m_poGrabber;
      m_poGrabber = new UnicapGrabber(getCurrentDevice());
      m_poTimer->start(40);
    }else{
      m_bCapturing = false;
      if(m_poGrabber) delete m_poGrabber;
      m_poGrabber = 0;
      m_poTimer->stop();
    }
  }

  void CamCfgWidget::updateImage(){
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    ICLASSERT_RETURN(m_poGrabber);
    m_poICLWidget->setImage(m_poGrabber->grab((ImgBase**)0));
    m_poICLWidget->update();
  }
    
  UnicapDevice CamCfgWidget::getCurrentDevice(){
    // {{{ open
    return m_oUnicapDevice;
  }

  // }}}
  
  void CamCfgWidget::updateSizeCombo(){
    // {{{ open
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    m_bDisableSlots = true;
    while(m_poSizeCombo->count()){
      m_poSizeCombo->removeItem(0);
    }
    UnicapDevice dev = getCurrentDevice();
    if(dev.isValid()){
      UnicapFormat fmt = dev.getCurrentUnicapFormat();
      vector<Size> v = fmt.getPossibleSizes();
      if(v.size()){
        int currSizeIdx = -1;
        for(unsigned int i=0;i<v.size();i++){
          m_poSizeCombo->addItem(sizeToStr(v[i]).c_str());
          if(v[i] == dev.getCurrentSize()){
            currSizeIdx = i;
          }
        }
        m_poSizeCombo->setCurrentIndex(currSizeIdx);
      }else{
        m_poSizeCombo->addItem(sizeToStr(dev.getCurrentSize()).c_str());
      }
    }
    m_bDisableSlots = false;
  }

  // }}}
  
  void CamCfgWidget::updateFormatCombo(){
    // {{{ open
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    m_bDisableSlots = true;
    while(m_poFormatCombo->count()){
      m_poFormatCombo->removeItem(0);
    }
    UnicapDevice dev(m_poDeviceCombo->currentIndex());

    if(dev.isValid()){
      UnicapFormat currf = dev.getCurrentUnicapFormat();
      
      vector<UnicapFormat> fmts = dev.getFormats();
      int currIdx = -1;
      for(unsigned int i=0;i<fmts.size();i++){
        m_poFormatCombo->addItem(fmts[i].getID().c_str());
        if(fmts[i].getID() == currf.getID()){
          currIdx = i;
        }
      }
      if(currIdx != -1){
        m_poFormatCombo->setCurrentIndex(currIdx);
      }
    }
    m_bDisableSlots = false;
    updateSizeCombo();

  }

  // }}}
  
  void CamCfgWidget::updateDeviceCombo(){
    // {{{ open
    m_bDisableSlots = true;
    while(m_poDeviceCombo->count()){
      m_poDeviceCombo->removeItem(0);
    }
    vector<UnicapDevice> deviceList = UnicapGrabber::getDeviceList();
    for(unsigned int j=0;j<deviceList.size();j++) m_poDeviceCombo->addItem(deviceList[j].getID().c_str());
    m_bDisableSlots = false;
    
    if(deviceList.size()){
      deviceChanged(deviceList[0].getID().c_str());
    }else{
      ERROR_LOG("no supported devices were found!");
    }
  }

  // }}}

  void CamCfgWidget::updatePropertyPanel(){
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    m_bDisableSlots = true;
    vector<UnicapProperty> vec = getCurrentDevice().getProperties();
    for(unsigned int i=0;i<vec.size();i++){
      switch(vec[i].getType()){
        case UnicapProperty::range:{
          DoubleSlider *ds = new DoubleSlider(this,vec[i].getID().c_str());
          ds->setMinDouble(vec[i].getRange().minVal);
          ds->setMaxDouble(vec[i].getRange().maxVal);
          ds->setDoubleValue(vec[i].getValue());
          m_vecPropertySliders.push_back(ds);
          BorderBox *poBorderBox = new BorderBox(vec[i].getID().c_str(),ds,this);
          m_poRightPanelLayout->addWidget(poBorderBox);
          m_vecPropertyBorderBoxes.push_back(poBorderBox);
          //QLabel *label = new QLabel(vec[i].getID().c_str(),this);
          //m_vecPropertyLabels.push_back(label);
          //m_poRightPanelLayout->addWidget(label);
          //m_poRightPanelLayout->addWidget(ds);
          connect(ds,SIGNAL(doubleValueChanged(const QString&,double)),this,SLOT(propertySliderChanged(const QString&,double)));
          break;
        }
        case UnicapProperty::valueList:{
          QLabel *label = new QLabel(vec[i].getID().c_str(),this);
          //          m_vecPropertyLabels.push_back(label);
          m_poRightPanelLayout->addWidget(label);
          
          QString propName = QString("[")+vec[i].getID().c_str()+"]";
          QComboBox *cb = new QComboBox(this);
          vector<double> vals = vec[i].getValueList();
          int iCurrIdx = -1;
          for(unsigned int j=0;j<vals.size();j++){
            if(vals[j] == vec[i].getValue()){
              iCurrIdx = i;
            }
            cb->addItem(propName+" "+QString::number(vals[j]));
          }
          if(iCurrIdx != -1){
            cb->setCurrentIndex(iCurrIdx);
          }
          connect(cb,SIGNAL(currentIndexChanged(QString)),this,SLOT(propertyComboBoxChanged(QString)));
          m_poRightPanelLayout->addWidget(cb);
          break;
        
        }
        case UnicapProperty::menu:{
          QLabel *label = new QLabel(vec[i].getID().c_str(),this);
          //  m_vecPropertyLabels.push_back(label);
          m_poRightPanelLayout->addWidget(label);
          
          QString propName = QString("[")+vec[i].getID().c_str()+"]";
          QComboBox *cb = new QComboBox(this);
          vector<string> men = vec[i].getMenu();
          int iCurrIdx = -1;
          for(unsigned int j=0;j<men.size();j++){
            if(men[j] == vec[i].getMenuItem()){
              iCurrIdx = i;
            }
            cb->addItem(propName+" "+men[j].c_str());
          }
          if(iCurrIdx != -1){
            cb->setCurrentIndex(iCurrIdx);
          }
          connect(cb,SIGNAL(currentIndexChanged(QString)),this,SLOT(propertyComboBoxChanged(QString)));
          m_poRightPanelLayout->addWidget(cb);
          break;
        }
        default: // not yet supported via gui!
          break;
      }
    }    
    m_bDisableSlots = false;
  }
}

