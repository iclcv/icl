/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
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
        PylonGrabberThread(Pylon::IStreamGrabber* grabber, Mutex* camMutex,
                           int bufferCount=0, int bufferSize=0);
        /// Destructor frees all allocated memory
        ~PylonGrabberThread();
        /// acquires images and writes them into an internal queue
        void run();
        /// frees all previously allocated memory and reinitializes it
        /**
        * @param bufferCount The amount of buffer the GrabberThread should queue.
        * @param bufferSize The size a buffer needs to hold a single image.
        */
        void resetBuffer(int bufferSize, int bufferCount);
        /// getter for the most current image
        /**
        * @return a pointer to an internally used TsBuffer the buffer
        *         is removed from the grabbing-queue and can safely
        *         be used until the next call to getCurrentImage()
        *         or resetBuffer().
        */
        TsBuffer<int16_t>* getCurrentImage();
        /// The mutex used for internal buffer-monitoring.
        Mutex m_BufferMutex;
      private:
        /// A pointer to the image-providing StreamGrabber.
        Pylon::IStreamGrabber* m_Grabber;
        /// A pointer to the camera-mutex.
        Mutex* m_CamMutex;
        /// The size of a single image-buffer.
        int m_BufferSize;
        /// The number of buffers the queue holds.
        int m_BufferCount;
        /// A counter for acquisition errors.
        int m_Error;
        /// A counter for acquisition timeouts.
        int m_Timeout;
        /// A counter for correct acquisitions.
        int m_Acquired;
        /// A queue holding the image buffers.
        std::queue<TsBuffer<int16_t>*> m_BufferQueue;
        /// tells whether a new image is available since last getCurrentImage().
        bool m_NewAvail;

        /// Fills m_BufferQueue with newly allocated TsBuffers
        /**
        * not thread safe. always lock m_BufferMutex before calling this.
        * @throw ICLException when m_BufferQueue is not empty
        */
        void initBuffer();
        /// pops and deletes all buffers from m_BufferQueue.
        /**
        * not thread safe. always lock m_BufferMutex before calling this.
        */
        void clearBuffer();
        /// grabs a single image into m_BufferQueue.
        void grab();
  };

  }//namespace pylon
}//namespace icl
#endif
