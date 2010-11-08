/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQt/CamCfgWidget.h>

#include <ICLIO/GenericGrabber.h>
#include <ICLQt/GUIWidget.h>
#include <ICLQt/ComboHandle.h>
#include <ICLQt/BoxHandle.h>
#include <ICLQt/LabelHandle.h>
#include <ICLQt/CheckBoxHandle.h>
#include <ICLQt/ButtonHandle.h>
#include <ICLUtils/FPSLimiter.h>

#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QBoxLayout>
#include <QtGui/QScrollArea>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtGui/QPushButton>


namespace icl{
  

  struct VolatileUpdater : public QTimer{
    std::string prop;
    GUI &gui;
    Grabber &grabber;
    VolatileUpdater(int msec, const std::string &prop, GUI &gui, Grabber &grabber):
      prop(prop),gui(gui),grabber(grabber){
      setInterval(msec);
    }
    virtual void timerEvent(QTimerEvent * e){
      LabelHandle &l = gui.getValue<LabelHandle>("#i#"+prop);
      (**l).setText(grabber.getValue(prop).c_str());
      (**l).update(); 
      QApplication::processEvents();
    }
  };
    
  struct CamCfgWidget::Data{
    bool complex;
    std::string deviceFilter;
    GUI gui;
    GenericGrabber grabber;
    std::vector<GrabberDeviceDescription> foundDevices;
    bool scanScope;
    bool settingUpDevice;
    bool grabbing;
    bool loadParamsScope;
    

    Data(bool complex):complex(complex),mutex(QMutex::Recursive),fps(5),fpsLimiter(10,10){
      scanScope = false;
      settingUpDevice = false;
      grabbing = false;
      useFPSLimiter = false;
      end = false;
      loadParamsScope = false;
    }
    
    QScrollArea *scroll;
    GUI propGUI; // contains the dataStore ...
    std::vector<SmartPtr<VolatileUpdater> > timers;
    QMutex mutex;
    
    FPSEstimator fps;
    FPSLimiter fpsLimiter;
    bool useFPSLimiter;
    bool end;
    
    
  };

  
  CamCfgWidget::CamCfgWidget(const std::string &deviceFilter,QWidget *parent):
    QWidget(parent), data(new Data(true)){
    
    data->deviceFilter = deviceFilter;
      
    data->gui = GUI("hsplit",this);
    
    data->gui <<  ( GUI("vsplit") 
                    << "image[@handle=image@minsize=8x6@label=preview]"
                    << ( GUI("vbox[@minsize=18x15@maxsize=100x15]")
                         << ( GUI("hbox[@label=devices]") 
                              << "combo(no devices found)[@handle=device]"
                              << "button(rescan)[@handle=scan@maxsize=3x8]"
                            )
                         << ( GUI("hbox") 
                              << "combo(no devices found)[@handle=format@label=format]"
                              << "combo(no devices found)[@handle=size@label=size]"
                            )
                         <<  ( GUI("hbox[@label=control / FPS]") 
                               << "togglebutton(capture!,stop)[@handle=capture@out=grabbing]"
                               << "combo(max 1Hz,max 5Hz,max 10Hz,max 15Hz,max 20Hz,max 25Hz,max"
                                  " 30Hz,max 50Hz,max 100Hz,max 120Hz,!no limit)[@handle=hz@maxsize=4x2]"
                               << "label(--.--)[@handle=fps]"
                              
                             )
                         <<  ( GUI("hbox[@label=desired params]")
                               << "checkbox(use,off)[@handle=desired-use]"
                               << "combo(QQVGA,!QVGA,VGA,SVGA,XGA,XGAP,UXGA)[@handle=desired-size@label=size]"
                               << "combo(!depth8u,depth16s,depth32s,depth32f,depth64f)[@handle=desired-depth@label=depth]"
                               << "combo(formatGray,!formatRGB,formatHLS,formatYUV,formatLAB,formatChroma,formatMatrix)"
                                  "[@handle=desired-format@label=format]"
                             )
                       )
                    )
              <<  ( GUI("vbox")
                    << "vbox[@handle=props@minsize=10x1@label=properties]"
                    << ( GUI("hbox[@maxsize=100x2]")
                         << "button(load props)[@handle=load]"
                         << "button(save props)[@handle=save]"
                        )
                   );

    data->gui.create();

    setLayout(new QBoxLayout(QBoxLayout::LeftToRight,this));
    layout()->setContentsMargins(2,2,2,2);
    layout()->addWidget(data->gui.getRootWidget());

    data->gui.registerCallback(SmartPtr<GUI::Callback>(this,false),"device,scan,format,size,capture,fps,load,save,"
                               "desired-use,desired-size,desired-depth,desired-format,hz");
    
    QWidget *w = (*data->gui.getValue<BoxHandle>("props"));
    w->layout()->setContentsMargins(0,0,0,0);
    w->layout()->addWidget(data->scroll = new QScrollArea(w));
    
    scan();
    start();
  }


