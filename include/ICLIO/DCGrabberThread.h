/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/DCGrabberThread.h                        **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_DC_GRABBER_THREAD_H
#define ICL_DC_GRABBER_THREAD_H

#include <ICLIO/DC.h>
#include <ICLIO/DCDeviceOptions.h>
#include <ICLUtils/Thread.h>
#include <ICLCore/Types.h>
#include <ICLUtils/Time.h>


#include <vector>


namespace icl{

  /** \cond */
  class DCGrabberImpl;
  /** \endcond */
  
  namespace dc{
    
    /** \cond */
    class DCFrameQueue;
    /** \endcond */
    
    /// Internally spawned thread class to provide continuous grabbing without drop frames \ingroup DC_G
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
      friend class icl::DCGrabberImpl;

      /// the thread function (moved frames) 
      virtual void run();
      
      /// resets the dc-camera at the end
      virtual void finalize();
      
      /// called by the signal handler to stop all grabber threads
      static void stopAllGrabberThreads();
      
      /// internally calls dc1394_reset_bus (not save!)
      void resetBus();
      
      private: 
      /// private constructor )can only be called by icl::DCGrabber
      DCGrabberThread(dc1394camera_t* c, DCDeviceOptions *options);

      /// Destructor
      ~DCGrabberThread();

      /// private image access function
      //void getCurrentImage(ImgBase **ppoDst,dc1394color_filter_t bayerLayout);
      
      /// complex function to get the next image
      /** The function gets all desired params from the top level grabber, which it
          should fullfill. But in some cases it's not possible to satisfy all 
          desired params constraints. In this case, this function will use the
          second given ImgBase** (ppoDstTmp) as destination image and it will set
          the boolean reference named desiredParamsFullfilled to false. The parent
          grabber can check this variable, to decide whether to use the original
          destination pointer (ppoDst) or to use the ppoDstTmp pointer temporarily
          and convert is into ppoDst by itself, using the desired params for ppoDst.
          <b>TODO: some more text here !</b>
      */
      void getCurrentImage(ImgBase **ppoDst, 
                           ImgBase **ppoDstTmp,
                           bool &desiredParamsFullfilled, 
                           const Size &desiredSizeHint, 
                           format desiredFormatHint,
                           depth desiredDepthHint,
                           dc1394color_filter_t bayerLayout,
                           dc1394bayer_method_t bayerMethod=DC1394_BAYER_METHOD_BILINEAR);

      /// returns the current image directly (if no desried parameters are set)
      void getCurrentImage(ImgBase **ppoDst, 
                           dc1394color_filter_t bayerLayout,
                           dc1394bayer_method_t bayerMethod=DC1394_BAYER_METHOD_BILINEAR);

      
      dc1394video_frame_t *waitForNextImageFrame();
      
      /// internally used DCFrameQueue object
      DCFrameQueue *m_poFrameQueue;

      /// Associated camera
      dc1394camera_t* m_poCam;
      
      /// internally used buffer for RGB-Bayer image conversion
      std::vector<icl8u> m_oRGBInterleavedBuffer;
      
      /// Parents DCGrabbers options pointer
      DCDeviceOptions *m_poOptions;

      /// to remember the time stamp of the last frame grabbed
      Time m_lastFramesTimeStamp;
    };
  }
}

#endif
