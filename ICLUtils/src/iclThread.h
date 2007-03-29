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
    void stop();

    /// virtual run function doing all the work
    virtual void run();

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
    
    //
    pthread_attr_t m_oAttr;
    pthread_t m_oPT;
    Mutex m_oMutex;
    pthread_cond_t m_oWaitCond;
    pthread_mutex_t m_oWaitMutex;
    bool m_bRunning;
  };
}


#endif