  CamCfgWidget::CamCfgWidget(const std::string &devType, const std::string &devID,QWidget *parent):
    QWidget(parent),data(new Data(false)){
    // how to create the grabber
    
    std::string devText = "";
    bool needDeviceCombo = false;
    if(devType.length()){
      try{
        data->grabber.init(devType,devType+"="+devID);
        devText = devType + " " + devID;
      }catch(const std::exception &ex){
        QMessageBox::critical(0,"Unable to create CamCfgWidget!",
                              ex.what());
      }
    }else{
      data->foundDevices = GenericGrabber::getDeviceList("",false);
      if(!data->foundDevices.size()){
        QMessageBox::critical(0,"Unable to create CamCfgWidget!",
                              QString("no used devices available"));
      
      }else{
        if(data->foundDevices.size() == 1){
          devText = data->foundDevices.front().type + " " + data->foundDevices.front().id;
          data->grabber.init(data->foundDevices.front());
        }else{
          needDeviceCombo = true;
          devText = "";
          data->grabber.init(data->foundDevices.front());
        }
      }
    }

    data->gui = GUI("vbox",this);
    if(needDeviceCombo){
      data->gui << "combo(no devices found)[@handle=device@maxsize=100x2@label=available devices]";
    }
    data->gui << "vbox[@handle=props@minsize=10x1@label=properties for device "+devText+"]"
              << ( GUI("hbox[@maxsize=100x2]")
                   << "button(load props)[@handle=load]"
                   << "button(save props)[@handle=save]"
                 );
    data->gui.create();

    if(needDeviceCombo){
      ComboHandle devices = data->gui.getValue<ComboHandle>("device");
      devices.clear();
      for(unsigned int i=0;i<data->foundDevices.size();++i){
        devices.add("[" + data->foundDevices[i].type + "] ID:" + data->foundDevices[i].id);
      }
    }
    
    setLayout(new QBoxLayout(QBoxLayout::LeftToRight,this));
    layout()->setContentsMargins(2,2,2,2);
    layout()->addWidget(data->gui.getRootWidget());

    if(needDeviceCombo){
      data->gui.registerCallback(SmartPtr<GUI::Callback>(this,false),"load,save,device");
    }else{
      data->gui.registerCallback(SmartPtr<GUI::Callback>(this,false),"load,save");
    }
    
    QWidget *w = (*data->gui.getValue<BoxHandle>("props"));
    w->layout()->setContentsMargins(0,0,0,0);
    w->layout()->addWidget(data->scroll = new QScrollArea(w));

    exec("device");
  }

  const ImgBase *CamCfgWidget::getCurrentImage(){
    QMutexLocker __lock(&data->mutex); 
    if(data->grabber.isNull()) return 0;
    
    const ImgBase *image = data->grabber.grab();
    
    if(data->complex){
      data->gui["image"] = image;
      data->gui["image"].update();
      data->gui["fps"] = data->fps.getFPSString();
    }

    return image;
  }

  
  CamCfgWidget::~CamCfgWidget(){
    if(data->complex){
      data->end = true;
      Thread::stop();
    }
    delete data;
  }
  
  void CamCfgWidget::setVisible (bool visible){
    if(!visible){
      data->grabbing = false;
    }
    QWidget::setVisible(visible);
  }
  
  static std::string strip(const std::string &s, char a, char b){
    int begin = 0;
    int len = s.length();
    if(s[0] == a) {
      ++begin;
      --len;
    }
    if(s[s.length()-1] == b){
      --len;
    }
    return s.substr(begin,len);
  }

  std::string get_combo_content_str(const std::string &p, Grabber &grabber){
    std::ostringstream s;
    std::vector<std::string> ts = tok(strip(grabber.getInfo(p),'{','}'),",");
    for(unsigned int i=0;i<ts.size();++i){
      s << strip(ts[i],'"','"');
      if(i < ts.size()-1) s << ',';
    }
    return s.str();
  }
  
  const std::string remove_commas(std::string s){
    for(unsigned int i=0;i<s.length();++i){
      if(s[i] == ',') s[i] = ' ';
    }
    return s;
  }
  
