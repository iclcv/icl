// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#pragma once

#include <QtMultimedia/QCamera>
#include <QtMultimedia/QMediaCaptureSession>
#include <icl/qt/Common.h>
#include <icl/qt/ICLVideoSurface.h>
#include <icl/io/source/SourceBackend.h>

namespace icl::qt {
    class ICLQt_API QtCameraSource: public icl::io::SourceBackend{
      public:

        /// Create Camera grabber with given device id or name
        QtCameraSource(const std::string &device="0");

        /// Destructor
        ~QtCameraSource();

        /// grab function
        core::Image acquireImage() override;

      protected:
        QCamera* cam;
        QMediaCaptureSession* captureSession;
        ICLVideoSurface* surface;
    };
  }