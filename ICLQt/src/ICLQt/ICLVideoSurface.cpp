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

#include <ICLQt/ICLVideoSurface.h>

namespace icl{
  namespace qt{
    ICLVideoSurface::ICLVideoSurface() :
        QAbstractVideoSurface()
    {
      init();
      useLocking.store(false);
    }
    ICLVideoSurface::~ICLVideoSurface(){
    }

    // Everything BELOW YUV420P is not natively supported!
    // You might experience lag.
    QList<QVideoFrame::PixelFormat> ICLVideoSurface::supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType) const
    {
      return QList<QVideoFrame::PixelFormat>()
          << QVideoFrame::Format_RGB24
          << QVideoFrame::Format_YUV420P
          << QVideoFrame::Format_ARGB32
          << QVideoFrame::Format_ARGB32_Premultiplied
          << QVideoFrame::Format_RGB32
          << QVideoFrame::Format_RGB565 
          << QVideoFrame::Format_RGB555;
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
      nextFrameReady=false;
    }

    bool ICLVideoSurface::start(const QVideoSurfaceFormat &format) {
      if ((format.pixelFormat() != QVideoFrame::Format_RGB24) ||
          (format.pixelFormat() != QVideoFrame::Format_RGB24)) {
        WARNING_LOG("Using non native conversion. Performance may suffer.")
      }
      QAbstractVideoSurface::start(format);
      init();
      useLocking.store(true);
      return true;
    }

    void ICLVideoSurface::stop() {
      QAbstractVideoSurface::stop();
      useLocking.store(false);
      waitCondition.notify_one();
    }

    bool ICLVideoSurface::present(const QVideoFrame &frame)
    {
        if (frame.isValid()) {
            QVideoFrame cloneFrame(frame);
            cloneFrame.map(QAbstractVideoBuffer::ReadWrite);
            lock.lock();
            std::swap(imgWork,imgNextDisplay);
            if(cloneFrame.pixelFormat() == QVideoFrame::Format_RGB24) {
                imgWork->setChannels(3);
                imgWork->setSize(utils::Size(cloneFrame.width(),cloneFrame.height()));
              core::interleavedToPlanar<uchar,icl8u>(cloneFrame.bits(),imgWork,cloneFrame.bytesPerLine());
            }
            else if(cloneFrame.pixelFormat() == QVideoFrame::Format_YUV420P) {
              imgWork->setChannels(3);
              imgWork->setSize(utils::Size(cloneFrame.bytesPerLine(0),cloneFrame.height()));
              core::convertYUV420ToRGB8(cloneFrame.bits(),utils::Size(cloneFrame.bytesPerLine(),cloneFrame.height()),imgWork);
              if(cloneFrame.bytesPerLine()!=cloneFrame.width()) {
                imgWork->setROI(utils::Point(0,0),utils::Size(cloneFrame.width(),cloneFrame.height()));
              }
              else
                imgWork->setFullROI();
            } else {
              const QImage tmpImg(cloneFrame.bits(),
                           cloneFrame.width(),
                           cloneFrame.height(),
                           QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat()));
              QVideoFrame tmp = QVideoFrame(tmpImg.convertToFormat(QImage::Format_RGB888));  
              tmp.map(QAbstractVideoBuffer::ReadWrite);
              cloneFrame = tmp;
              imgWork->setChannels(3);
              imgWork->setSize(utils::Size(cloneFrame.width(),cloneFrame.height()));
              core::interleavedToPlanar<uchar,icl8u>(cloneFrame.bits(),imgWork,cloneFrame.bytesPerLine());
            }
            if(useLocking.load()) {
              boost::mutex::scoped_lock waitLock(waitMutex);
              if(!nextFrameReady)
              {
                  nextFrameReady=true;
                  waitCondition.notify_one();
              }
            }
            nextFrameReady=true;
            lock.unlock();
            return true;
        }
        return false;
    }

    const Img8u* ICLVideoSurface::getImage(){
      if(useLocking.load()) {
        boost::mutex::scoped_lock waitLock(waitMutex);
        while(!nextFrameReady) {
            waitCondition.wait(waitLock);
        }
      }
      lock.lock();
      if(nextFrameReady) {
        std::swap(imgDisplay,imgWork);
        nextFrameReady=false;
      }
      lock.unlock();
      return imgDisplay;
    }
  }
}
