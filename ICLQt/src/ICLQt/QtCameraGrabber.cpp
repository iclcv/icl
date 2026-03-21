/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2015 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/QtCameraGrabber.cpp                    **
** Module : ICLQt                                                  **
** Authors: Matthias Esau                                          **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
********************************************************************/

#include <ICLQt/QtCameraGrabber.h>
#include <QtMultimedia/QMediaDevices>
#include <QtMultimedia/QCameraDevice>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::io;

namespace icl{
  namespace qt{
    QtCameraGrabber::QtCameraGrabber(const std::string &deviceIn)
      : cam(0), captureSession(0), surface(0)
    {
      surface = new ICLVideoSurface;

      std::vector<std::string> deviceTokens = tok(deviceIn,"|||",false);
      if(!deviceTokens.size()){
        throw ICLException("Could not create device from empty QtCamera device id");
      }
      std::string device = deviceTokens[0];

      QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
      if(!cameras.size()){
        throw ICLException("QtCamera device id " + str(device) +
                           " could not be instantiated (no supported cameras were found)");
      }

      bool isInt = false;
      int deviceIndex = QString::fromLatin1(device.c_str()).toInt(&isInt);
      if(isInt){
        if(deviceIndex >= cameras.length()){
          throw ICLException("QtCamera device index " + str(deviceIndex) +
                             " out of range (possible range: [0," + str(cameras.size()-1) + "])");
        }
      }else{
        deviceIndex = -1;
        std::vector<std::string> allCams;
        for(int i = 0; i < cameras.size(); ++i){
          allCams.push_back(cameras[i].description().toLatin1().data());
          if(allCams.back() == device){
            deviceIndex = i;
          }
        }
        if(deviceIndex == -1){
          throw ICLException("QtCamera device id " + device
                             + " not found (found devices were " + cat(allCams) + ")");
        }
      }

      cam = new QCamera(cameras[deviceIndex]);

      // Qt6: wire camera → capture session → video sink
      captureSession = new QMediaCaptureSession;
      captureSession->setCamera(cam);
      captureSession->setVideoSink(surface->videoSink());

      cam->start();

      addProperty("format", "menu", "{default},","default",0,"Sets the cameras image size and format");
      addProperty("size", "menu", "adjusted by format","adjusted by format", 0,"this is set by format");
    }

    QtCameraGrabber::~QtCameraGrabber() {
      if(cam){
        cam->stop();
      }
      delete captureSession;
      delete cam;
      delete surface;
    }

    const core::ImgBase *QtCameraGrabber::acquireImage() {
      return surface->getImage();
    }

    Grabber* createQtCameraGrabber(const std::string &param){
      return new QtCameraGrabber(param);
    }

    const std::vector<GrabberDeviceDescription>& getQtCameraDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(rescan){
        deviceList.clear();
        QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
        for(int i = 0; i < cameras.size(); ++i){
          deviceList.push_back(GrabberDeviceDescription("qtcam",
            str(i) + "|||" + cameras[i].id().constData(),
            str("Qt camera source ") + cameras[i].description().toLatin1().data()));
        }
      }
      return deviceList;
    }

    REGISTER_GRABBER(qtcam,utils::function(createQtCameraGrabber), utils::function(getQtCameraDeviceList),"qtcam:device index or name:Qt based Camera source");
  }
}
