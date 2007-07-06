#ifndef ICL_THREAD_H
#define ICL_THREAD_H

#include <pthread.h>
#include <unistd.h>
#include <iclMutex.h>

namespace icl{
  /// Simple object oriented thread class wrapping the pthread library
  /** This Thread class is very simple to understand, and behaves essentially
      like Qts QThread. Create a custom Thread class derived from this class, 
      reimplement the virtual run() function and use start and stop to contol the
      thread.
      a call to the threads start function will internally create a pthread which
      calls the Threads run() function a single time, so you have to add a while(1)
      statement to create a thread that is running until stop is called. Once a 
      Thread run function returns, the corresponding pthread will call the Threads
      stop() function.
      Here is an example of a counter thread!
      \code
      class CounterThread : public Thread{
        public:
        CounterThread(int n):n(n){}
        virtual void run(){
           while(n>0){
              lock();
              n--;
              printf("%d cycles left\n",n);
              unlock();
              sleep(1);
           }
        }
        private:
        int n;
      
      };
      \endcode
      Additionally each Thread can be initialized with a given priority level.
      This feature is copied from Qt (version 4.2.x) and is not yet tested
      explicitly. 
      */
  class Thread{
    public:
    friend void *icl_thread_handler(void *);

    /// used priority stepping (see Qts QThread!)
    enum priority{
      idle,     /**< this thread is only working when the processor is idle   */   
      lowest,   /**< lowest priority */
      low,      /**< lowe priority */
      normal,   /**< normal priority (default) */
      heigh,    /**< high priority */
      heighest, /**< highest priority */
      critical, /**< time critical priority */
      inherit   /**< inherit priority */
    };

    
    /// Create a new Thread
    Thread(priority p=normal);

    /// Destructor
    virtual ~Thread();
    
    /// starts the thread
    void start();
    
    /// stops the thread
    /** @param locked if set to false, the running thread is stopped immediately
                      without waiting for it to return from the internal locked
                      state (use locked = false only if you know what you are 
                      doing). An example for a non-locking call to a threads
                      stop function may be explicit signal handling e.g. from the
                      SIGINT signal. Here it might be necessary not to wait
                      for the running thread.
    */
    void stop(bool locked = true);
    
    /// virtual run function doing all the work
    virtual void run();

    /// at the end of the stop function, this function is called
    virtual void finalize(){}
    /// internal used lock function
    /** This function (and unlock) can be used inside the reimplementation of
        the run function to enshure, that the code between lock() and unlock() 
        is executed to the end befor the stop function is able to join the 
        Thread 
    **/
    void lock() { m_oMutex.lock(); }
    
    /// internal used unlock function
    void unlock(){ m_oMutex.unlock(); }
    
    /// waits for this thread to be stopped
    /** This function can be called from the parent thread when it is not
        clear, when the icl-Thread has run to the end 
    **/
    void waitFor(){
      while(m_bRunning){
        pthread_cond_wait(&m_oWaitCond,&m_oWaitMutex);
      }
    }

 
    /// static utility function which deletes a pointer and sets it to NULL
    /** Internally this function template will create a specific mutex for the
        given class T. All calls to the saveDelete function are protected
        by the static mutex. So saveDelete(XXX) can be called from different
        threads [but with the identical pointer reference] without the risk 
        of segmentation violations or double free exceptions.
    */
    template<class T>
    static inline void saveDelete(T* &pointer){
      static Mutex m;
      m.lock();
      ICL_DELETE(pointer);
      m.unlock();
    }
    /// static utility function which ensures Thread-safety for object functions
    /** Internally this function template will create a specific mutex for the
        given class T and the given member function. When using the saveCall template
        to call an objects member-function from different threads, the function 
        will automatically become Thread-save, as it is only executes once at one time.
    */
    template<class T, void (T::*func)()>
    static inline void saveCall(T *obj){
      static Mutex m;
      m.lock();
      (obj->*func)();
      m.unlock();
    }

    protected:
    
    /// sets this thread to sleep for some milli-seconds
    /** @param msecs time in msecs to sleep **/
    void msleep(unsigned int msecs){
      usleep(msecs*1000);
    }
    
    /// sets this thread to sleep for some seconds
    /** @param secs time in secs to sleep  ( float precision!)**/
    void sleep(float secs){
      usleep((long)secs*1000000);
    }
    
    
    private:
    
    pthread_attr_t m_oAttr;
    pthread_t m_oPT;
    Mutex m_oMutex;
    pthread_cond_t m_oWaitCond;
    pthread_mutex_t m_oWaitMutex;
    bool m_bRunning;
  };
}


#endif
