/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2015 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ICLVideoSurface.cpp                    **
** Module : ICLQt                                                  **
** Authors: Matthias Esau                                          **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
********************************************************************/

#include <ICLQt/ICLVideoSurface.h>
#include <QImage>

using namespace icl::core;
using namespace icl::utils;

namespace icl{
  namespace qt{
    ICLVideoSurface::ICLVideoSurface() : QObject() {
      init();
      m_sink = new QVideoSink(this);
      connect(m_sink, &QVideoSink::videoFrameChanged,
              this, &ICLVideoSurface::onVideoFrameChanged);
      useLocking.storeRelaxed(true);
    }

    ICLVideoSurface::~ICLVideoSurface(){
      stop();
    }

    void ICLVideoSurface::init() {
      img0.setChannels(3);
      img0.setSize(utils::Size(100,100));
      img1.setChannels(3);
      img1.setSize(utils::Size(100,100));
      img2.setChannels(3);
      img2.setSize(utils::Size(100,100));
      imgWork = &img0;
      imgDisplay = &img1;
      imgNextDisplay = &img2;
      nextFrameReady = false;
    }

    void ICLVideoSurface::stop() {
      useLocking.storeRelaxed(false);
      waitCondition.wakeOne();
    }

    void ICLVideoSurface::onVideoFrameChanged(const QVideoFrame &frame) {
      if (!frame.isValid()) return;

      QVideoFrame f(frame);
      f.map(QVideoFrame::ReadOnly);

      lock.lock();
      std::swap(imgWork, imgNextDisplay);

      // Use QVideoFrame::toImage() for universal format support,
      // then convert to RGB888 for interleavedToPlanar
      QImage img = f.toImage().convertToFormat(QImage::Format_RGB888);
      imgWork->setChannels(3);
      imgWork->setSize(utils::Size(img.width(), img.height()));
      interleavedToPlanar<uchar, icl8u>(img.constBits(), imgWork, img.bytesPerLine());

      f.unmap();

      if(useLocking.loadRelaxed()) {
        QMutexLocker waitLock(&waitMutex);
        if(!nextFrameReady) {
          nextFrameReady = true;
          waitCondition.wakeOne();
        }
      }
      nextFrameReady = true;
      lock.unlock();
    }

    const Img8u* ICLVideoSurface::getImage(){
      if(useLocking.loadRelaxed()) {
        QMutexLocker waitLock(&waitMutex);
        while(!nextFrameReady) {
          waitCondition.wait(&waitMutex);
        }
      }
      lock.lock();
      if(nextFrameReady) {
        std::swap(imgDisplay, imgWork);
        nextFrameReady = false;
      }
      lock.unlock();
      return imgDisplay;
    }
  }
}
