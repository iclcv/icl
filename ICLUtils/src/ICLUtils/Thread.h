/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Thread.h                         **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/ShallowCopyable.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  namespace utils{

    /** \cond */
    class ICLUtils_API ThreadImpl;
    struct ICLUtils_API ThreadImplDelOp{
      static void delete_func( ThreadImpl* impl );
    };
    /** \endcond */



    /// Simple object oriented thread class wrapping the pthread library \ingroup THREAD
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
        class CounterThread : public utils::Thread{
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

        \section PERF Performance
        Although the Thread class implements the ShallowCopyable interface, it
        is desingned for optimal performance. On a 1.6GHz Pentium-M (linux), you can
        create, run, stop and release about 50.000 Threads per second. Thus it
        is possible to use this simple Thread implementation to create multi-threaded
        image processing modules as filters and so on.  \n
        <b>TODO:</b> Implement a dedicated class framework for this
    */
    class ICLUtils_API Thread : public ShallowCopyable<ThreadImpl, ThreadImplDelOp>{
      public:

      /// Create a new Thread
      Thread();

      /// Destructor (if the thread is still running, it is ended)
      virtual ~Thread();

      /// starts the thread
      /** if the thread is already running, an error message is shown */
      void start();

      /// stops the thread and waits till it is ended
      virtual void stop();

      /// waits for this thread to be ended
      /** The Thread ends automatically when its run function is over, or - if the
          run() function has not run to the end yet - it is ended when stop is called.
      **/
      void wait();

      /// pure virtual run function doing all the work
      virtual void run()=0;

      /// at the end of the stop function, this function is called
      virtual void finalize(){}


      /// just calling usleep
      static void usleep(unsigned int usec);

      /// sets the current thread to sleep for some milli-seconds
      /** @param msecs time in msecs to sleep **/
      static void msleep(unsigned int msecs);

      /// sets the current thread to sleep for some seconds
      /** @param secs time in secs to sleep  ( float precision!)**/
      static void sleep(float secs);

      /// returns the internal running state
      /** Internally, the implementation mutex will be locked here.
          This *might* cause issues */
      bool running() const;

      /// returns the running state of the thread (without locking the mutex)
      /** this should usually be better (since less prone to producing
          deadlocks) then the running() method */
      bool runningNoLock() const;

      protected:

      /// exits the thread using pthread_exit (must be called from within the run function)
      void exit();

      /// internal used lock function
      /** This function (and unlock) can be used inside the reimplementation of
          the run function to enshure, that the code between lock() and unlock()
          is executed to the end befor the stop function is able to join the
          Thread
      **/
      void lock();
      /// internal used trylock function
      /** works like lock but without blocking (it returns immediately).
          @return zero if lock is acquired. otherwise an error-number
      **/
      int trylock();

      /// internal used unlock function
      void unlock();

      /// internal used join function
      void join();
    };


    /// static utility function which deletes a pointer and sets it to NULL \ingroup THREAD
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
    /// static utility function which ensures Thread-safety for object functions \ingroup THREAD
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
  } // namespace utils
}


