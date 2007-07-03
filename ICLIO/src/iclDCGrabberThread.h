#ifndef ICL_DC_GRABBER_THREAD_H
#define ICL_DC_GRABBER_THREAD_H

#include <iclDC.h>
#include <iclThread.h>
#include <iclTypes.h>


#include <vector>


namespace icl{

  /// internal used namespace for libdc1394 dependent help functions and classes
  namespace dc{
    
    /** \cond */
    class DCFrameQueue;
    /** \endcond */
    
    /// Internal spawned thread class to provide continoues grabbing without drop frames
    /** Each DCGrabber uses a DCGrabberThread instance, which continously queues and 
        enqueus frames. Each frame can be either inside of the DMA queue or inside of the
        DCGrabberThreads wrapped DCFrameQueue at on time. The following ASCII art should
        illustrate this:
        <pre>
        
        Example: using a 5-frame DMA ring buffer
        
        DMA-Queue           [ F1 ][ F2 ]
                                                         System-Space
        --------------------------------------------------------------
                                                         User-Space
        
                              <------------
        DCFrameQueue       [ F3 ][ F4 ][ F5 ] <-- new frames a pushed here
                             /\                   so the newest frame is always
                             |                    QUEUE.back()
                             |
                          old frames move more to the left, the leftest frame is 
                          then removed from the DCFrameQueue and enqeued into the
                          DMA-Queue, where it is filled with new frame data.
        
        </pre>
        
        
        
        
        */
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
