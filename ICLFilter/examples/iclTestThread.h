#include <iclThread.h>
#include <iclUnaryOp.h>

namespace icl {
  class TestThread : public Thread{
    public:
    TestThread(UnaryOp *op):Thread(),m_poSrc(0),m_ppoDst(0),m_poOp(op),m_bNewWorkPresent(false),m_bWorkDone(false){
      pthread_mutex_init(&m_oWaitHEREMutex,0);
      pthread_cond_init(&m_oWaitHERECond,0);
      pthread_mutex_init(&m_oWaitTHEREMutex,0);
      pthread_cond_init(&m_oWaitTHERECond,0);
      start();
    }
    virtual void run(){
      while(1){
        printf("run 1 \n");
        pthread_mutex_lock(&m_oWaitHEREMutex);
        printf("run 2 \n");
        while(!m_bNewWorkPresent){
          printf("run 3 \n");
          pthread_cond_wait(&m_oWaitHERECond,&m_oWaitHEREMutex);
          printf("run 4 \n");
        }
        printf("applying filter in thread %p \n",(void*)this);
        m_poOp->apply(m_poSrc, m_ppoDst);
        m_bNewWorkPresent = false;
        m_bWorkDone = true;        
        pthread_mutex_unlock(&m_oWaitHEREMutex);
                
        pthread_cond_signal(&m_oWaitTHERECond);
        

      }    
    }
    
    virtual void apply(const ImgBase *poSrc, ImgBase ** ppoDst){
      pthread_mutex_lock(&m_oWaitHEREMutex);
      printf("apply 1 \n");
      m_bWorkDone = false;
      printf("apply 2 \n");
      m_poSrc = poSrc;
      m_ppoDst = ppoDst;
      printf("apply 3 \n");
      pthread_cond_signal(&m_oWaitHERECond);
      printf("apply 4 \n");
      pthread_mutex_unlock(&m_oWaitHEREMutex);
    }
    void waitForApply(){
      printf("wait for apply 1 \n");
      pthread_mutex_lock(&m_oWaitTHEREMutex);
      printf("wait for apply 2 \n");
      while(!m_bWorkDone){
        printf("wait for apply 3 \n");
        pthread_cond_wait(&m_oWaitTHERECond,&m_oWaitTHEREMutex);
        printf("wait for apply 4 \n");
      }
      printf("wait for apply 5 \n");
      pthread_mutex_unlock(&m_oWaitTHEREMutex);
      printf("wait for apply 6 \n");
    }
    
    private:
    const ImgBase *m_poSrc;
    ImgBase **m_ppoDst;
    UnaryOp *m_poOp;
    
    pthread_cond_t m_oWaitHERECond;
    pthread_mutex_t m_oWaitHEREMutex; 

    pthread_cond_t m_oWaitTHERECond;
    pthread_mutex_t m_oWaitTHEREMutex;
    bool m_bNewWorkPresent;
    bool m_bWorkDone;
  };
}

