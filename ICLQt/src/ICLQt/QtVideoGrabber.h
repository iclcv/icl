// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#pragma once

#include <QtMultimedia/QMediaPlayer>
#include <QFileInfo>
#include <ICLQt/Common.h>
#include <ICLQt/ICLVideoSurface.h>

namespace icl::qt {
    class ICLQt_API QtVideoGrabber: public icl::io::Grabber{
      public:

        /// Create video grabber with given video-file name
        QtVideoGrabber(const std::string &filename);

        /// Destructor
        ~QtVideoGrabber();

        /// grab function
        virtual const core::ImgBase *acquireDisplay();

        /// direct access to pause video playback (grab will block then)
        void pause();

        /// direct access to unpause video playback
        void unpause();

        /// direct access to restart video playback from the first frame
        void restart();

      protected:
        QMediaPlayer* player;
        ICLVideoSurface* surface;
    };
  }