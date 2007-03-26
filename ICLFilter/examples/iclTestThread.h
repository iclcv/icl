#include <iclThread.h>
#include <iclUnaryOp.h>

namespce icl{
  class TestThread : public Thread{
    public:
    TestThread(UnaryOp *op):Thread(),m_poSrc(0),m_ppoDst(0){
      m_oRunMutex.lock();
      start();
    }
    virtual void run(){
      while(1){
        m_oRunMutex.lock();
        m_oDataMutex.lock();
        m_poOp->apply(m_poSrc, m_poDst);
        m_oDataMutex.unlock();
        m_oRunMutex.unlock();
      }    
    }
    
    virtual void apply(const ImgBase *poSrc, ImgBase ** ppoDst){
      m_oDataMutex.lock();
      m_poSrc = poSrc;
      m_ppoDst = ppoDst;
      m_oDataMutex.unlock();
      
      m_oRunMutex.unlock();
      usleep(1);
      m_oRunMutex.lock();
    }
    
    private:
    const ImgBase *m_poSrc;
    ImgBase ***m_poDst;
    UnaryOp *m_poOp;
    Mutex m_oDataMutex;
    Mutex m_oRunMutex;
  };
}

