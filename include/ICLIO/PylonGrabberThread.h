/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/PylonGrabberThread.h                        **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_PYLON_GRABBER_THREAD_H
#define ICL_PYLON_GRABBER_THREAD_H

#include <ICLUtils/Thread.h>
#include <ICLIO/PylonUtils.h>
#include <ICLIO/PylonColorConverter.h>
#include <ICLIO/PylonCameraOptions.h>

#include <queue>

namespace icl {
  namespace pylon {

    /// Internally spawned thread class for continuous grabbing \ingroup GIGE_G
    class PylonGrabberThread : public Thread {
      public:
        /// Constructor sets all internal fields and allocates memory
        /**
        * @param grabber The IStreamGrabber providing the images.
        * @param camMutex The Camera mutex
        * @param bufferCount The number of buffers the Grabber should queue.
        * @param bufferSize The size a buffer needs to hold a single image.
        */
        PylonGrabberThread(Pylon::IStreamGrabber* grabber,
                                PylonColorConverter* converter,
                                PylonCameraOptions* options);
        /// Destructor frees all allocated memory
        ~PylonGrabberThread();
        /// acquires images and writes them into an internal queue
        void run();
        /// reinitializes buffer
        /**
        * @param bufferSize The size a buffer needs to hold a single image.
        */
        void resetBuffer();
        /// getter for the most current image
        /**
        * @return a pointer to an internally used TsBuffer the buffer
        *         can safely be used until the next call to
        *         getCurrentImage() or resetBuffer().
        */
        ImgBase* getCurrentImage();
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
        /// used for framerate preserving.
        double m_ResultingFramerate;

        /// grabs a single image into m_BufferQueue.
        void grab();
  };

  }//namespace pylon
}//namespace icl
#endif
