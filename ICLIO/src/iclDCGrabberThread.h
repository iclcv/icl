#ifndef ICL_DC_GRABBER_THREAD_H
#define ICL_DC_GRABBER_THREAD_H

#include <iclDC.h>
#include <iclThread.h>
#include <iclTypes.h>


#include <vector>


namespace icl{

  /** \cond */
  class DCGrabber;
  /** \endcond */
  
  /// internal used namespace for libdc1394 dependent help functions and classes
  namespace dc{
    
    /** \cond */
    class DCFrameQueue;
    /** \endcond */
    
    /// Internally spawned thread class to provide continuous grabbing without drop frames
    /** Each DCGrabber instance uses a DCGrabberThread, which continuously dequeues and 
        enqueus frames. Each frame can either be inside of the DMA queue or inside of the
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
        
        The DCGrabberThread continuously pops the oldest frame from its internal DCFrameQueue,
        and enques this frame into the DMA-Queue immediately. Then, it waits for the next frame
        that was filled by the DMA-Thread by calling dc1394_capture_deque(..,POLICY_WAIT). When
        this function call returns, the Thread will push the new frame into its internal
        DCFrameQueue, where it can be accessed by the application by calling the getCurrentImage()
        function.\n
        <b>Note:</b> As it is strongly recommended <b>not</b> to create an own DCGrabberThread, but
        to use an instance of the DCGrabber instead, the DCGrabberThread has no public constructor.
    */
    class DCGrabberThread : public Thread{
      public: 
      /// A DCGrabberThread can only be instantiated by a DCGrabber
      friend class icl::DCGrabber;

      /// the thread function (moved frames) 
      virtual void run();
      
      /// resets the dc-camera at the end
      virtual void finalize();
      
      private: 
      /// private constructor )can only be called by icl::DCGrabber
      DCGrabberThread(dc1394camera_t* c);
      
      /// private image access function
      void getCurrentImage(ImgBase **ppoDst);
      
      /// internally used DCFrameQueue object
      DCFrameQueue *m_poFrameQueue;

      /// Associated camera
      dc1394camera_t* m_poCam;
      
      /// internally used buffer for RGB-Bayer image conversion
      std::vector<icl8u> m_oRGBInterleavedBuffer;
    };
  }
}

#endif
