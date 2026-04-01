// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#include <ICLQt/QtVideoGrabber.h>

using namespace icl::utils;
using namespace icl::io;

namespace icl{
  namespace qt{
    QtVideoGrabber::QtVideoGrabber(const std::string &filename) {

      if(!File(filename).exists()){
        throw FileNotFoundException(filename);
      }

      surface = new ICLVideoSurface;
      player = new QMediaPlayer;
      player->setVideoSink(surface->videoSink());
      player->setSource(QUrl::fromLocalFile(QFileInfo(QString(filename.c_str())).absoluteFilePath()));
      player->play();
    }

    QtVideoGrabber::~QtVideoGrabber() {
      delete player;
      delete surface;
    }

    const core::ImgBase *QtVideoGrabber::acquireDisplay() {
      return surface->getDisplay();
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

    Grabber* createQtVideoGrabber(const std::string &param){
      return new QtVideoGrabber(param);
    }

    const std::vector<GrabberDeviceDescription>& getQtVideoDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(!rescan) return deviceList;

      deviceList.clear();
      if(hint.size()) deviceList.push_back(
        GrabberDeviceDescription("qtvideo", hint, "A grabber video files.")
        );
      return deviceList;
    }

    REGISTER_GRABBER(qtvideo,createQtVideoGrabber, getQtVideoDeviceList,"qtvideo:video filename:Qt based video file source");
  }
}
