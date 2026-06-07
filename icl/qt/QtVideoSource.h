// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#pragma once

#include <QtMultimedia/QMediaPlayer>
#include <QFileInfo>
#include <icl/qt/Common.h>
#include <icl/qt/ICLVideoSurface.h>
#include <icl/io/detail/SourceBackend.h>

namespace icl::qt {
    class ICLQt_API QtVideoSource: public icl::io::SourceBackend{
      public:

        /// Create video grabber with given video-file name
        QtVideoSource(const std::string &filename);

        /// Destructor
        ~QtVideoSource();

        /// grab function
        core::Image acquireImage() override;

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