#ifndef ICL_THREAD_UTILS_H
#define ICL_THREAD_UTILS_H

#include "iclThread.h"
#include "iclMutex.h"
#include <vector>

namespace icl{

  /** \cond */
  /// Utility class used in the exec_threaded template function 
  template<class Callback>
  class CallbackThread : public Thread{
    std::vector<Callback> cbs;
    Mutex mutex;
    int sleeptime;
    bool looped;
    
    public:
    
    CallbackThread(int sleeptimeMsec, bool looped):
    sleeptime(sleeptimeMsec),looped(looped){
      start();
    }

    void setSleepTime(int sleepTimeMsecs){
      sleeptime = sleepTimeMsecs;
    }
    
    void add(Callback cb){
      mutex.lock();
      cbs.push_back(cb);
      mutex.unlock();
    }
    
    virtual void run(){
      while(1){
        { /// use a scoped locker to enshure, that the mutex is unlocked at the end
          Mutex::Locker locker(mutex);
          if(!cbs.size()){
            msleep(10);
          }else{
            for(unsigned int i=0;i<cbs.size();++i){
              cbs[i]();
            }
          }
          mutex.unlock();
          msleep(sleeptime);
        }
        if(!looped) break;
      }
    }
  };
  /** \endcond */
  
  /// Start a function or a functor in an own thread \ingroup THREAD 
  /** This function is a convenience wrapper for threaded applications. 
      Instead of deriving the Thread class of the ICL an implementing it's
      virtual void run() function, the exec_threaded template function
      can easily be used to start a thread, that executes any function or
      functor. 
      If the template parameter Callback is a function, it must get no 
      argumtents, if its a functor class, it has to have an empty functor '()'.

      Multiple times calling exec_threaded with different functions, will 
      start different threads for each function. Multiple times calling 
      exec_threaded with an identical function, will exec this function
      that many times before calling msleep(sleeptime)
      
      @param cb given callback (a function or functor)
      @param loop if true, the function is looped (while(1) { cb(); } )
             otherwise it is called once
      @param sleeptime milliseconds sleeptime that is slept by the 
             instantiated thread between two successive calles of a
             callback (this is only useful if the looped flag is set to true)
      */
  template<class Callback>
  static void exec_threaded(Callback cb,bool loop=true, int sleepMsecs=0){
    static CallbackThread<Callback> *cbt = new CallbackThread<Callback>(sleepMsecs,loop);
    cbt->setSleepTime(sleepMsecs);
    cbt->add(cb);
  }
}

#endif
