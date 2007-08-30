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
      QSemaphore *m_poProvided; // provided work
      QSemaphore *m_poDone;     // done work
      bool m_pbNewDataAvailable;
      
      // public:
      WorkingThread(QSemaphore *provided, QSemaphore *done):
      m_ptFunctor(0),doEnd(false),m_poProvided(provided),m_poDone(done),m_pbNewDataAvailable(false){
        start();
      }
      ~WorkingThread(){
        doEnd = true;
        wait();
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
          if(!m_pbNewDataAvailable){
            usleep(0);
          }else{
            m_poProvided->acquire();
            if(doEnd) break;
            
            if(m_ptFunctor){
              (*m_ptFunctor)();
              m_pbNewDataAvailable = false;
            }
            m_poDone->release();
          }
        }
      }
    };

    public:
    MultiThreader(int nThreads=2):m_oProvidedSem(nThreads),m_oDoneSem(nThreads),m_iNumThreads(nThreads){
      m_vecThreads.resize(nThreads);
      
      m_oProvidedSem.acquire(nThreads);
      m_oDoneSem.acquire(m_vecThreads.size());
      
      for(int i=0;i<nThreads;i++){
        m_vecThreads[i] = new WorkingThread(&m_oProvidedSem,&m_oDoneSem);
      }

      
    }
    ~MultiThreader(){

      for(int i=0;i<numThreads();i++){ 
        m_vecThreads[i]->end();
        m_oProvidedSem.release(1);
        delete m_vecThreads[i];
      }
    }
    int numThreads() const { return m_iNumThreads; }
    
    void apply(std::vector<T*> &functors){
      ICLASSERT_RETURN((int)functors.size() == numThreads() );

      for(int i=0;i<numThreads();i++){
        m_vecThreads[i]->setFunctor(functors[i]);
      }
      m_oProvidedSem.release(numThreads());
      m_oDoneSem.acquire(numThreads());
    }
    
    private:

    std::vector<WorkingThread*> m_vecThreads;
    QSemaphore m_oProvidedSem;
    QSemaphore m_oDoneSem;
    int m_iNumThreads;
    
  };
  
}

#endif
