/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/CamCfgWidget.cpp                             **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLQt/CamCfgWidget.h>
#include <ICLUtils/StackTimer.h>
#include <ICLUtils/Thread.h>

#include <ICLIO/Grabber.h>

#ifdef HAVE_VIDEODEV
#include <ICLIO/PWCGrabber.h>
#endif

#ifdef HAVE_UNICAP
#include <ICLIO/UnicapGrabber.h>
#endif

#ifdef HAVE_LIBDC
#include <ICLIO/DCGrabber.h>
#endif

#ifdef HAVE_LIBMESASR
#include <ICLIO/SwissRangerGrabber.h>
#endif

#include <ICLQt/Widget.h>
#include <ICLQt/DoubleSlider.h>
#include <ICLUtils/Thread.h>
#include <ICLQt/BorderBox.h>
#include <ICLQt/ImgParamWidget.h>
#include <ICLQt/StringSignalButton.h>
#include <ICLUtils/StringUtils.h>

#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QSizePolicy>
//#include <QTimer>
#include <QPushButton>
#include <QGroupBox>

#include <QMessageBox>
#include <QFileDialog>

#include <ICLUtils/Thread.h>

using namespace icl;
using namespace std;

namespace{
  struct CamCfgThread : public Thread{
    CamCfgWidget *parent;
    bool on;
    CamCfgThread(CamCfgWidget *parent):parent(parent),on(false){
      start();
    }
    virtual void run(){
      while(1){
        while(!on){
          msleep(100);
        }
        lock();
        parent->updateImage();
        parent->updateInfoLabels();
        unlock();
        msleep(10);
      }
    } 
    void setOn(bool on){
      this->on = on;
    }
    void changeOn(){
      on = !on;
    }
    bool isOn() const{
      return on;
    }
  };

}

namespace icl{

  struct CamCfgWidget::InfoLabel{
    CompabilityLabel *label;
    std::string propName;
    int tabIdx;
    void update(Grabber *grabber){
      std::string t = grabber->getValue(propName);
      if(t != label->text().toLatin1().data()){
        label->setText(t.c_str());
        label->updateFromOtherThread();
      }
    }
  };
  
