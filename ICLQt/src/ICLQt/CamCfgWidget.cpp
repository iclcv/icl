/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/CamCfgWidget.cpp                       **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

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

namespace icl{
  namespace qt{

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
      QMutex mutex;

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
                      << Image().handle("image").minSize(10,14).label("preview")
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

      data->gui.registerCallback(function(this,&icl::qt::CamCfgWidget::callback),"device,scan,capture,fps,hz");

      scan();
    }

    const ImgBase *CamCfgWidget::getCurrentImage(){
      QMutexLocker __lock(&data->mutex);
      if(data->grabber.isNull()) return 0;

      const ImgBase *image = data->grabber.grab();

      if(data->complex){
        data->gui["image"] = image;
        data->gui["fps"] = data->fps.getFPSString();
      }

      return image;
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
            data->grabber.init(data->foundDevices.at((int)data->gui["device"]));
            if(data->grabber.isNull()){
              throw ICLException("unable to create device");
            }
          }
          BoxHandle b = data->gui.get<BoxHandle>("props");
          data -> properties = Prop(data->foundDevices.at((int)data->gui["device"]).name()).handle("camcfg");
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
          data->gui["image"] = data->grabber.grab();
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
  } // namespace qt
}
