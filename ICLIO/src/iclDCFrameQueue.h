#ifndef ICL_DC_FRAME_QUEUE_H
#define ICL_DC_FRAME_QUEUE_H

#include "iclDC.h"
#include "iclDCDeviceOptions.h"

#include <iclMutex.h>
#include <queue>

namespace icl{
  
  namespace dc{
    
    /// Internal used utility class for fast handling of DMA buffers
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
      DCFrameQueue(dc1394camera_t* c, DCDeviceOptions *options, int nDMABuffers=5, int nQueuedBuffers=1);

      /// releases the internal camera using icl::dc::release_dc_cam()
      ~DCFrameQueue();
      
      /// performs a new step
      /** - enqueue the oldest frame front() in the DMA-queue
            and pop() it from the internal queue
          - dequeue a frame F ( this is the newest one now)
          - push F into the internal frame
      */
      void step();

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
      void push(dc1394video_frame_t *f);
      
      /// pops the oldest frame from the front of the internal queue (auto lock()/unlock())
      /** The function internally locks the queue */
      dc1394video_frame_t *pop();
      
      /// shows some debugging information for the wrapped camera
      void showDetails() const;
      
      private:
      /// wrapped queue structure
      std::queue<dc1394video_frame_t*> q;
      
      /// queue mutex
      Mutex mutex;
      
      /// associated camera
      dc1394camera_t *m_poCam;
      
      /// count of internal used DMA buffers
      int m_iBuffers;
      
      /// count of queued buffers
      int m_iQueuedBuffers;
      
    };
  }
}

#endif
