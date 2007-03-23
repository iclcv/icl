#ifndef ICL_THREAD_H
#define ICL_THREAD_H

#include <pthread.h>
#include <unistd.h>

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
      
      */
  class Thread{
    public:
    /// Create a new Thread
    Thread();

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
    void lock() { pthread_mutex_lock(&m_oMutex); }
    
    /// internal used unlock function
    void unlock(){ pthread_mutex_unlock(&m_oMutex); }
    
    void msleep(unsigned int msecs){
      usleep(msecs*1000);
    }
    void sleep(float secs){
      usleep((long)secs*1000000);
    }
    private:

    pthread_t m_oPT;
    pthread_mutex_t m_oMutex;
  };
}


#endif
