#include <iclDCGrabberThread.h>
#include <iclDCFrameQueue.h>
#include <iclMacros.h>
#include <iclMutex.h>
#include <iclSignalHandler.h>
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

      g_oGrabberThreadMutex.lock();
      g_bStopAllGrabberThreadsCalled = true;

      for(unsigned int i=0;i<g_vecAllThreads.size();i++){
        g_vecAllThreads[i]->stop(false);
      }
      g_vecAllThreads.clear();
      g_bStopAllGrabberThreadsCalled = false;
      g_oGrabberThreadMutex.unlock();
    }
    
    DCGrabberThread::DCGrabberThread(dc1394camera_t* c):
      m_poFrameQueue(0),m_poCam(c){
      g_oGrabberThreadMutex.lock();
      g_vecAllThreads.push_back(this);
      g_oGrabberThreadMutex.unlock();
    }
    
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
    
    void DCGrabberThread::getCurrentImage(ImgBase **ppoDst){
      while(!m_poFrameQueue) usleep(1000*10);
      m_poFrameQueue->lock();
      extract_image_to(m_poFrameQueue->back(),ppoDst,m_oRGBInterleavedBuffer);
      m_poFrameQueue->unlock();
    }
    
  }
}
