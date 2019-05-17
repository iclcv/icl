/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/VideoGrabber.h                         **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
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

#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>

namespace icl{
  namespace io{

    /// Xine-based Video Grabber (grabs most common image formats) \ingroup MOVIE_FILE_G
    /** The VideoGrabber implementation differs somehow from other Grabber implementations, as
        video playback is heavily constrained by the current video framerate. The
        VideoGrabber's property interface provides functions to set up the grabbers video playback
        speed.
        Internally video frames are obtained successively (using xine's 'experimental feature:
        framegrab_video_port').

        Currently, drop-frames are not allowed/possible, so if the grabber is too slow to reach the
        desired Video-framerate, playback speed is decreased implicitly

        \section KI Known Issues
        The video grabber does not really work very stable.
        * In particular when linked against
          the default xine build shipped with ubuntu, you will most likely get some seg-faults due
          to a known xine bug.
        * Video streams that have a stream-size whose width is not a multiple of 8, will not be
          displayed correctly due to some undocumented data allignment issues.
        * Some formats just cause seg-faults within 3rd party libs.
        * seeking using setProperty("stream-pos") does not work robustly, however
          it does work in the icl-video-player application

        \section ALT Alternative
        Please note, that the OpenCVVideoGrabber might be an alternative for you
     */
    class VideoGrabber : public Grabber{
      public:

        /// Create video grabber with given video-file name
        VideoGrabber(const std::string &fileName) ;

        /// Destructor
        ~VideoGrabber();

        /// grab function
        virtual const core::ImgBase *acquireImage();

        /// direct access to pause video playback (grab will block then)
        void pause();

        /// direct access to unpause video playback
        void unpause();

        /// direct access to restart video playback from the first frame
        void restart();

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);


      protected:
        /** \cond */
        struct Data;
        struct XineHandle;
        struct Params;
        /** \endcond */

        XineHandle *m_xine; //!< internal data belonging to the xine library
        Data *m_data;       //!< internal data
        Params *m_params;   //!< property values
    };

  } // namespace io
}

