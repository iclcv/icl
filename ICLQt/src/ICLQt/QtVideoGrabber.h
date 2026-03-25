/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2015 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/QtVideoGrabber.h                       **
** Module : ICLQt                                                  **
** Authors: Matthias Esau                                          **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
********************************************************************/

#pragma once

#include <QtMultimedia/QMediaPlayer>
#include <QFileInfo>
#include <ICLQt/Common.h>
#include <ICLQt/ICLVideoSurface.h>

namespace icl{
  namespace qt{
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
}