  static void create_property_gui(GUI &gui,GenericGrabber &grabber, GUI::CallbackPtr cb,  
                                  std::vector<SmartPtr<VolatileUpdater> > &timers){
    gui = GUI("vbox");
    std::ostringstream ostr;
    std::vector<std::string> propertyList = grabber.getPropertyList();
    
    
    for(unsigned int i=0;i<propertyList.size();++i){
      const std::string &p = propertyList[i];
      const std::string pp = remove_commas(p);
      if(p == "size" || p == "format") continue; // these two are handled elsewhere
      
      std::string pt = grabber.getType(p);

      //      std::cout << "property " << i << " is " << p << " (type is " << pt << ")" << std::endl; 
      
      if(pt == "range"){
        // todo check stepping ...
        std::string handle="#r#"+p;
        SteppingRange<float> r = parse<SteppingRange<float> >(grabber.getInfo(p));
        std::string c = grabber.getValue(p);
        gui << "fslider("+str(r.minVal)+","+str(r.maxVal)+","+c+")[@handle="+handle+"@minsize=12x2@label="+pp+"]";
        ostr << '\1' << handle;
      }else if(pt == "menu" || pt == "value-list" || pt == "valueList"){
        std::string handle = (pt == "menu" ? "#m#" : "#v#")+p;
        gui << "combo("+get_combo_content_str(p,grabber)+")[@handle="+handle+"@minsize=12x2@label="+pp+"]";
        ostr << '\1' << handle;
      }else if(pt == "command"){
        std::string handle = "#c#"+p;
        ostr << '\1' << handle;
        gui << "button("+pp+")[@handle="+handle+"@minsize=12x2]";
      }else if(pt == "info"){
        std::string handle = "#i#"+p;
        ostr << '\1' << handle;
        gui << "label("+grabber.getValue(p)+")[@handle="+handle+"@minsize=12x2@label="+p+"]";
        int volatileness = grabber.isVolatile(p);
        if(volatileness){
          timers.push_back(new VolatileUpdater(volatileness,p,gui,grabber));
        }
      }else{
        ERROR_LOG("unable to create GUI-component for property \"" << p << "\" (unsupported property type: \"" + pt+ "\")");
      }
    }
  
    gui.show();
    
    std::string cblist = ostr.str();
    if(cblist.size() > 1){
      gui.registerCallback(cb,cblist.substr(1),'\1');
    }
    for(unsigned int i=0;i<timers.size();++i){
      timers[i]->start();
    }
  }
  
  static void process_callback(char t, const std::string &property, GenericGrabber &grabber, GUI &gui){
    switch(t){
      case 'r': 
      case 'm': 
      case 'v': 
        grabber.setProperty(property,gui[str("#")+t+"#"+property]);
        break;
      case 'c': 
        grabber.setProperty(property,"");
        break;
      default:
        ERROR_LOG("invalid callback ID type char: \"" << t << "\"");
    }
  }
  
