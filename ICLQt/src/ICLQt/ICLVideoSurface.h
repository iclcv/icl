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

#pragma once

#include <QtCore>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))

#include <QtMultimedia/QVideoSurfaceFormat>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QFileInfo>
#include <ICLQt/Common.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLCore/CCFunctions.h>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QAtomicInt>
//#include <boost/thread/mutex.hpp>
//#include <boost/thread/condition_variable.hpp>
//#include <boost/atomic.hpp>

namespace icl{
  namespace qt{

    class ICLQt_API ICLVideoSurface : public QAbstractVideoSurface
    {
      Mutex lock;
      bool newImage;
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
      //boost::mutex waitMutex;                   // QMutex then ...
      //boost::condition_variable waitCondition;  // !QWaitCondition (maybe?)
      //boost::atomic<bool> useLocking;           // !use QAtomicInt

    public:
      ICLVideoSurface();
      ~ICLVideoSurface();
      QList<QVideoFrame::PixelFormat> supportedPixelFormats(
          QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;

      bool start(const QVideoSurfaceFormat &format);

      void stop();

      bool present(const QVideoFrame &frame);

      const Img8u* getImage();
    private:
      void init();
    };
  }
}

#endif
