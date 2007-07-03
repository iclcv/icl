#ifndef ICL_DC_FRAME_QUEUE_H
#define ICL_DC_FRAME_QUEUE_H

#include "iclDC.h"

#include <iclMutex.h>
#include <queue>

namespace icl{
  namespace dc{
    
    class DCFrameQueue{
      public:
      DCFrameQueue(dc1394camera_t* c,int nDMABuffers=5, int nQueuedBuffers=2);

      ~DCFrameQueue();
      
      void step();

      inline dc1394video_frame_t *front() { return q.front(); }
      inline dc1394video_frame_t *back() { return q.back(); }
      
      inline void lock() { mutex.lock(); }
      inline void unlock() { mutex.unlock(); }
      
      void push(dc1394video_frame_t *f);
      
      dc1394video_frame_t *pop();
      
      void showDetails() const;
      
      private:
      std::queue<dc1394video_frame_t*> q;
      Mutex mutex;
      dc1394camera_t *m_poCam;
      int m_iBuffers;
      int m_iQueuedBuffers;
      
    };
  }
}

#endif
