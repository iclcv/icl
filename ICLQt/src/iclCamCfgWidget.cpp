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
  
  // {{{ Utilities BorderBox, sizeToStr and strToSize

  struct BorderBox : public QGroupBox{
    // {{{ open

    BorderBox(const QString &label, QWidget *content, QWidget *parent) : 
      QGroupBox(label,parent), m_poContent(content){
      m_poLayout = new QVBoxLayout;
      m_poLayout->setMargin(3);
      m_poLayout->addWidget(content);
      setLayout(m_poLayout);
    }
    
    QWidget *content() { return m_poContent; }
  private:
    QVBoxLayout *m_poLayout;
    QWidget *m_poContent;
    
  };

  // }}}
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

  // }}}
  
  CamCfgWidget::CamCfgWidget() : m_bDisableSlots(false), m_bCapturing(false){
    // {{{ open

    // TOP LEVEL
    m_poTopLevelLayout = new QHBoxLayout(this);
    m_poICLWidget = new ICLWidget(this);
    m_poTopLevelLayout->addWidget(m_poICLWidget);
    
    /// THREE PANELS
    m_poCenterPanel = new QWidget(this);
    m_poTabWidget = new QTabWidget(this);
    
    m_poTabWidget->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    m_poCenterPanelLayout = new QVBoxLayout(m_poCenterPanel);
    m_poCenterPanel->setLayout(m_poCenterPanelLayout);
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
    connect(m_poDeviceCombo,SIGNAL(currentIndexChanged(int)), SLOT(deviceChanged(int)));
    connect(m_poFormatCombo,SIGNAL(currentIndexChanged(QString)), SLOT(formatChanged(QString)));
    connect(m_poSizeCombo,SIGNAL(currentIndexChanged(QString)), SLOT(sizeChanged(QString)));
    connect(m_poCaptureButton,SIGNAL(toggled(bool)),this,SLOT(startStopCapture(bool)));

    /// RIGHT WIDGETS
    m_vecDeviceList = UnicapGrabber::getDeviceList();
        

    for(unsigned int j=0;j<m_vecDeviceList.size();j++){
      QString name = m_vecDeviceList[j].getID().c_str();
      m_poDeviceCombo->addItem(name);
      QWidget *w = new QWidget(this);
      QVBoxLayout *l = new QVBoxLayout(w);
      QScrollArea *sa = new QScrollArea(this);
      fillLayout(l,m_vecDeviceList[j]);
      sa->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
      w->setLayout(l);
      sa->setWidget(w);      
      m_poTabWidget->addTab(sa,name);
      m_poTabWidget->setTabEnabled(j,false);
    }
    m_poTopLevelLayout->addWidget(m_poTabWidget);
    
    
    /// FINISHING : FINAL LAYOUTING
    setLayout(m_poTopLevelLayout);
    setGeometry(50,50,1100,400);
    setWindowTitle("ICL Camera Configuration Tool");
    
    m_poTimer = new QTimer(this);
    m_poGrabber = 0;
    connect(m_poTimer,SIGNAL(timeout()),this,SLOT(updateImage()));
    show();
  }

  // }}}
  
  void CamCfgWidget::deviceChanged(const QString &text){
    // {{{ open

    if(m_bDisableSlots) return;
    bool wasCapturing = m_bCapturing;
    if(wasCapturing){
      startStopCapture(false);
    }

    bool found = false;
    for(unsigned int i=0;i<m_vecDeviceList.size();i++){
      if(m_vecDeviceList[i].getID() == text.toLatin1().data()){
        m_oUnicapDevice = m_vecDeviceList[i];
        if(m_oUnicapDevice.getCurrentSize() == Size(-1,-1)){
          vector<UnicapFormat> formats = m_oUnicapDevice.getFormats();
          if(formats.size()){
            m_oUnicapDevice.setFormat(formats[0]);
          }
        }
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
    formatChanged(m_poFormatCombo->currentText());
  }

  // }}}
  
  void CamCfgWidget::deviceChanged(int index){
    // {{{ open
    m_poTabWidget->setCurrentIndex(index);
  }

  // }}}
  
  void CamCfgWidget::formatChanged(const QString &text){
    // {{{ open

    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    if(m_bDisableSlots) return;
    bool wasCapturing = m_bCapturing;
    if(wasCapturing){
      startStopCapture(false);
    }
    m_oUnicapDevice.setFormatID(text.toLatin1().data());
    
    if(wasCapturing){ 
      startStopCapture(true);
    }
  }

  // }}}
  void CamCfgWidget::sizeChanged(const QString &text){
    // {{{ open

    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    if(m_bDisableSlots) return;
    bool wasCapturing = m_bCapturing;
    if(wasCapturing){
      startStopCapture(false);
    }

    m_oUnicapDevice.setFormatSize(strToSize(text.toLatin1().data()));
    
    if(wasCapturing){ 
      startStopCapture(true);
    }
  }

  // }}}
  void CamCfgWidget::propertySliderChanged(const QString &id, double value){
    // {{{ open

    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    UnicapProperty p = m_oUnicapDevice.getProperty(id.toLatin1().data());
    if(p.isValid()){
      p.setValue(value);
      m_oUnicapDevice.setProperty(p);
    }else{
      WARNING_LOG("noting known about property \""<< id.toLatin1().data() << "\"\n");
    }
  }

  // }}}
  void CamCfgWidget::propertyComboBoxChanged(const QString &text){
    // {{{ open

    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    QString first = text.section(']',0,0);
    QString sec = text.section(']',1,1);
    string propName = first.toLatin1().data()+1;
    string propValue = sec.toLatin1().data()+1;
    UnicapProperty p = m_oUnicapDevice.getProperty(propName);
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
      m_oUnicapDevice.setProperty(p);
    }else{
      WARNING_LOG("noting known about property \""<< propName << "\"\n");
    }
  }
  // }}}

  void CamCfgWidget::startStopCapture(bool on){
    // {{{ open
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    if(on){
      m_bCapturing = true;
      if(m_poGrabber) delete m_poGrabber;
      m_poGrabber = new UnicapGrabber(m_oUnicapDevice);
      m_poTimer->start(40);
    }else{
      m_bCapturing = false;
      if(m_poGrabber) delete m_poGrabber;
      m_poGrabber = 0;
      m_poTimer->stop();
    }
  }

  // }}}


  void CamCfgWidget::updateImage(){
    // {{{ open

    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    ICLASSERT_RETURN(m_poGrabber);
    m_poICLWidget->setImage(m_poGrabber->grab((ImgBase**)0));
    m_poICLWidget->update();
  }

  // }}}
  void CamCfgWidget::updateSizeCombo(){
    // {{{ open
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    m_bDisableSlots = true;
    while(m_poSizeCombo->count()){
      m_poSizeCombo->removeItem(0);
    }

    QString currentFormatString = m_poFormatCombo->currentText();

    vector<UnicapFormat> formats = m_oUnicapDevice.getFormats();
    Size currSize = m_oUnicapDevice.getCurrentSize();
    for(unsigned int j=0;j<formats.size();j++){
      if(currentFormatString == formats[j].getID().c_str() ){
        vector<Size> sizes = formats[j].getPossibleSizes();
        int currSizeIdx = -1;
        if(!sizes.size()){
          m_poSizeCombo->addItem(sizeToStr(currSize).c_str());
          currSizeIdx = 0;
        }else{
          for(unsigned int i=0;i<sizes.size();i++){
            m_poSizeCombo->addItem(sizeToStr(sizes[i]).c_str());
            if(sizes[i] == currSize){
              currSizeIdx = i;
            }
          }
        }
        if(currSizeIdx != -1){
          m_poSizeCombo->setCurrentIndex(currSizeIdx);
        }
        break;
      }
    }
    m_bDisableSlots = false;
  }
    /*************************************************
        UnicapFormat fmt = dev.getCurrentUnicapFormat();
        vector<Size> v = fmt.getPossibleSizes();
        if(v.size()){
        int currSizeIdx = -1;
        
        for(unsigned int i=0;i<v.size();i++){
        printf("case 1: adding size %dx%d\n",v[i].width,v[i].height);
        m_poSizeCombo->addItem(sizeToStr(v[i]).c_str());
        if(v[i] == dev.getCurrentSize()){
        currSizeIdx = i;
        }
        }
        m_poSizeCombo->setCurrentIndex(currSizeIdx);
        }else{
        m_poSizeCombo->addItem(sizeToStr(dev.getCurrentSize()).c_str());
        }
     *****************************************************/
  // }}}
  void CamCfgWidget::updateFormatCombo(){
    // {{{ open
    ICLASSERT_RETURN(m_oUnicapDevice.isValid());
    m_bDisableSlots = true;
    while(m_poFormatCombo->count()){
      m_poFormatCombo->removeItem(0);
    }


    UnicapFormat currf = m_oUnicapDevice.getCurrentUnicapFormat();
    
    vector<UnicapFormat> fmts = m_oUnicapDevice.getFormats();
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
  
    m_bDisableSlots = false;
    updateSizeCombo();
    
  }

  // }}}
 


  void CamCfgWidget::fillLayout(QLayout *l, UnicapDevice &dev){
    // {{{ open

    ICLASSERT_RETURN(dev.isValid());
    
    vector<UnicapProperty> vec = dev.getProperties();
    QWidget *PARENT = 0;
    for(unsigned int i=0;i<vec.size();i++){
      switch(vec[i].getType()){
        case UnicapProperty::range:{
          DoubleSlider *ds = new DoubleSlider(PARENT,vec[i].getID().c_str());
          ds->setMinDouble(vec[i].getRange().minVal);
          ds->setMaxDouble(vec[i].getRange().maxVal);
          ds->setDoubleValue(vec[i].getValue());
          BorderBox *poBorderBox = new BorderBox(vec[i].getID().c_str(),ds,PARENT);
          l->addWidget(poBorderBox);
          connect(ds,SIGNAL(doubleValueChanged(const QString&,double)),this,SLOT(propertySliderChanged(const QString&,double)));
          break;
        }
        case UnicapProperty::valueList:{
          QString propName = QString("[")+vec[i].getID().c_str()+"]";
          QComboBox *cb = new QComboBox(PARENT);
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
          BorderBox *poBorderBox = new BorderBox(vec[i].getID().c_str(),cb,PARENT);
          l->addWidget(poBorderBox);
          break;
        
        }
        case UnicapProperty::menu:{
          QString propName = QString("[")+vec[i].getID().c_str()+"]";
          QComboBox *cb = new QComboBox(PARENT);
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
          BorderBox *poBorderBox = new BorderBox(vec[i].getID().c_str(),cb,PARENT);
          l->addWidget(poBorderBox);
          break;
        }
        default: // not yet supported via gui!
          break;
      }
    }    
    printf("everything done !\n");
    m_bDisableSlots = false;
  }

  // }}}
}

