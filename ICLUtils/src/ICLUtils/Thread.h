// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <thread>
#include <mutex>
#include <atomic>

namespace icl::utils {
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
      void lock();

      /// internal used trylock function
      /** @return zero if lock is acquired, non-zero otherwise */
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

  } // namespace icl::utils