  void CamCfgWidget::updateInfoLabels(){
    QMutexLocker l(&m_infoLabelMutex);
    QMutexLocker ql(&m_oGrabberMutex);

    if(!m_poGrabber) return; 
    for(unsigned int i=0;i<m_infoLabels.size();++i){
      /// this might be unsafe!
      if(m_infoLabels[i]->tabIdx == m_poTabWidget->currentIndex()){
        m_infoLabels[i]->update(m_poGrabber);
      }
    }
  }

  
  void CamCfgWidget::initialize(CamCfgWidget::CreationFlags flags){

    m_poGrabber = 0;
    m_bDisableSlots = false;
    m_bCapturing = false;
    m_isoMBits = flags.isoMBits;
    m_oFPSE = FPSEstimator(10);
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

    
    m_poLoadButton = new QPushButton("load",this);
    m_poSaveButton = new QPushButton("save",this);
    m_poGrabButtonAndFpsLabelLayout->addWidget(m_poLoadButton);
    m_poGrabButtonAndFpsLabelLayout->addWidget(m_poSaveButton);
    
    connect(m_poLoadButton,SIGNAL(clicked()),this,SLOT(loadClicked()));
    connect(m_poSaveButton,SIGNAL(clicked()),this,SLOT(saveClicked()));

    /// RIGHT WIDGETS-----------------------------------------
    m_bDisableSlots = true;


    /// parse the device hint list..
    bool useHintList = flags.deviceHintList != "";

    std::map<std::string,std::string> hints;
    
    if(useHintList){
      std::vector<std::string> tokens = tok(flags.deviceHintList,",");
      
      for(unsigned int i=0;i<tokens.size();++i){
        if(tokens[i].size() >= 2){
          string::size_type p = tokens[i].find('=');
          std::string dev = tokens[i].substr(0,p);
          if(p != string::npos && p < tokens[i].size() ){
            hints[dev] = tokens[i].substr(p+1);
          }else{
            hints[dev] = "";
          }
        }else{
          ERROR_LOG("token is too short: " << tokens[i]);
        }
      }
      
      flags.disableUnicap = (hints.find("unicap") == hints.end());
      flags.disableDC = (hints.find("dc") == hints.end());
      flags.disablePWC = (hints.find("pwc") == hints.end());
    }

    
    
    int jAll = 0;

#ifdef HAVE_UNICAP
    if(!flags.disableUnicap){
      /// add unicap devices: ?? how to deactivate dc devices ??
      m_vecDeviceList = UnicapGrabber::getDeviceList(hints["unicap"]);
     
      for(unsigned int j=0;j<m_vecDeviceList.size();j++){
        QString name = QString("[UNICAP]")+m_vecDeviceList[j].getID().c_str();
        m_poDeviceCombo->addItem(name.toLatin1().data());
        QWidget *w = new QWidget(this);
        QVBoxLayout *l = new QVBoxLayout(w);
        QScrollArea *sa = new QScrollArea(this);
        sa->setWidgetResizable(true);
        UnicapGrabber grabber(m_vecDeviceList[j]);
        fillLayout(l,&grabber,name.toLatin1().data());
        sa->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        w->setLayout(l);
        sa->setWidget(w);      
        m_poTabWidget->addTab(sa,name);
        m_poTabWidget->setTabEnabled(jAll++,false);
      }
    }
#endif

#ifdef HAVE_LIBMESASR
    if(!flags.disableSR){
      try{
        SwissRangerGrabber grabber;
        m_poDeviceCombo->addItem("[SR] SwissRanger-Camera");
        QWidget *w = new QWidget(this);
        QVBoxLayout *l = new QVBoxLayout(w);
        QScrollArea *sa = new QScrollArea(this);
        sa->setWidgetResizable(true);
        fillLayout(l,&grabber,"[SR] SwissRanger-Camera");
        sa->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        w->setLayout(l);
        sa->setWidget(w);      
        m_poTabWidget->addTab(sa,"[SR] SwissRanger-Camera");
        m_poTabWidget->setTabEnabled(jAll++,false);
      }catch(...){}
    }

#endif
     
#ifdef HAVE_LIBDC
    if(!flags.disableDC){
      /// add DC devices
      m_vecDCDeviceList = DCGrabber::getDeviceList(flags.resetDCBus);
      for(unsigned int j=0;j<m_vecDCDeviceList.size();j++){
        if(useHintList && hints["dc"] != ""){
          int dev = to32s(hints["dc"]);
          if(dev != (int)j) continue;
        }
        QString name = QString("[DC]")+m_vecDCDeviceList[j].getUniqueStringIdentifier().c_str();
        m_poDeviceCombo->addItem(name);
        QWidget *w = new QWidget(this);
        QVBoxLayout *l = new QVBoxLayout(w);
        QScrollArea *sa = new QScrollArea(this);
        sa->setWidgetResizable(true);
        DCGrabber grabber(m_vecDCDeviceList[j]);
        //      grabber.grab();
        fillLayout(l,&grabber,name.toLatin1().data());
        sa->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        w->setLayout(l);
        sa->setWidget(w);      
        m_poTabWidget->addTab(sa,name);
        m_poTabWidget->setTabEnabled(jAll++,false);
      }
    }
#endif
    
#ifdef HAVE_VIDEODEV
    if(!flags.disablePWC){
      /// add philips webcam devices
      m_vecPWCDeviceList = PWCGrabber::getDeviceList();
      
      for(unsigned int j=0;j<m_vecPWCDeviceList.size();j++){

        if(useHintList && hints["pwc"] != ""){
          int dev = to32s(hints["pwc"]);
          if(dev != (int)j) continue;
        }

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
        fillLayout(l,&grabber,name.toLatin1().data());
        sa->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        w->setLayout(l);
        sa->setWidget(w);      
        m_poTabWidget->addTab(sa,name);
        m_poTabWidget->setTabEnabled(jAll++,false);
      }
    }
#endif

    //m_poTopLevelLayout->addWidget(m_poTabWidget);
    m_poBottomSplitter->addWidget(m_poTabWidget);

    m_bDisableSlots = false;
    if(m_poDeviceCombo->count()){
      createGrabber(m_poDeviceCombo->currentText());     
      updateFormatCombo();
    }
 
    /// FINISHING : FINAL LAYOUTING
    //    setLayout(m_poVTopLevelLayout);
    m_thread = new CamCfgThread(this);
    //    m_poTimer = new QTimer(this);
    //connect(m_poTimer,SIGNAL(timeout()),this,SLOT(updateImage()));
    //show();
  }

  // }}}

  void CamCfgWidget::setVisible (bool visible){
    if(!visible){
      if(m_poCaptureButton->isChecked()){
        m_poCaptureButton->setChecked(false);
      }
    }
    QSplitter::setVisible(visible);
  }
  
  CamCfgWidget::~CamCfgWidget(){
    if(m_bCapturing){
      startStopCapture(false);
    }
    ICL_DELETE(m_thread);
    m_oGrabberMutex.lock();
    ICL_DELETE(m_poGrabber);

    m_oGrabberMutex.unlock();
    
  }

