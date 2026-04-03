// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLIO/DC.h>
#include <ICLIO/DCDeviceOptions.h>

#include <queue>
#include <mutex>

namespace icl::io {
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
      std::recursive_mutex mutex;

      /// associated camera
      dc1394camera_t *m_poCam;

      /// count of internal used DMA buffers
      int m_iBuffers;

      /// count of queued buffers
      int m_iQueuedBuffers;
    };
  }
  } // namespace icl::io