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
#include <thread>
#include <mutex>
#include <atomic>

namespace icl{
  namespace utils{

    /// Simple object oriented thread class wrapping std::thread \ingroup THREAD
    /** Create a custom Thread class derived from this class,
        reimplement the virtual run() function and use start() and stop()
        to control the thread. Subclass run() methods should check running()
        periodically and return when it becomes false.

        \code
        class CounterThread : public utils::Thread{
          public:
          CounterThread(int n):n(n){}
          virtual void run(){
             while(running() && n > 0){
                lock();
                n--;
                printf("%d cycles left\n",n);
                unlock();
                msleep(1000);
             }
          }
          private:
          int n;
        };
        \endcode
    */
    class ICLUtils_API Thread {
      public:

      /// Create a new Thread
      Thread();

      /// Destructor (if the thread is still running, it is stopped)
      virtual ~Thread();

      /// Non-copyable
      Thread(const Thread&) = delete;
      Thread& operator=(const Thread&) = delete;

      /// starts the thread
      /** if the thread is already running, an error message is shown */
      void start();

      /// stops the thread (cooperative: sets running flag to false and joins)
      virtual void stop();

      /// waits for this thread to be ended
      void wait();

      /// pure virtual run function doing all the work
      virtual void run() = 0;

      /// at the end of the stop function, this function is called
      virtual void finalize(){}

      /// sets the current thread to sleep for some micro-seconds
      static void usleep(unsigned int usec);

      /// sets the current thread to sleep for some milli-seconds
      static void msleep(unsigned int msecs);

      /// sets the current thread to sleep for some seconds
      static void sleep(float secs);

      /// returns the internal running state (atomic, lock-free)
      bool running() const;

      /// returns the running state of the thread (same as running())
      bool runningNoLock() const;

      protected:

      /// sets the running flag to false (run() should check running() and return)
      void exit();

      /// internal used lock function
      /** This function (and unlock) can be used inside the reimplementation of
          the run function to ensure that the code between lock() and unlock()
          is executed to the end before the stop function is able to join the
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

      private:
      std::thread m_thread;
      std::recursive_mutex m_mutex;
      std::mutex m_lifecycle;
      std::atomic<bool> m_running{false};
    };

  } // namespace utils
}
