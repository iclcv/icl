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


#include <ICLQt/QtCameraGrabber.h>
#include <QtMultimedia/QCameraInfo>
#include <ICLUtils/StringUtils.h>
#include <QtMultimedia/QCameraExposure>
#include <QtMultimedia/QCameraImageProcessing>

namespace icl{
  namespace qt{
    QtCameraGrabber::QtCameraGrabber(const std::string &deviceIn) :cam(0),surface(0){
      surface = new ICLVideoSurface;
      std::vector<std::string> deviceTokens = tok(deviceIn,"|||",false);
      if(!deviceTokens.size()){
        throw ICLException("Could not create device from empty QtCamera device id");
      }
      std::string device = deviceTokens[0];
      QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
      if(!cameras.size()){
        throw ICLException("QtCamera device id " +str(device) +
                           " could not be instantiated (no supported cameras were found)");
      }
      bool isInt = false;
      int deviceIndex = QString::fromLatin1(device.c_str()).toInt(&isInt);
      if(isInt){
        if(deviceIndex>=cameras.length()){
          throw ICLException("QtCamera device index " +str(deviceIndex) +
                             " out of range (possible range: [" + str(0) + "," + str(cameras.size()-1) +"])");
        }
      }else{
        deviceIndex = -1;
        std::vector<std::string> allCams;
        for(int i=0;i<cameras.size();++i){
          allCams.push_back(cameras[i].description().toLatin1().data());
          if(allCams.back() == device){
            deviceIndex = i;
          }
        }
        if(deviceIndex == -1){
          throw ICLException("QtCamera device name " + device
                             + " not found (found devices were " + cat(allCams) + ")");
        }
      }

    
      cam = new QCamera(cameras[deviceIndex]);
      cam->setViewfinder(surface);
      cam->start();

      
      addProperty("format", "menu", "{default},","default",0,"Sets the cameras image size and format");
      addProperty("size", "menu", "adjusted by format","adjusted by format", 0,"this is set by format");

      //QList<QSize> sizes = cam->supportedViewfinderResolutions(); needs qt 5.5
      //for(int i=0;i<sizes.size();++i){
      //  DEBUG_LOG(sizes[i].width());
      //  DEBUG_LOG(sizes[i].height());
      //}
      // todo add properties format, and size
    }

    QtCameraGrabber::~QtCameraGrabber() {
      
      /* deletion leads to issues
          if(cam){
          cam->stop();
          delete cam;
          cam = 0;
          }
      */
      //ICL_DELETE(surface);
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
        QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
        for(int i=0;i<cameras.size();++i){
          deviceList.push_back(GrabberDeviceDescription("qtcam",
                                                        str(i)+"|||"+cameras[i].deviceName().toLatin1().data(),
                                                        str("Qt camera source ") +
                                                        cameras[i].description().toLatin1().data()));
        }
      }
      return deviceList;
    }

    REGISTER_GRABBER(qtcam,utils::function(createQtCameraGrabber), utils::function(getQtCameraDeviceList),"qtcam:video filename:Qt based Camera source");
  }
}
