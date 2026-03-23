/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2015 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ICLVideoSurface.h                      **
** Module : ICLQt                                                  **
** Authors: Matthias Esau                                          **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
********************************************************************/

#pragma once

#include <QObject>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>
#include <ICLQt/Common.h>
#include <ICLCore/CCFunctions.h>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QAtomicInt>
#include <mutex>

namespace icl{
  namespace qt{

    /// Receives video frames from Qt6 multimedia and converts to ICL images
    class ICLQt_API ICLVideoSurface : public QObject
    {
      Q_OBJECT

      std::recursive_mutex lock;
      core::Img8u img0;
      core::Img8u img1;
      core::Img8u img2;
      core::Img8u *imgWork;
      core::Img8u *imgDisplay;
      core::Img8u *imgNextDisplay;
      volatile bool nextFrameReady;
      QMutex waitMutex;
      QWaitCondition waitCondition;
      QAtomicInt useLocking;
      QVideoSink *m_sink;

    public:
      ICLVideoSurface();
      ~ICLVideoSurface();

      /// Returns the QVideoSink for connecting to QMediaPlayer or QMediaCaptureSession
      QVideoSink *videoSink() { return m_sink; }

      /// Blocking call: returns the most recent frame as an ICL image
      const core::Img8u* getImage();

      /// Stops receiving frames and wakes any blocked consumer
      void stop();

    private Q_SLOTS:
      void onVideoFrameChanged(const QVideoFrame &frame);

    private:
      void init();
    };
  }
}
