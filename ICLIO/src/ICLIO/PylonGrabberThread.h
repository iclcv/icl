// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Thread.h>
#include <ICLIO/PylonUtils.h>
#include <ICLIO/PylonColorConverter.h>
#include <ICLIO/PylonCameraOptions.h>

#include <queue>

namespace icl::io {
  namespace pylon {

    /// Internally spawned thread class for continuous grabbing \ingroup GIGE_G
    class PylonGrabberThread : public utils::Thread {
      public:
        /// Constructor sets all internal fields and allocates memory
        /**
        * @param grabber The IStreamGrabber providing the images.
        * @param converter
        * @param options
        */
        ICLIO_API PylonGrabberThread(Pylon::IStreamGrabber* grabber,
                                PylonColorConverter* converter,
                                PylonCameraOptions* options);
        /// Destructor frees all allocated memory
        ICLIO_API ~PylonGrabberThread();
        /// acquires images and writes them into an internal queue
        ICLIO_API void run();
        /// reinitializes buffer
        ICLIO_API void resetBuffer();
        /// getter for the most current image
        /**
        * @return a pointer to an internally used TsBuffer the buffer
        *         can safely be used until the next call to
        *         getCurrentDisplay() or resetBuffer().
        */
        ICLIO_API core::ImgBase* getCurrentDisplay();
      private:
        /// A pointer to the image-providing StreamGrabber.
        Pylon::IStreamGrabber* m_Grabber;
        /// A pointer to the ColorConverter.
        PylonColorConverter* m_Converter;
        /// A pointer to the CameraOptions.
        PylonCameraOptions* m_Options;
        /// A buffer holding read and write buffers
        ConcGrabberBuffer m_Buffers;
        /// A counter for acquisition errors.
        int m_Error;
        /// A counter for acquisition timeouts.
        int m_Timeout;
        /// A counter for correct acquisitions.
        int m_Acquired;

        /// grabs a single image into m_BufferQueue.
        void grab();
  };

  }//namespace pylon
  } // namespace icl::io