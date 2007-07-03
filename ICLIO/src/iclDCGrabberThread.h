#ifndef ICL_DC_GRABBER_THREAD_H
#define ICL_DC_GRABBER_THREAD_H

#include <iclDC.h>
#include <iclThread.h>
#include <iclTypes.h>


#include <vector>


namespace icl{
  namespace dc{
    
    class DCFrameQueue;
    
    class DCGrabberThread : public Thread{
      public:
      DCGrabberThread(dc1394camera_t* c);
      
      virtual void run();
      virtual void finalize();
      
      void getCurrentImage(ImgBase **ppoDst);
      
      private: 
      DCFrameQueue *m_poFrameQueue;
      dc1394camera_t* m_poCam;
      std::vector<icl8u> m_oRGBInterleavedBuffer;
    };
  }
}

#endif
