#include <pthread.h>

namespace icl{
  /// Mutex class of the ICL
  /** This mutex class is a simple object oriented wrapper of the
      pthread_mutex_t struct.
      
      Mutices can be:
      - created (pthread_mutex_init)
      - locked (pthread_mutex_lock)
      - unlocked (pthread_mutex_unlock)
      - and destroyed (Destructor)-> (pthread_mutex_destroy)
  **/
  class Mutex{
    public:
    /// Create a mutex
    Mutex(){
      pthread_mutex_init(&m,0);
    }
    /// Destroys the mutex
    ~Mutex(){
      pthread_mutex_destroy(&m);
    }
    /// locks the mutex
    void lock(){
      pthread_mutex_lock(&m);
    }

    /// unlocks the mutex
    void unlock(){
      pthread_mutex_unlock(&m);
    }
    
    private:
    /// wrapped thread_mutex_t struct
    pthread_mutex_t m;
  };
}
