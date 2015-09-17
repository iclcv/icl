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

#include <QtMultimedia/QMediaPlayer>
#include <QFileInfo>
#include <ICLQt/Common.h>
#include <ICLQt/ICLVideoSurface.h>

namespace icl{
  namespace qt{
    class ICLQt_API QtVideoGrabber: public Grabber{
      public:

        /// Create video grabber with given video-file name
        QtVideoGrabber(const std::string &filename) throw (utils::FileNotFoundException,utils::InvalidFileException);

        /// Destructor
        ~QtVideoGrabber();

        /// grab function
        virtual const core::ImgBase *acquireImage();

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
#endif