  const ImgBase *CamCfgWidget::getCurrentImage(){
  // {{{ open
    return grabbedImage;
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
    
    if(prefix == "DC"){
#ifdef HAVE_LIBDC
      m_vecDCDeviceList = DCGrabber::getDeviceList();
      
      for(unsigned int i=0;i<m_vecDCDeviceList.size();i++){
        if(m_vecDCDeviceList[i].getUniqueStringIdentifier() == text.toLatin1().data()){
          m_poGrabber = new DCGrabber(m_vecDCDeviceList[i], m_isoMBits);
          break;
        }
      }
#endif
    }else if(prefix == "SR"){
#ifdef HAVE_LIBMESASR
      m_poGrabber = new SwissRangerGrabber;
#endif
    }else if(prefix == "UNICAP"){
#ifdef HAVE_UNICAP

#ifdef HAVE_LIBDC
      m_vecDCDeviceList.clear();
      icl::dc::free_static_context();
#endif
      Thread::sleep(1);
      
      //search for a unicap grabber:
      for(unsigned int i=0;i<m_vecDeviceList.size();i++){
        if(m_vecDeviceList[i].getID() == text.toLatin1().data()){
          m_poGrabber = new UnicapGrabber(m_vecDeviceList[i]);
          Thread::sleep(1);
          break;
        }
      }
#endif
    }else if(prefix == "PWC"){
#ifdef HAVE_VIDEODEV

#ifdef HAVE_LIBDC
      m_vecDCDeviceList.clear();
      icl::dc::free_static_context();
#endif
      Thread::sleep(1);

      if(text == " Philips 740 Webcam [device 0]"){
        m_poGrabber = new PWCGrabber(Size::null,40,0);
      }else if(text == " Philips 740 Webcam [device 1]"){
        m_poGrabber = new PWCGrabber(Size::null,40,1);
      }else if(text == " Philips 740 Webcam [device 2]"){
        m_poGrabber = new PWCGrabber(Size::null,40,2);
      }else if(text == " Philips 740 Webcam [device 3]"){
        m_poGrabber = new PWCGrabber(Size::null,40,3);
      }
      #endif
    }

    if(!m_poGrabber){
      ERROR_LOG("video device "<< text.toLatin1().data() << "no longer available!");
      m_oGrabberMutex.unlock();
      return;
    }
    if(m_oVideoSize == Size(-1,-1)){
      m_poGrabber->setIgnoreDesiredParams(true);
    }else{
      m_poGrabber->setIgnoreDesiredParams(false);
      m_poGrabber->setDesiredSize(m_oVideoSize);
      m_poGrabber->setDesiredDepth(m_eVideoDepth);
      m_poGrabber->setDesiredFormat(m_eVideoFormat);
    }
    m_oGrabberMutex.unlock();

    if(wasCapturing){
      startStopCapture(true);
    }
  }

  // }}}
  
  QString CamCfgWidget::getConfigurationFile(){
    // {{{ open
    
    return configurationFile;
  }

  // }}}

