#ifndef ICL_MULTI_THREADER_H
#define ICL_MULTI_THREADER_H

#include <vector>
#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include <iclMacros.h>


namespace icl{
  
  template<class T>
  class MultiThreader{

    class WorkingThread : public QThread{
      friend class MultiThreader;

      T *m_ptFunctor;
      bool doEnd;
      QSemaphore *m_poProvided1; // provided work
      QSemaphore *m_poProvided2; // provided work
      QSemaphore *m_poDone;     // done work
      bool m_pbNewDataAvailable;
      bool m_bEnded;
    
      
      // public:
      WorkingThread(QSemaphore *provided1,QSemaphore *provided2, QSemaphore *done):
      m_ptFunctor(0),
      doEnd(false),
      m_poProvided1(provided1),
      m_poProvided2(provided2),
      m_poDone(done),
      m_pbNewDataAvailable(false),
      m_bEnded(false){
        start();
      }
      ~WorkingThread(){
        doEnd = true;
        while(!m_bEnded){
          msleep(10);
        }
        printf("calling quit()! \n");
        quit(); // term
        printf("calling wait()! \n");
        wait();
        printf("deleting the functor! \n");
        ICL_DELETE(m_ptFunctor);
      }
     
      void end(){ doEnd = true; }
      
      void setFunctor(T* t){
        if(m_ptFunctor != t){
          ICL_DELETE(m_ptFunctor);
          m_ptFunctor = t; 
        }
        m_pbNewDataAvailable = true;
      }
      
      void applyFunctor(){
        if(m_ptFunctor){
          (*m_ptFunctor)();
        }
      }
      
      void run(){
        while(!doEnd){
          printf("-- run [1] \n");
          m_poProvided1->acquire();
          printf("-- run [2] \n");
          if(doEnd) break;
          
          printf("-- run [3] \n");
          if(m_ptFunctor){
            (*m_ptFunctor)();
            m_pbNewDataAvailable = false;
          }
          printf("-- run [4] \n");
          m_poDone->release();

          printf("-- run [5] \n");
          m_poProvided2->acquire();
          printf("-- run [6] \n");
          if(doEnd) break;
          
          printf("-- run [7] \n");
          if(m_ptFunctor){
            (*m_ptFunctor)();
            m_pbNewDataAvailable = false;
          }
          printf("-- run [8] \n");
          m_poDone->release();
          printf("-- run [9] \n");

        }
        m_bEnded = true;
      }
    };

    public:

    MultiThreader(int nThreads=2):
    m_iProvidedSemIdx(0),
    m_vecThreads(nThreads),
    m_oDoneSem(nThreads),
    m_iNumThreads(nThreads),      
    m_bThreadsRunning(false){
      startThreads();
    }
    
    ~MultiThreader(){
      stopThreads();
    }
    int numThreads() const { return m_iNumThreads; }
    
    void stopThreads(){
      if(m_bThreadsRunning){
        printf("-- stopThreads [1] \n");
        for(int i=0;i<numThreads();i++){
          printf("-- stopThreads [2.%d] \n",i); 
          m_vecThreads[i]->end();
          printf("-- stopThreads [3.%d] \n",i); 
          m_apoProvidedSem[m_iProvidedSemIdx]->release(1);       
          printf("-- stopThreads [4.%d] \n",i); 
          //          m_apoProvidedSem[!m_iProvidedSemIdx]->release(1);
          delete m_vecThreads[i];
          printf("-- stopThreads [5.%d] \n",i); 
        }    
        printf("-- stopThreads [6] \n");
        m_bThreadsRunning = false;
      }
      printf("-- stopThreads [7] \n");
      delete m_apoProvidedSem[0];
      m_apoProvidedSem[0] = 0;
      printf("-- stopThreads [8] \n");
      delete m_apoProvidedSem[1];
      m_apoProvidedSem[1] = 0;
      printf("-- stopThreads [9] \n");
    }

    void startThreads(){
      if (!m_bThreadsRunning){
        int n = numThreads();
        m_apoProvidedSem[0] = new QSemaphore(n);
        m_apoProvidedSem[1] = new QSemaphore(n);
        m_apoProvidedSem[0]->acquire(n);
        m_apoProvidedSem[1]->acquire(n);
        m_oDoneSem.acquire(n);
        
        for(int i=0;i<n;i++){
          m_vecThreads[i] = new WorkingThread(m_apoProvidedSem[0],m_apoProvidedSem[1],&m_oDoneSem);
        }
        m_bThreadsRunning = true;
        m_iProvidedSemIdx = 0; 
      } 
    }
    
    void apply(std::vector<T*> &functors){
      ICLASSERT_RETURN((int)functors.size() == numThreads() );
      printf("-- apply [1] \n");
      for(int i=0;i<numThreads();i++){
        m_vecThreads[i]->setFunctor(functors[i]);
      }      
      printf("-- apply [2] \n");
      m_apoProvidedSem[m_iProvidedSemIdx]->release(numThreads());
      printf("-- apply [3] \n");
      m_oDoneSem.acquire(numThreads());
      printf("-- apply [4] \n");
      m_iProvidedSemIdx = !m_iProvidedSemIdx;
      printf("-- apply [5] \n");
    }
    
    private:
    int m_iProvidedSemIdx;
    std::vector<WorkingThread*> m_vecThreads;
    QSemaphore *m_apoProvidedSem[2];
    QSemaphore m_oDoneSem;
    int m_iNumThreads;
    bool m_bThreadsRunning;
    
    
  };
  
}

#endif
