#ifndef ICL_LOCKABLE_H
#define ICL_LOCKABLE_H

#include <ICLUtils/Mutex.h>
#include <ICLUtils/UncopiedInstance.h>

namespace icl{

  /// Interface for objects, that can be locked using an internal mutex
  class Lockable{

    /// mutable and uncopied mutex instance (locking preserves constness)
    mutable UncopiedInstance<Mutex> m_mutex;
    public:
    
    /// lock object
    void lock() const { m_mutex.lock(); }
    
    /// unlock object
    void unlock() const { m_mutex.unlock(); }
    
    /// returns mutex of this object
    Mutex &getMutex() const { return m_mutex; }
  };
  
}

#endif