  void CamCfgWidget::exec(const std::string &source){
    QMutexLocker __lock(&data->mutex);

    if(source.length()>3 && source[0] == '#'){
      process_callback(source[1],source.substr(3),data->grabber,data->propGUI);
    }else if(source == "scan"){
      scan();
    }else if(data->scanScope || data->settingUpDevice){
      return;
    }else if(source == "device"){
      data->settingUpDevice = true;
      QWidget *w = data->scroll->widget();
      
      if(w){
        w->setVisible(false);
        data->scroll->setWidget(0);
        for(unsigned int i=0;i<data->timers.size();++i){
          data->timers[i]->stop();
        }
        data->timers.clear();
        delete w;
      }

      if(data->complex){
        ComboHandle &fmt = data->gui.getValue<ComboHandle>("format");
        ComboHandle &siz = data->gui.getValue<ComboHandle>("size");
        fmt.clear();
        siz.clear();
      }
      
      try{
        if(data->complex || data->foundDevices.size() > 1){
          if(!data->loadParamsScope){
            data->grabber.init(data->foundDevices.at((int)data->gui["device"]));
            if(data->grabber.isNull()){
              ERROR_LOG("unable to initialize grabber");
            }
          }
          if(data->complex){
            data->grabber.setUseDesiredParams(data->gui.getValue<CheckBoxHandle>("desired-use").isChecked());
          }
        }
        
        create_property_gui(data->propGUI,data->grabber,SmartPtr<GUI::Callback>(this,false), data->timers);
        data->scroll->setWidget(data->propGUI.getRootWidget());

        if(data->complex){
          ComboHandle &fmt = data->gui.getValue<ComboHandle>("format");
          ComboHandle &siz = data->gui.getValue<ComboHandle>("size");
        
          std::vector<std::string> fmts = tok(strip(data->grabber.getInfo("format"),'{','}'),",");
          std::vector<std::string> sizes = tok(strip(data->grabber.getInfo("size"),'{','}'),",");
          
          for(unsigned int i=0;i<fmts.size();++i){
            fmt.add(strip(fmts[i],'"','"'));
          }
          
          for(unsigned int i=0;i<sizes.size();++i){
            siz.add(strip(sizes[i],'"','"'));
          }
        }
        
      }catch(const std::exception &x){
        ComboHandle &fmt = data->gui.getValue<ComboHandle>("format");
        ComboHandle &siz = data->gui.getValue<ComboHandle>("size");

        fmt.add("no devices found");
        siz.add("no devices found");
      }
      
      // fill format and size combos
      data->settingUpDevice = false;
      
      if(data->complex){
        exec("desired-params");
        exec("desired-format");
        exec("desired-size");
        exec("desired-depth");
      }
    }else if(source == "hz"){
      std::string v = data->gui["hz"];
      if(v.length() < 5) {
        ERROR_LOG("invalid FPS-value [this should not happen]");
      }else{
        if(v == "no limit"){
          data->useFPSLimiter = false;
        }else{
          v = v.substr(4,v.length()-6);
          data->useFPSLimiter = true;
          data->fpsLimiter.setMaxFPS(parse<int>(v));
        }
      }
    }else if(source == "format" || source == "size"){
      if(data->grabber.isNull()) return;
      data->grabber.setProperty(source,data->gui[source]);
    }else if(source == "desired-use"){
      if(data->grabber.isNull()) return;
      data->grabber.setUseDesiredParams(data->gui.getValue<CheckBoxHandle>(source).isChecked());
    }else if(source == "desired-format"){
      if(data->grabber.isNull()) return;
      data->grabber.setDesiredFormat(parse<format>(data->gui[source]));
    }else if(source == "desired-size"){
      if(data->grabber.isNull()) return;
      data->grabber.setDesiredSize(parse<Size>(data->gui[source]));
    }else if(source == "desired-depth"){
      if(data->grabber.isNull()) return;
      data->grabber.setDesiredDepth(parse<icl::depth>(data->gui[source]));    
    }else if(source == "save"){
      if(data->grabber.isNull()){
        QMessageBox::information(this,"No device selected!","You can only save properties if a device is selected");
        return;
      }
      QString s = QFileDialog::getSaveFileName(this,"Save device properties ...","","XML-files (*.xml)");
      if(s.isNull() || s=="")return;
      try{
        data->grabber.saveProperties(s.toLatin1().data(),false);
      }catch(ICLException &e){
        ERROR_LOG(e.what());
      }
    }else if(source == "load"){
      if(data->grabber.isNull()){
        QMessageBox::information(this,"No device selected!","You can only load properties if a device is selected");
        return;
      }
      QString s = QFileDialog::getOpenFileName(this,"Load device properties ...","","XML-files (*.xml)");
      if(s.isNull() || s == "") return;
      data->grabber.loadProperties(s.toLatin1().data(),false);
      data->loadParamsScope = true;
      exec("device");
      data->loadParamsScope = false;
    }
  }

  void CamCfgWidget::scan(){
    data->scanScope = true;
    
    ComboHandle &devices = data->gui.getValue<ComboHandle>("device");
    devices.clear();
    data->foundDevices = GenericGrabber::getDeviceList(data->deviceFilter);
    if(data->foundDevices.size()){
      for(unsigned int i=0;i<data->foundDevices.size();++i){
        devices.add("[" + data->foundDevices[i].type + "]:" + data->foundDevices[i].description);
      }
    }else{
      devices.add("no devices found");
    }
    data->scanScope = false;
    exec("device");
  }

  void CamCfgWidget::run(){
    bool &b = data->gui.getValue<bool>("grabbing");
    data->mutex.lock();
    while(1){
      if(data->end){
        break;
      }
      while(!b){
        data->gui["fps"] = str("--.--");
        data->grabbing = false;
        data->mutex.unlock();
        Thread::msleep(100);
        data->mutex.lock();
      }
      data->grabbing = true;

      if(!data->grabber.isNull()){
        data->gui["image"] = data->grabber.grab();
        data->gui["image"].update();
        data->gui["fps"] = data->fps.getFPSString();
        data->mutex.unlock();
        if(data->useFPSLimiter){
          data->fpsLimiter.wait();
        }
        Thread::msleep(0);
        data->mutex.lock();
      }else{
        data->mutex.unlock();
        Thread::msleep(100);
        data->mutex.lock();
      }
      
    }
  }
}


