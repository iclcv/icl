#include <iclDCGrabberThread.h>
#include <iclDCFrameQueue.h>
#include <iclMacros.h>
#include <iclMutex.h>
#include <iclSignalHandler.h>
#include <iclDCDevice.h>
#include <algorithm>
#include <vector>


using namespace std;
namespace icl{
  namespace dc{
    
    /// mutex protected list of all currently running grabber threads
    Mutex g_oGrabberThreadMutex;
    vector<DCGrabberThread*> g_vecAllThreads;
    bool g_bStopAllGrabberThreadsCalled = false;

    
    
    void DCGrabberThread::stopAllGrabberThreads(){
      // {{{ open

      g_oGrabberThreadMutex.lock();
      g_bStopAllGrabberThreadsCalled = true;

      for(unsigned int i=0;i<g_vecAllThreads.size();i++){
        g_vecAllThreads[i]->stop();
      }
      g_vecAllThreads.clear();
      g_bStopAllGrabberThreadsCalled = false;
      g_oGrabberThreadMutex.unlock();
    }

    // }}}
    
    DCGrabberThread::DCGrabberThread(dc1394camera_t* c, DCDeviceOptions *options):
      // {{{ open

      m_poFrameQueue(0),m_poCam(c),m_poOptions(options){
      g_oGrabberThreadMutex.lock();
      g_vecAllThreads.push_back(this);
      g_oGrabberThreadMutex.unlock();
    }

    // }}}
    
    void DCGrabberThread::run(){
      // {{{ open
      if(!m_poFrameQueue){
        m_poFrameQueue = new DCFrameQueue(m_poCam, m_poOptions);
      }
      while(true){
        lock();
        m_poFrameQueue->step();
        unlock();
        msleep(1);
      }
    }

    // }}}

    void DCGrabberThread::finalize(){
      // {{{ open

      ICL_DELETE(m_poFrameQueue);
      if(!g_bStopAllGrabberThreadsCalled){
        /// remove from the grabber thread list to 
        g_oGrabberThreadMutex.lock();
        vector<DCGrabberThread*>::iterator it = find(g_vecAllThreads.begin(),g_vecAllThreads.end(),this);
        if(it != g_vecAllThreads.end()){
          g_vecAllThreads.erase(it);
        }
        g_oGrabberThreadMutex.unlock();
      }
    }

    // }}}
    
    void DCGrabberThread::getCurrentImage(ImgBase **ppoDst){
      // {{{ open
      
      while(!m_poFrameQueue) usleep(1000*10);
      m_poFrameQueue->lock();
      extract_image_to(m_poFrameQueue->back(),ppoDst,m_oRGBInterleavedBuffer);
      m_poFrameQueue->unlock();
    }

    // }}}

    void DCGrabberThread::getCurrentImage(ImgBase **ppoDst, 
                                          ImgBase **ppoDstTmp,
                                          bool &desiredParamsFullfilled,
                                          const Size &desiredSizeHint, 
                                          format desiredFormatHint,
                                          depth desiredDepthHint,
                                          dc1394bayer_method_t bayerMethod){
      // {{{ open

      while(!m_poFrameQueue) usleep(1000*10);
    
      m_poFrameQueue->lock();
      
      dc1394video_frame_t *frame = m_poFrameQueue->back();
      
      DCDevice dev(m_poCam);
      desiredParamsFullfilled = can_extract_image_to(frame,dev,desiredSizeHint,desiredFormatHint,desiredDepthHint);
      extract_image_to(frame,dev,
                       desiredParamsFullfilled ? ppoDst : ppoDstTmp,
                       desiredSizeHint,
                       desiredFormatHint,
                       desiredDepthHint,
                       m_oRGBInterleavedBuffer,
                       bayerMethod);
      
      m_poFrameQueue->unlock();
      
    }

    // }}}
  }
}