  void CamCfgWidget::setConfigurationFile(QString config){
    // {{{ open

    configurationFile = config;
    if(!config.isEmpty()){
      load(config);
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
      if(m_oVideoSize == Size(-1,-1)){
        m_poGrabber->setIgnoreDesiredParams(true);
      }else{
        m_poGrabber->setIgnoreDesiredParams(false);
        m_poGrabber->setDesiredSize(m_oVideoSize);
        m_poGrabber->setDesiredDepth(m_eVideoDepth);
        m_poGrabber->setDesiredFormat(m_eVideoFormat);
      }
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
        ((CamCfgThread*)m_thread)->setOn(true);
        //        m_poTimer->start(10);
      }
    }else{
      if(m_bCapturing){
        if(!m_bDisableSlots){
          m_bDisableSlots = true;
          m_poCaptureButton->setChecked(false);
          m_bCapturing = false;
          ((CamCfgThread*)m_thread)->setOn(false);
          //          m_poTimer->stop();
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
      grabbedImage = m_poGrabber->grab();
      
      m_poICLWidget->setImage(grabbedImage);
      m_poICLWidget->update();

      m_poFpsLabel->setText(m_oFPSE.getFPSString().c_str());
      m_poFpsLabel->updateFromOtherThread();
    }
    m_oGrabberMutex.unlock();
    
    emit(newImageGrabbed(grabbedImage));

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
 


  void CamCfgWidget::fillLayout(QLayout *l, Grabber *grabber, const std::string &name){
    // {{{ open

    GUIComponentAssociationTable &gcat = m_guiCompAss[name]; // this is created new here!
    
    vector<string> propertyList = grabber->getPropertyList();
    
    QWidget *PARENT = 0;
    for(unsigned int i=0;i<propertyList.size();i++){
      string &prop = propertyList[i];



      if(prop == "size" || prop == "format") continue; // this two properties are handled externally

      string typeStr = grabber->getType(prop);

      GUIComponentAssociation &gca = gcat[prop];
      gca.type = typeStr;

      if(typeStr == "range"){
        SteppingRange<double> sr = Grabber::translateSteppingRange(grabber->getInfo(prop));
        DoubleSlider *ds = new DoubleSlider(PARENT,prop.c_str());
        
        gca.widget = ds;
        
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

        gca.widget = cb;
        gca.propName = propName.toLatin1().data();
        
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

        gca.widget = cb;
        gca.propName = propName.toLatin1().data();

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

        gca.widget = b;

        connect(b,SIGNAL(clicked(QString)),this,SLOT(propertyButtonClicked(QString)));
        BorderBox *poBorderBox = new BorderBox(prop.c_str(),b,PARENT);
        poBorderBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
        l->addWidget(poBorderBox);
      }else if(typeStr == "info"){
        QMutexLocker lock(&m_infoLabelMutex);
        m_infoLabels.push_back(new InfoLabel);
        InfoLabel &il = *m_infoLabels.back(); 
        il.label = new CompabilityLabel("",PARENT);
        il.label->setMinimumSize(QSize(100,15));
        il.propName = prop;
        il.tabIdx = m_poTabWidget->count();
        BorderBox *poBorderBox = new BorderBox(prop.c_str(),il.label,PARENT);
        poBorderBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
        l->addWidget(poBorderBox);
        
      }
      m_bDisableSlots = false;
    }
  }
  
  // }}}

 


  void CamCfgWidget::loadClicked(){
    if(!m_poGrabber){
      QMessageBox::information(this,"No device selected!","You can only load properties if a device is selected (and running)");
      return;
    }
    QString s = QFileDialog::getOpenFileName(this,"Load device properties ...","","XML-files (*.xml)");
    if(!s.isNull() && s!=""){
      load(s);
    }
    
   
  }
  

  void CamCfgWidget::load(QString filename){
    configurationFile = filename;
    try{
      bool wasCapturing = m_bCapturing;
      if(wasCapturing){
        startStopCapture(false);
        Thread::msleep(500);
      }
      m_poGrabber->loadProperties(filename.toLatin1().data(),false);

      std::string currGrabberName = m_poDeviceCombo->currentText().toLatin1().data();
      std::map<std::string,GUIComponentAssociationTable>::iterator it = m_guiCompAss.find(currGrabberName);
      if(it == m_guiCompAss.end()){
        ERROR_LOG("no GUI components were associated with current grabber ???");
        if(wasCapturing){
          Thread::msleep(500);
          startStopCapture(true);
        }
        return;
      }
      GUIComponentAssociationTable &gcat = it->second;
      for(GUIComponentAssociationTable::iterator it2=gcat.begin(); it2 != gcat.end(); ++it2){
        const std::string &prop = it2->first;
        GUIComponentAssociation &gca = it2->second;

        if(gca.type == "menu" || gca.type == "value-list"){
          QComboBox *cb = dynamic_cast<QComboBox*>(gca.widget);
          if(!cb){
            ERROR_LOG("invalid component for type menu/value-list (expected QComboBox)");
          }else{
            std::string val = m_poGrabber->getValue(prop);
            if(gca.propName != "") val = gca.propName+" "+val;
            //DEBUG_LOG("value for property -" << prop << "- is -" << val << "-");
            int idx = cb->findText(val.c_str());
            if(idx >= 0){
              cb->setCurrentIndex(idx);
            }else{
              ERROR_LOG("found illegal value-list or menu entry for property: \"" << prop << "\"");
            }
          }
        }else if(gca.type == "range"){
          DoubleSlider *ds = dynamic_cast<DoubleSlider*>(gca.widget);
          if(!ds){
            ERROR_LOG("invalid component for type range (expected DoubleSlider)");
          }else{
            ds->setDoubleValue(parse<double>(m_poGrabber->getValue(prop)));
          }
        }
      }
      if(wasCapturing){
        Thread::msleep(500);
        startStopCapture(true);
      }
    }catch(ICLException &e){
      ERROR_LOG(e.what());
    }
  }


  void CamCfgWidget::saveClicked(){
    if(!m_poGrabber){
      QMessageBox::information(this,"No device selected!","You can only save properties if a device is selected (and running)");
      return;
    }

    QString s = QFileDialog::getSaveFileName(this,"Save device properties ...","","XML-files (*.xml)");
    if(!s.isNull() && s!=""){
      save(s);
    }
  }
  
  
  void CamCfgWidget::save(QString filename){
    configurationFile = filename;
    try{
      QMessageBox::StandardButton b = QMessageBox::question(this,"prepare saving ...","Remove instable parameters?\n(recommended)",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
    
      //        DEBUG_LOG("ignore desired:" << str(m_poGrabber->getIgnoreDesiredParams()));
      m_poGrabber->saveProperties(filename.toLatin1().data(),false,b==QMessageBox::Yes);
    }catch(ICLException &e){
      ERROR_LOG(e.what());
    }
  }
}

