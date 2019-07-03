/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2015 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/QtVideoGrabber.cpp                     **
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


#include <ICLQt/QtVideoGrabber.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))

namespace icl{
  namespace qt{
    QtVideoGrabber::QtVideoGrabber(const std::string &filename) {

      if(!File(filename).exists()){
        throw FileNotFoundException(filename);
      }

      surface = new ICLVideoSurface;
      player = new QMediaPlayer;
      player->setVideoOutput(surface);
      player->setMedia(QUrl::fromLocalFile(QFileInfo(QString(filename.c_str())).absoluteFilePath()));
      player->play();
    }

    QtVideoGrabber::~QtVideoGrabber() {
      delete player;
      delete surface;
    }

    const core::ImgBase *QtVideoGrabber::acquireImage() {
      return surface->getImage();
    }

    void QtVideoGrabber::pause() {
      player->pause();
    }

    void QtVideoGrabber::unpause() {
      player->play();
    }

    void QtVideoGrabber::restart() {
      player->setPosition(0);
    }

//    REGISTER_CONFIGURABLE(QtVideoGrabber, return new QtVideoGrabber(""));

    Grabber* createQtVideoGrabber(const std::string &param){
      return new QtVideoGrabber(param);
    }

    const std::vector<GrabberDeviceDescription>& getQtVideoDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(!rescan) return deviceList;

      deviceList.clear();
      // if filter exists, add grabber with filter
      if(hint.size()) deviceList.push_back(
        GrabberDeviceDescription("qtvideo", hint, "A grabber video files.")
        );
      return deviceList;
    }

    REGISTER_GRABBER(qtvideo,utils::function(createQtVideoGrabber), utils::function(getQtVideoDeviceList),"qtvideo:video filename:Qt based video file source");
  }
}

#endif
