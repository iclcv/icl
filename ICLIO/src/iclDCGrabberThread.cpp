#include <iclDCGrabberThread.h>
#include <iclDCFrameQueue.h>
#include <iclMacros.h>

namespace icl{
  namespace dc{
    
    DCGrabberThread::DCGrabberThread(dc1394camera_t* c):
      m_poCam(c),m_poFrameQueue(0){}
    
    void DCGrabberThread::run(){
      if(!m_poFrameQueue){
        m_poFrameQueue = new DCFrameQueue(m_poCam);
      }
      while(true){
        lock();
        m_poFrameQueue->step();
        unlock();
        msleep(1);
      }
    }
    void DCGrabberThread::finalize(){
      ICL_DELETE(m_poFrameQueue);
    }
    
    void DCGrabberThread::getCurrentImage(ImgBase **ppoDst){
      while(!m_poFrameQueue) usleep(1000*10);
      m_poFrameQueue->lock();
      extract_image_to(m_poFrameQueue->back(),ppoDst,m_oRGBInterleavedBuffer);
      m_poFrameQueue->unlock();
    }
    
  }
}
