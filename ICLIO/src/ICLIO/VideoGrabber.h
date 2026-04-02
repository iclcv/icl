// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#pragma once

#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>

namespace icl::io {
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
        VideoGrabber(const std::string &fileName);

        /// Destructor
        ~VideoGrabber();

        /// grab function
        virtual const core::ImgBase *acquireDisplay();

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

  } // namespace icl::io