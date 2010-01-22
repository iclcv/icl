#ifndef ICL_SEMAPHORE_H
#define ICL_SEMAPHORE_H

#include <ICLUtils/ShallowCopyable.h>


namespace icl{
  
  /** \cond */
  class SemaphoreImpl;
  class SemaphoreImplDelOp{
    public: static void delete_func(SemaphoreImpl *impl);
  };
  /** \endcond */
  
  
  /// Simple Semaphore implementation wrapping the standard linux "sem_t"-struct \ingroup THREAD
  class Semaphore : public ShallowCopyable<SemaphoreImpl,SemaphoreImplDelOp>{
    public:
    /// create a semaphore initialized with n resources
    Semaphore(int n=1);
    
    /// releases a resource
    void operator++(int dummy);
    
    /// acquires a new resource
    void operator--(int dummy);
    
    /// releases val resources
    void operator+=(int val);
    
    /// acquires val resources
    void operator-=(int val);
    
    /// releases val resources
    void acquire(int val=1){ (*this)-=val; }

    /// acquires val resources
    void release(int val=1){ (*this)+=val; }
    
    /// trys to acquire one resource if successfull it returns true else false
    bool tryAcquire();
    
    /// releases one resource only if resources are aqcuired
    bool tryRelease();
    
    /// returns the current value
    int getValue();
    
    /// returns the semaphores max-value
    int getMaxValue();
  };
  
}

#endif
