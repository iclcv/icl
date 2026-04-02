// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <ICLQt/CamCfgWidget.h>

#include <ICLIO/GenericGrabber.h>
#include <ICLQt/ContainerGUIComponents.h>
#include <ICLQt/GUIWidget.h>
#include <ICLQt/ComboHandle.h>
#include <ICLQt/BoxHandle.h>
#include <ICLQt/LabelHandle.h>
#include <ICLQt/CheckBoxHandle.h>
#include <ICLQt/ButtonHandle.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLUtils/Thread.h>

#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QtCore/QMutex>
#include <QPushButton>


using namespace icl::utils;
using namespace icl::core;
using namespace icl::io;

namespace icl::qt {
    class CamCfgWidget::Data{
    public:
      bool complex;
      std::string deviceFilter;
      GUI gui;
      GUI properties;
      GenericGrabber grabber;
      std::vector<GrabberDeviceDescription> foundDevices;
      bool scanScope;
      bool settingUpDevice;
      bool grabbing;
      bool loadParamsScope;


      Data(bool complex):complex(complex),fps(5),fpsLimiter(10,10){
        scanScope = false;
        settingUpDevice = false;
        grabbing = false;
        useFPSLimiter = false;
        end = false;
        loadParamsScope = false;
      }

      QScrollArea *scroll;
      GUI propGUI; // contains the dataStore ...
      QRecursiveMutex mutex;

      FPSEstimator fps;
      FPSLimiter fpsLimiter;
      bool useFPSLimiter;
      bool end;
    };

    CamCfgWidget::CamCfgWidget(const std::string &deviceFilter,QWidget *parent):
      QWidget(parent), data(new Data(true)){
      data->deviceFilter = deviceFilter;

      data->gui = VSplit(this);
      data->gui <<  (HSplit()
                      << Display().handle("image").minSize(10,14).label("preview")
                      )
                << ( HSplit().maxSize(100,15)
                    << ( HBox().label("devices")
                         << Combo("no devices found").handle("device").minSize(10,2)
                         << Button("rescan").handle("scan").maxSize(3,8)
                       )
                    <<  ( HBox().label("control / FPS")
                          << Button("capture!","stop").handle("capture").out("grabbing")
                          << Combo("max 1Hz,max 5Hz,max 10Hz,max 15Hz,max 20Hz,max 25Hz,max 30Hz,max 50Hz,max 100Hz,max 120Hz,!no limit").handle("hz").minSize(5,2).maxSize(5,2)
                          << Label("--.--").handle("fps")
                        )
                  );

      data->gui << VBox().handle("props").minSize(10,18).label("Camera properties");
      data->gui.create();

      setLayout(new QBoxLayout(QBoxLayout::LeftToRight,this));
      layout()->setContentsMargins(2,2,2,2);
      layout()->addWidget(data->gui.getRootWidget());

      data->gui.registerCallback([this](const std::string &handle) { callback(handle); },"device,scan,capture,fps,hz");

      scan();
    }

    const ImgBase *CamCfgWidget::getCurrentDisplay(){
      QMutexLocker __lock(&data->mutex);
      if(data->grabber.isNull()) return 0;

      Image image = data->grabber.grabImage();

      if(data->complex){
        data->gui["image"] = image;
        data->gui["fps"] = data->fps.getFPSString();
      }

      return image.ptr();
    }


    CamCfgWidget::~CamCfgWidget(){
      if(data->complex){
        data->end = true;
      }
      ICL_DELETE(data);
    }

    void CamCfgWidget::setVisible (bool visible){
      if(!visible){
        data->grabbing = false;
      }
      QWidget::setVisible(visible);
    }

    void CamCfgWidget::callback(const std::string &source){
      QMutexLocker __lock(&data->mutex);

      if(source == "scan"){
        scan();
      }else if(data->scanScope || data->settingUpDevice){
        return;
      }else if(source == "device"){
        data->settingUpDevice = true;
        if(data->properties.hasBeenCreated()) data->properties.hide();

        try{
          if(data->foundDevices.size() == 1){
            data->grabber.init(data->foundDevices[0]);
            if(data->grabber.isNull()){
              throw ICLException("unable to create device");
            }
          }else if(data->foundDevices.size() > 1){
            data->grabber.init(data->foundDevices.at(static_cast<int>(data->gui["device"])));
            if(data->grabber.isNull()){
              throw ICLException("unable to create device");
            }
          }
          BoxHandle b = data->gui.get<BoxHandle>("props");
          data -> properties = Prop(data->foundDevices.at(static_cast<int>(data->gui["device"])).name()).handle("camcfg");
          data -> properties.show();
          b.add(data->properties.getRootWidget());
        } catch(const std::exception &x){
          DEBUG_LOG("error while initializing grabber: " << x.what());
        }
        data->settingUpDevice = false;
      }else if(source == "hz"){
        std::string v = data->gui["hz"].as<std::string>();
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
      }
    }

    void CamCfgWidget::scan(){
      data->scanScope = true;

      ComboHandle &devices = data->gui.get<ComboHandle>("device");
      devices.clear();
      data->foundDevices = GenericGrabber::getDeviceList(data->deviceFilter);

      if(data->foundDevices.size()){
        for(unsigned int i=0;i<data->foundDevices.size();++i){
          devices.add(data->foundDevices[i].name());
        }
      }else{
        devices.add("no devices found");
      }
      data->scanScope = false;
      callback("device");
    }

    void CamCfgWidget::update(){
      data->mutex.lock();
      bool &b = data->gui.get<bool>("grabbing");
      if(!b){
        data->gui["fps"] = str("--.--");
        data->grabbing = false;
        data->mutex.unlock();
        Thread::msleep(100);
        return;
      } else {
        data->grabbing = true;

        if(!data->grabber.isNull()){
          data->gui["image"] = data->grabber.grabImage();
          data->gui["fps"] = data->fps.getFPSString();
          data->mutex.unlock();
          if(data->useFPSLimiter){
            data->fpsLimiter.wait();
          }
          Thread::msleep(0);
        }else{
          data->mutex.unlock();
          Thread::msleep(100);
        }
      }
    }
  } // namespace icl::qt