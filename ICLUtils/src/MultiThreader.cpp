#include <ICLUtils/MultiThreader.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/Semaphore.h>
#include <ICLUtils/Mutex.h>


namespace icl{
  struct MultiThreader::MTWorkThread : public Thread{
    // {{{ open

    MTWorkThread(Semaphore *sem1, Semaphore *sem2, Semaphore *semDone):work(0),m_bCurr(0){
      // {{{ open

      m_apoSems[0] = sem1;
      m_apoSems[1] = sem2;
      m_poSemDone = semDone;
      m_bEnd = false;
    }

    // }}}
    
    ~MTWorkThread(){
      // {{{ open

      m_oWorkMutex.lock();
      work = 0;
      m_oWorkMutex.unlock();
      m_bEnd = true;
      m_apoSems[m_bCurr]->release(1);
    }

    // }}}
    
    virtual void run(){
      // {{{ open

      while(!m_bEnd){
        m_apoSems[m_bCurr]->acquire(1);
        Thread::usleep(0);
        m_oWorkMutex.lock();
        Thread::usleep(0);
        if(work){

          work->perform();
        }
        Thread::usleep(0);
        m_oWorkMutex.unlock();
        Thread::usleep(0);
        m_poSemDone->release(1);
        Thread::usleep(0);
        m_bCurr = !m_bCurr;
        
        Thread::usleep(0);
      }
    }

    // }}}
    
    void setWork(MultiThreader::Work *work){
      // {{{ open

      m_oWorkMutex.lock();
      this->work = work;
      m_oWorkMutex.unlock();
    }

    // }}}
    
  private:
    MultiThreader::Work *work;
    Semaphore *m_apoSems[2];
    Semaphore *m_poSemDone;
    Mutex m_oWorkMutex;
    bool m_bCurr;
    bool m_bEnd;
  };

  // }}}

  class MultiThreaderImpl{
    // {{{ open

  public:
    MultiThreaderImpl(int nThreads):
      // {{{ open

      m_iNThreads(nThreads),m_bCurr(0){
      m_vecThreads.resize(nThreads);
      
      m_apoSems[0] = new Semaphore(nThreads);
      m_apoSems[1] = new Semaphore(nThreads);
      m_poDoneSem = new Semaphore(nThreads);
      
      
      m_apoSems[0]->acquire(nThreads);
      m_apoSems[1]->acquire(nThreads);
      m_poDoneSem->acquire(nThreads);
      
      for(int i=0;i<nThreads;++i){
        m_vecThreads[i] = new MultiThreader::MTWorkThread(m_apoSems[0],m_apoSems[1],m_poDoneSem);
        m_vecThreads[i]->start();
      }
    }

    // }}}
    ~MultiThreaderImpl(){
      // {{{ open

      for(int i=0;i<m_iNThreads;++i){
        delete m_vecThreads[i];
      }
      delete m_apoSems[0];
      delete m_apoSems[1];
    }

    // }}}
    
    inline void apply(MultiThreader::WorkSet &ws){
      // {{{ open

      ICLASSERT_RETURN((int)ws.size() == m_iNThreads);
      
      /// arm the threads
      for(unsigned int i=0;i<ws.size();i++){
        m_vecThreads[i]->setWork(ws[i]);
      }
      
      m_apoSems[m_bCurr]->release(m_iNThreads);
      m_poDoneSem->acquire(m_iNThreads);
      
      /// defuse the threads !
      for(unsigned int i=0;i<ws.size();i++){
        m_vecThreads[i]->setWork(0);
      }
      
      m_bCurr =! m_bCurr;
    }

    // }}}
    
    inline int getNumThreads() const { 
      // {{{ open

      return m_iNThreads;
    }

    // }}}

  private:
    int m_iNThreads;
    std::vector<MultiThreader::MTWorkThread*> m_vecThreads;
    Semaphore *m_apoSems[2];
    Semaphore *m_poDoneSem;
    Mutex m_oSemMutex;
    bool m_bCurr;

  };

  // }}}

  void MultiThreaderImplDelOp::delete_func( MultiThreaderImpl *impl){
    // {{{ open

    ICL_DELETE( impl);
  }

  // }}}
  
  MultiThreader::MultiThreader():
    // {{{ open

    ShallowCopyable<MultiThreaderImpl,MultiThreaderImplDelOp>(0){
  
  }

  // }}}

  MultiThreader::MultiThreader(int nThreads):
    // {{{ open

    ShallowCopyable<MultiThreaderImpl,MultiThreaderImplDelOp>(new MultiThreaderImpl(nThreads)){
    
  }

  // }}}
  
  void MultiThreader::operator()(MultiThreader::WorkSet &ws){
    // {{{ open
    ICLASSERT_RETURN(!isNull());
    impl->apply(ws);
  }

  // }}}

  int MultiThreader::getNumThreads() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),0);
    return impl->getNumThreads();
  }

  // }}}
}
