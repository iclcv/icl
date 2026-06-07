// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#include <icl/qt/QtCameraSource.h>
#include <icl/utils/prop/Constraints.h>
#include <QtMultimedia/QMediaDevices>
#include <QtMultimedia/QCameraDevice>
#include <icl/utils/StringUtils.h>
#include <QCoreApplication>
#include <QPermission>

using namespace icl::utils;
using namespace icl::io;

namespace icl::qt {
    QtCameraSource::QtCameraSource(const std::string &deviceIn)
      : cam(0), captureSession(0), surface(0)
    {
      // Qt6 on macOS: check camera permission
      // Note: Qt6's permission plugin requires .app bundles on macOS.
      // For CLI tools, the terminal app must have camera access granted
      // in System Settings > Privacy & Security > Camera.
      QCameraPermission cameraPermission;
      auto permStatus = qApp->checkPermission(cameraPermission);
      if(permStatus == Qt::PermissionStatus::Denied){
        throw ICLException("Camera permission denied. Grant camera access to your terminal app "
                           "in System Settings > Privacy & Security > Camera. "
                           "Alternatively, use the OpenCV backend: -i cvcam 0");
      }
      if(permStatus == Qt::PermissionStatus::Undetermined){
        // For CLI tools, try to request but it may not show a dialog
        qApp->requestPermission(cameraPermission, [](const QPermission &){});
        QCoreApplication::processEvents();
      }

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

      addProperty("format", prop::Menu{"{default}"}, "default", "Sets the cameras image size and format");
      addProperty("size", prop::Menu{"adjusted by format"}, "adjusted by format", "this is set by format");
    }

    QtCameraSource::~QtCameraSource() {
      if(cam){
        cam->stop();
      }
      delete captureSession;
      delete cam;
      delete surface;
    }

    core::Image QtCameraSource::acquireImage() {
      const core::ImgBase *p = surface->getDisplay();
      return p ? core::Image(p->deepCopy()) : core::Image();
    }

    SourceBackend* createQtCameraGrabber(const std::string &param){
      return new QtCameraSource(param);
    }

    const std::vector<DeviceDescription>& getQtCameraDeviceList(std::string hint, bool rescan){
      static std::vector<DeviceDescription> deviceList;
      if(rescan){
        deviceList.clear();
        QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
        for(int i = 0; i < cameras.size(); ++i){
          deviceList.push_back(DeviceDescription("qtcam",
            str(i) + "|||" + cameras[i].id().constData(),
            str("Qt camera source ") + cameras[i].description().toLatin1().data()));
        }
      }
      return deviceList;
    }

    REGISTER_SOURCE_BACKEND(qtcam,createQtCameraGrabber, getQtCameraDeviceList,"device index or name~Qt based Camera source");
  }