// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#pragma once

#include <QtMultimedia/QCamera>
#include <QtMultimedia/QMediaCaptureSession>
#include <ICLQt/Common.h>
#include <ICLQt/ICLVideoSurface.h>

namespace icl{
  namespace qt{
    class ICLQt_API QtCameraGrabber: public icl::io::Grabber{
      public:

        /// Create Camera grabber with given device id or name
        QtCameraGrabber(const std::string &device="0");

        /// Destructor
        ~QtCameraGrabber();

        /// grab function
        virtual const core::ImgBase *acquireDisplay();

      protected:
        QCamera* cam;
        QMediaCaptureSession* captureSession;
        ICLVideoSurface* surface;
    };
  }
}
