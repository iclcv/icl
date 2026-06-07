// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#include <icl/qt/QtVideoGrabber.h>

using namespace icl::utils;
using namespace icl::io;

namespace icl::qt {
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

    core::Image QtVideoGrabber::acquireImage() {
      const core::ImgBase *p = surface->getDisplay();
      return p ? core::Image(p->deepCopy()) : core::Image();
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

    SourceBackend* createQtVideoGrabber(const std::string &param){
      return new QtVideoGrabber(param);
    }

    const std::vector<DeviceDescription>& getQtVideoDeviceList(std::string hint, bool rescan){
      static std::vector<DeviceDescription> deviceList;
      if(!rescan) return deviceList;

      deviceList.clear();
      if(hint.size()) deviceList.push_back(
        DeviceDescription("qtvideo", hint, "A grabber video files.")
        );
      return deviceList;
    }

    REGISTER_SOURCE_BACKEND(qtvideo,createQtVideoGrabber, getQtVideoDeviceList,"qtvideo:video filename:Qt based video file source");
  }