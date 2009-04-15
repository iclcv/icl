#include <iclMutex.h>
#include <iclLockable.h>

namespace icl{

  Mutex::Locker::Locker(Lockable *l):m(&l->getMutex()){
    m->lock();
  }
  Mutex::Locker::Locker(Lockable &l):m(&l.getMutex()){
    m->lock();
  }
  
}
