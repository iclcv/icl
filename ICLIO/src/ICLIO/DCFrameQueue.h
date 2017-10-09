/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/DCFrameQueue.h                         **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <ICLIO/DC.h>
#include <ICLIO/DCDeviceOptions.h>

#include <ICLUtils/Mutex.h>
#include <queue>

namespace icl{
  namespace io{

    namespace dc{

      /// Internal used utility class for fast handling of DMA buffers \ingroup DC_G
      /** This Queue implementation initializes the given camera. Then it dequeues
          all DMA ring buffer frames once, and pushes them into its wrapped
          std::queue<dc1394video_frame_t*>. After that, it re-enqueues a fixed number
          (given by the 3rd constructor argument nQueuedBuffers which is set to 2
          by default) into the DMA-queue. This buffers are filled continuously with new frame data
          by the DMA-Thread.
          After this initialization, the Queue is used by the DCGrabberThread
          to buffer grabbed image data as fast a possible, and to offer the newest frame
          safely at any time to other threads.
      **/
      class DCFrameQueue{
        public:
        /// Creates a new DCFrameQueue objects with given camera, DMABuffer size and count of queued buffers
        ICLIO_API DCFrameQueue(dc1394camera_t* c, DCDeviceOptions *options,int nDMABuffers=5, int nQueuedBuffers=1);

        /// releases the internal camera using icl::dc::release_dc_cam()
        ICLIO_API ~DCFrameQueue();

        /// performs a new step
        /** - enqueue the oldest frame front() in the DMA-queue
              and pop() it from the internal queue
            - dequeue a frame F ( this is the newest one now)
            - push F into the internal frame
        */
        ICLIO_API void step();

        /// returns the oldest frame in the queue
        /** new frames are pushed from the back.
            call lock() / unlock() before and after using this
            function to ensure, that the returned element remains
            the front one and that it stay in the queue
        */
        inline dc1394video_frame_t *front() { return q.front(); }

        /// returns the newest frame in the queue
        /** call lock() / unlock() before and after using this
            function to ensure, that the returned element remains
            the newest one and that is stays in the loop */
        inline dc1394video_frame_t *back() { return q.back(); }

        /// locks the queue
        inline void lock() { mutex.lock(); }

        /// unlocks the queue
        inline void unlock() { mutex.unlock(); }

        /// pushes a new frame to the back of the internal queue (auto lock()/unlock())
        /** The function internally locks the queue */
        ICLIO_API void push(dc1394video_frame_t *f);

        /// pops the oldest frame from the front of the internal queue (auto lock()/unlock())
        /** The function internally locks the queue */
        ICLIO_API dc1394video_frame_t *pop();

        /// shows some debugging information for the wrapped camera
        ICLIO_API void showDetails() const;

        /// calls dc1394_reset_bus  (not save!)
        ICLIO_API void resetBus();

        private:
        /// wrapped queue structure
        std::queue<dc1394video_frame_t*> q;

        /// queue mutex
        utils::Mutex mutex;

        /// associated camera
        dc1394camera_t *m_poCam;

        /// count of internal used DMA buffers
        int m_iBuffers;

        /// count of queued buffers
        int m_iQueuedBuffers;
      };
    }
  } // namespace io
}

