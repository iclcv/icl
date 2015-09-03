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

namespace icl{
  namespace qt{
    QtCameraGrabber::QtCameraGrabber(const std::string &device) {
      surface = new ICLVideoSurface;

      QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
      int id = atoi(device.c_str());
      if(id>=cameras.length()){
          throw ICLException("QtCamera initialization unable to find video device given device ID:"+device);
      }
      cam = new QCamera(cameras[id]);
      cam->setViewfinder(surface);
      cam->start();
    }

    QtCameraGrabber::~QtCameraGrabber() {
      delete cam;
      delete surface;
    }

    const core::ImgBase *QtCameraGrabber::acquireImage() {
      return surface->getImage();
    }

//    REGISTER_CONFIGURABLE(QtCameraGrabber, return new QtCameraGrabber(""));

    Grabber* createQtCameraGrabber(const std::string &param){
      return new QtCameraGrabber(param);
    }

    const std::vector<GrabberDeviceDescription>& getQtCameraDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(!rescan) return deviceList;

      deviceList.clear();
      // if filter exists, add grabber with filter
      if(hint.size()) deviceList.push_back(
        GrabberDeviceDescription("qtcam", hint, "A grabber Camera files.")
        );
      return deviceList;
    }

    REGISTER_GRABBER(qtcam,utils::function(createQtCameraGrabber), utils::function(getQtCameraDeviceList),"qtcam:video filename:Qt based Camera source");
  }
}
