#include "iclCamCfgWidget.h"
#include "iclStackTimer.h"

#include <iclPWCGrabber.h>
#include <iclUnicapGrabber.h>
#include <iclDCGrabber.h>
#include <iclWidget.h>
#include <iclDoubleSlider.h>
#include <iclThread.h>
#include <iclBorderBox.h>
#include <iclImgParamWidget.h>
#include <iclStringSignalButton.h>

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
  
  CamCfgWidget::CamCfgWidget(const CamCfgWidget::CreationFlags &flags,QWidget *parent):
    QSplitter(Qt::Vertical,parent),m_poGrabber(0), 
    m_bDisableSlots(false), m_bCapturing(false), 
    m_isoMBits(flags.isoMBits),
    m_omitDoubledDCFrames(flags.omitDoubledDCFrames),
    m_oFPSE(10) {
    // {{{ open

    // TOP LEVEL
    //m_poTopLevelLayout = new QHBoxLayout(0);
    //m_poVTopLevelLayout = new QVBoxLayout(0);

    //m_poHBoxWidget = new QWidget(this);
    m_poBottomSplitter = new QSplitter(Qt::Horizontal,this);

    m_poICLWidget = new ICLWidget(this);
    m_poICLWidget->setGeometry(0,0,640,480);
    this->addWidget(m_poICLWidget);
    this->addWidget(m_poBottomSplitter);
    //m_poHBoxWidget->setLayout(m_poTopLevelLayout);
    //m_poHBoxWidget->setMaximumHeight(400);
    /// THREE PANELS
    m_poCenterPanel = new QWidget(this);
    m_poTabWidget = new QTabWidget(this);
    
    m_poTabWidget->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    m_poCenterPanelLayout = new QVBoxLayout(m_poCenterPanel);
    m_poCenterPanel->setLayout(m_poCenterPanelLayout);
    //m_poTopLevelLayout->addWidget(m_poCenterPanel);
    m_poBottomSplitter->addWidget(m_poCenterPanel);

    /// CENTER WIDGETS
    m_poDeviceCombo = new QComboBox(this);
    m_poCenterPanelLayout->addWidget(new BorderBox("device",m_poDeviceCombo,this));
    
    m_poFormatCombo = new QComboBox(this);
    m_poCenterPanelLayout->addWidget(new BorderBox("format",m_poFormatCombo,this));
    
    m_poSizeCombo = new QComboBox(this);
    m_poCenterPanelLayout->addWidget(new BorderBox("size",m_poSizeCombo,this));

    m_poCaptureButton = new QPushButton("capture!",this);
    m_poCaptureButton->setCheckable(true);

    m_poFpsLabel  = new CompabilityLabel("fps: ---",this);
    m_poGrabButtonAndFpsLabelWidget = new QWidget(this);
    m_poGrabButtonAndFpsLabelLayout = new QHBoxLayout(m_poGrabButtonAndFpsLabelWidget);
    m_poGrabButtonAndFpsLabelLayout->addWidget(m_poCaptureButton);
    m_poGrabButtonAndFpsLabelLayout->addWidget(m_poFpsLabel);
    m_poGrabButtonAndFpsLabelWidget->setLayout(m_poGrabButtonAndFpsLabelLayout);
    m_poCenterPanelLayout->addWidget(m_poGrabButtonAndFpsLabelWidget);
    m_poImgParamWidget = new ImgParamWidget(this);
    BorderBox *poBorderBox = new BorderBox("output image",m_poImgParamWidget,this);
    m_poCenterPanelLayout->addWidget(poBorderBox);
    int w,h,d,f;
    m_poImgParamWidget->getParams(w,h,d,f);
    visImageParamChanged(w,h,d,f);

    connect(m_poImgParamWidget,SIGNAL(somethingChanged(int,int,int,int)),this,SLOT(visImageParamChanged(int,int,int,int)));

    connect(m_poDeviceCombo,SIGNAL(currentIndexChanged(QString)), SLOT(deviceChanged(QString)));
    connect(m_poDeviceCombo,SIGNAL(currentIndexChanged(int)), SLOT(deviceChanged(int)));
    connect(m_poFormatCombo,SIGNAL(currentIndexChanged(QString)), SLOT(formatChanged(QString)));
    connect(m_poSizeCombo,SIGNAL(currentIndexChanged(QString)), SLOT(sizeChanged(QString)));
    connect(m_poCaptureButton,SIGNAL(toggled(bool)),this,SLOT(startStopCapture(bool)));

    /// RIGHT WIDGETS-----------------------------------------
    m_bDisableSlots = true;
    
    int jAll = 0;
    if(!flags.disableUnicap){
      /// add unicap devices: ?? how to deactivate dc devices ??
      m_vecDeviceList = UnicapGrabber::getDeviceList();
     
      for(unsigned int j=0;j<m_vecDeviceList.size();j++){
        QString name = QString("[UNICAP]")+m_vecDeviceList[j].getID().c_str();
        m_poDeviceCombo->addItem(name);
        QWidget *w = new QWidget(this);
        QVBoxLayout *l = new QVBoxLayout(w);
        QScrollArea *sa = new QScrollArea(this);
        sa->setWidgetResizable(true);
        UnicapGrabber grabber(m_vecDeviceList[j]);
        fillLayout(l,&grabber);
        sa->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        w->setLayout(l);
        sa->setWidget(w);      
        m_poTabWidget->addTab(sa,name);
        m_poTabWidget->setTabEnabled(jAll++,false);
      }
    }
     
    if(!flags.disableDC){
      /// add DC devices
      m_vecDCDeviceList = DCGrabber::getDeviceList(flags.resetDCBus);
      for(unsigned int j=0;j<m_vecDCDeviceList.size();j++){
        QString name = QString("[DC]")+m_vecDCDeviceList[j].getModelID().c_str();
        m_poDeviceCombo->addItem(name);
        QWidget *w = new QWidget(this);
        QVBoxLayout *l = new QVBoxLayout(w);
        QScrollArea *sa = new QScrollArea(this);
        sa->setWidgetResizable(true);
        DCGrabber grabber(m_vecDCDeviceList[j]);
        //      grabber.grab();
        fillLayout(l,&grabber);
        sa->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        w->setLayout(l);
        sa->setWidget(w);      
        m_poTabWidget->addTab(sa,name);
        m_poTabWidget->setTabEnabled(jAll++,false);
      }
    }
    
    if(!flags.disablePWC){
      /// add philips webcam devices
      m_vecPWCDeviceList = PWCGrabber::getDeviceList();
      
      for(unsigned int j=0;j<m_vecPWCDeviceList.size();j++){
        QString name  = QString("[PWC] Philips 740 Webcam [device ")+QString::number(j)+"]";
        m_poDeviceCombo->addItem(name);
        QWidget *w = new QWidget(this);
        QVBoxLayout *l = new QVBoxLayout(w);
        QScrollArea *sa = new QScrollArea(this);
        sa->setWidgetResizable(true);
        PWCGrabber grabber;
        if(!grabber.init(Size::null,24,j)){
          printf("error while initializing grabber device %d! \n",j);
        }
        fillLayout(l,&grabber);
        sa->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        w->setLayout(l);
        sa->setWidget(w);      
        m_poTabWidget->addTab(sa,name);
        m_poTabWidget->setTabEnabled(jAll++,false);
      }
    }

    //m_poTopLevelLayout->addWidget(m_poTabWidget);
    m_poBottomSplitter->addWidget(m_poTabWidget);

    m_bDisableSlots = false;
    if(m_poDeviceCombo->count()){
      createGrabber(m_poDeviceCombo->currentText());     
      updateFormatCombo();
    }
 
    /// FINISHING : FINAL LAYOUTING
    //    setLayout(m_poVTopLevelLayout);
    setGeometry(50,50,800,800);
    setWindowTitle("ICL Camera Configuration Tool");
    m_poTimer = new QTimer(this);
    connect(m_poTimer,SIGNAL(timeout()),this,SLOT(updateImage()));
    show();
  }

  // }}}
  
  CamCfgWidget::~CamCfgWidget(){
    if(m_bCapturing){
      startStopCapture(false);
    }
    m_oGrabberMutex.lock();
    ICL_DELETE(m_poGrabber);
    m_oGrabberMutex.unlock();
    
  }

  const ImgBase *CamCfgWidget::getCurrentImage(){
  // {{{ open
    const ImgBase *image;

    image = 0;

    m_oGrabberMutex.lock();
    if(m_poGrabber){
      image = m_poGrabber->grab();
    }
    m_oGrabberMutex.unlock();
    return image;
  }
  
  // }}}
  void CamCfgWidget::createGrabber(const QString &textWithPrefix){
    // {{{ open
    QString prefix = textWithPrefix.section(']',0,0).toLatin1().data()+1;
    QString text = textWithPrefix.section(']',1);
    
    bool wasCapturing = m_bCapturing;
    
    if(m_bCapturing){
      startStopCapture(false);
    }
    
    m_oGrabberMutex.lock();
    if(m_poGrabber) delete m_poGrabber;
    m_poGrabber = 0;
    
    DEBUG_LOG("creating grabber with this prefix: " << prefix.toLatin1().data() );
    if(prefix == "DC"){
      
      m_vecDCDeviceList = DCGrabber::getDeviceList();
      
      for(unsigned int i=0;i<m_vecDCDeviceList.size();i++){
        if(m_vecDCDeviceList[i].getModelID() == text.toLatin1().data()){
          m_poGrabber = new DCGrabber(m_vecDCDeviceList[i], m_isoMBits,m_omitDoubledDCFrames);
          break;
        }
      }
    }else if(prefix == "UNICAP"){
      m_vecDCDeviceList.clear();
      icl::dc::free_static_context();

      Thread::sleep(1);
      
      //search for a unicap grabber:
      for(unsigned int i=0;i<m_vecDeviceList.size();i++){
        if(m_vecDeviceList[i].getID() == text.toLatin1().data()){
          m_poGrabber = new UnicapGrabber(m_vecDeviceList[i]);
          Thread::sleep(1);
          break;
        }
      }
    }else if(prefix == "PWC"){

      m_vecDCDeviceList.clear();

      icl::dc::free_static_context();

      Thread::sleep(2);



      if(text == " Philips 740 Webcam [device 0]"){
        m_poGrabber = new PWCGrabber(Size::null,40,0);
      }else if(text == " Philips 740 Webcam [device 1]"){
        m_poGrabber = new PWCGrabber(Size::null,40,1);
      }else if(text == " Philips 740 Webcam [device 2]"){
        m_poGrabber = new PWCGrabber(Size::null,40,2);
      }else if(text == " Philips 740 Webcam [device 3]"){
        m_poGrabber = new PWCGrabber(Size::null,40,3);
      }
    }

    if(!m_poGrabber){
      ERROR_LOG("video device "<< text.toLatin1().data() << "no longer available!");
      m_oGrabberMutex.unlock();
      return;
    }
    m_poGrabber->setDesiredSize(m_oVideoSize);
    m_poGrabber->setDesiredDepth(m_eVideoDepth);
    m_poGrabber->setDesiredFormat(m_eVideoFormat);
    m_oGrabberMutex.unlock();

    if(wasCapturing){
      startStopCapture(true);
    }
  }

  // }}}

  void CamCfgWidget::deviceChanged(const QString &text){
    // {{{ open

    if(m_bDisableSlots) return;
    createGrabber(text);
    updateFormatCombo();
    formatChanged(m_poFormatCombo->currentText());
  }

  // }}}
  
  void CamCfgWidget::deviceChanged(int index){
    // {{{ open
    m_poTabWidget->setCurrentIndex(index);
  }

  // }}}
  
  void CamCfgWidget::visImageParamChanged(int width, int height, int d, int fmt){
    // {{{ open
    m_oVideoSize = Size(width,height);
    m_eVideoFormat = (format)fmt;
    m_eVideoDepth = (icl::depth)d;
    
    if(m_poGrabber){
      m_oGrabberMutex.lock();
      m_poGrabber->setDesiredSize(m_oVideoSize);
      m_poGrabber->setDesiredDepth(m_eVideoDepth);
      m_poGrabber->setDesiredFormat(m_eVideoFormat);
      m_oGrabberMutex.unlock();
    }
  }

  // }}}

  void CamCfgWidget::formatChanged(const QString &text){
    // {{{ open

    if(!m_poGrabber || m_bDisableSlots) return;
    
    
    m_oGrabberMutex.lock();
    if(m_poGrabber->supportsProperty("format") && text != ""){
      m_poGrabber->setProperty("format",text.toLatin1().data());
    }
    m_oGrabberMutex.unlock();
  }

  // }}}
  void CamCfgWidget::sizeChanged(const QString &text){
    // {{{ open
    if(!m_poGrabber || m_bDisableSlots) return;
    m_oGrabberMutex.lock();
    if(m_poGrabber->supportsProperty("size") && text != ""){
      m_poGrabber->setProperty("size",text.toLatin1().data());
    }
    m_oGrabberMutex.unlock();
  }

  // }}}
  void CamCfgWidget::propertySliderChanged(const QString &id, double value){
    // {{{ open
    ICLASSERT_RETURN(m_poGrabber);
    m_oGrabberMutex.lock();
    m_poGrabber->setProperty(id.toLatin1().data(),QString::number(value).toLatin1().data());
    m_oGrabberMutex.unlock();
  }

  // }}}
  void CamCfgWidget::propertyComboBoxChanged(const QString &text){
    // {{{ open

    ICLASSERT_RETURN(m_poGrabber);
    QString first = text.section(']',0,0);
    QString sec = text.section(']',1,1);
    string propName = first.toLatin1().data()+1;
    string propValue = sec.toLatin1().data()+1;
    m_oGrabberMutex.lock();
    m_poGrabber->setProperty(propName,propValue);
    m_oGrabberMutex.unlock();
  }
  // }}}

  void CamCfgWidget::propertyButtonClicked(const QString &text){
    ICLASSERT_RETURN(m_poGrabber);
    m_oGrabberMutex.lock();
    m_poGrabber->setProperty(text.toLatin1().data(),"");
    m_oGrabberMutex.unlock();
  }


  void CamCfgWidget::startStopCapture(bool on){
    // {{{ open
    if(on){
      if(!m_bCapturing){
        m_bCapturing = true;
        m_poTimer->start(10);
      }
    }else{
      if(m_bCapturing){
        if(!m_bDisableSlots){
          m_bDisableSlots = true;
          m_poCaptureButton->setChecked(false);
          m_bCapturing = false;
          m_poTimer->stop();
          m_bDisableSlots = false;
        }          
      }
    }
  }

  // }}}


  void CamCfgWidget::updateImage(){
    // {{{ open

    m_oGrabberMutex.lock();
    if(m_poGrabber){
      const ImgBase *image = m_poGrabber->grab();
      m_poICLWidget->setImage(image);
      m_poICLWidget->updateFromOtherThread();

      m_poFpsLabel->setText(m_oFPSE.getFpsString().c_str());
      m_poFpsLabel->updateFromOtherThread();
    }
    m_oGrabberMutex.unlock();
    

    /****
        static float accu = 0;
        static const float fac = 0.1;
        accu = accu*(1.0-fac) + fac*(m_poGrabber->getCurrentFps());
        accu = ((float)((int)(accu*100)))/100;
        m_poFpsLabel->setText(QString("fps: ")+QString::number(accu));
    ***/
  }

  // }}}
  void CamCfgWidget::updateSizeCombo(){
    // {{{ open
    ICLASSERT_RETURN(m_poGrabber);
    m_bDisableSlots = true;
    while(m_poSizeCombo->count()){
      m_poSizeCombo->removeItem(0);
    }
    if(m_poGrabber->supportsProperty("size")){
      string sizeListStr = m_poGrabber->getInfo("size");
      vector<string> sizeList = Grabber::translateStringVec( sizeListStr );
      string currSizeStr = m_poGrabber->getValue("size");
      
      int iCurrIdx = -1;
      for(unsigned int i=0;i<sizeList.size();i++){
        if(sizeList[i]==currSizeStr){
          iCurrIdx = i;
        }
        m_poSizeCombo->addItem(sizeList[i].c_str());
      }
      if(iCurrIdx != -1){
        m_poSizeCombo->setCurrentIndex(iCurrIdx);
      }
    }
    m_bDisableSlots = false;
  }
    // }}}

  void CamCfgWidget::updateFormatCombo(){
    // {{{ open
    ICLASSERT_RETURN(m_poGrabber);
    m_bDisableSlots = true;
    while(m_poFormatCombo->count()){
      m_poFormatCombo->removeItem(0);
    }
    

    
    if(m_poGrabber->supportsProperty("format")){
      string sCurrFormat = m_poGrabber->getValue("format");
      
      string fmtListStr = m_poGrabber->getInfo("format");
      vector<string> fmtList = Grabber::translateStringVec( fmtListStr );
      
      int useFmtIdx = -1;
      for(unsigned int i=0;i<fmtList.size();i++){
        if(sCurrFormat == fmtList[i]) useFmtIdx = i;
        m_poFormatCombo->addItem(fmtList[i].c_str());
      }
      if(useFmtIdx != -1){
        m_poFormatCombo->setCurrentIndex(useFmtIdx);
      }
    }
    

    
    m_bDisableSlots = false;
    updateSizeCombo();
  }

  // }}}
 


  void CamCfgWidget::fillLayout(QLayout *l, Grabber *grabber){
    // {{{ open

    vector<string> propertyList = grabber->getPropertyList();
    
    QWidget *PARENT = 0;
    for(unsigned int i=0;i<propertyList.size();i++){
      string &prop = propertyList[i];
      if(prop == "size" || prop == "format") continue; // this two properties are handled externally
      
      string typeStr = grabber->getType(prop);
      if(typeStr == "range"){
        SteppingRange<double> sr = Grabber::translateSteppingRange(grabber->getInfo(prop));
        DoubleSlider *ds = new DoubleSlider(PARENT,prop.c_str());
        ds->setMinDouble(sr.minVal);
        ds->setMaxDouble(sr.maxVal);
        if(sr.stepping != 0){
          ds->setDoubleStepping(sr.stepping);
        }

        ds->setDoubleValue(atof(grabber->getValue(prop).c_str()));
        
        BorderBox *poBorderBox = new BorderBox(prop.c_str(),ds,PARENT);
        poBorderBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
        l->addWidget(poBorderBox);
        connect(ds,SIGNAL(doubleValueChanged(const QString&,double)),this,SLOT(propertySliderChanged(const QString&,double)));
      }else if(typeStr == "valueList"){
        QString propName = QString("[")+prop.c_str()+"]";
        QComboBox *cb = new QComboBox(PARENT);
        vector<double> values = Grabber::translateDoubleVec( grabber->getInfo(prop) );
        string currValue = grabber->getValue(prop);
        int iCurrIdx = -1;
        for(unsigned int j=0;j<values.size();j++){
          char buf[30];
          sprintf(buf,"%f",values[j]);
          if( abs(values[j] - atof(currValue.c_str())) < 0.001 ){
            iCurrIdx = j;
          }
          cb->addItem(propName+" "+buf);
        }
        if(iCurrIdx != -1){
          cb->setCurrentIndex(iCurrIdx);
        }
        connect(cb,SIGNAL(currentIndexChanged(QString)),this,SLOT(propertyComboBoxChanged(QString)));
        BorderBox *poBorderBox = new BorderBox(prop.c_str(),cb,PARENT);
        poBorderBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
        l->addWidget(poBorderBox);
      }else if(typeStr == "menu"){
        QString propName = QString("[")+prop.c_str()+"]";
        QComboBox *cb = new QComboBox(PARENT);
        vector<string> men = Grabber::translateStringVec(grabber->getInfo(prop));
        string menuItem = grabber->getValue( prop );
        int iCurrIdx = -1;
        for(unsigned int j=0;j<men.size();j++){
          if(men[j] == menuItem){
            iCurrIdx = j;
          }
          cb->addItem(propName+" "+men[j].c_str());
        }
        if(iCurrIdx != -1){
          cb->setCurrentIndex(iCurrIdx); 
        }
        connect(cb,SIGNAL(currentIndexChanged(QString)),this,SLOT(propertyComboBoxChanged(QString)));
        BorderBox *poBorderBox = new BorderBox(prop.c_str(),cb,PARENT);
        poBorderBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));

        l->addWidget(poBorderBox);
      }else if(typeStr == "command"){
        StringSignalButton *b = new StringSignalButton(prop.c_str(),PARENT);
        connect(b,SIGNAL(clicked(QString)),this,SLOT(propertyButtonClicked(QString)));
        BorderBox *poBorderBox = new BorderBox(prop.c_str(),b,PARENT);
        poBorderBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
        l->addWidget(poBorderBox);
      }
      m_bDisableSlots = false;
    }
  }
  
  // }}}
}

