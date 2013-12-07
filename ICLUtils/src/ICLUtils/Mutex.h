/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Mutex.h                          **
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

#include <pthread.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/CompatMacros.h>

namespace icl{
  namespace utils{
  
    /** \cond */
    class Lockable;
    /** \endcond */  
  
    /// Mutex class of the ICL \ingroup THREAD
    /** This mutex class is a simple object oriented wrapper of the
        pthread_mutex_t struct.
        
        Mutices can be:
        - created (pthread_mutex_init)
        - locked (pthread_mutex_lock)
        - unlocked (pthread_mutex_unlock)
        - and destroyed (Destructor)-> (pthread_mutex_destroy)
    **/
    class ICL_UTILS_EXP Mutex : public Uncopyable{
      public:
  
      /// This enum holds available mutex types.
      enum MutexType {
      //#ifndef ICL_SYSTEM_WINDOWS
        /// normal mutex can not be locked by owner before unlocking
        mutexTypeNormal = PTHREAD_MUTEX_NORMAL,
        /// recursive mutex can be locked repeatedly by owner-thread. needs equal unlocks.
        mutexTypeRecursive = PTHREAD_MUTEX_RECURSIVE
      //#else
  
      //#endif
      } type;
  
      /// Create a mutex
      /**
           @param type The default MutexType is MutexType::mutexTypeNormal
      **/
      Mutex(MutexType type = mutexTypeNormal):type(type){
  	  //#ifndef ICL_SYSTEM_WINDOWS
        pthread_mutexattr_init(&a);
        pthread_mutexattr_settype(&a, type);
        pthread_mutex_init(&m,&a);
  	  //#else
  	  
  	  //#endif
      }
      /// Destroys the mutex
      ~Mutex(){
  	  //#ifndef ICL_SYSTEM_WINDOWS
          pthread_mutex_destroy(&m);
  	  //#else
  	  
  	  //#endif
      }
      /// locks the mutex
      void lock(){
  	  //#ifndef ICL_SYSTEM_WINDOWS
        pthread_mutex_lock(&m);
  	  //#else
  	  
  	  //#endif
      }
      /// locks the mutex without blocking. returns immediately.
      /**
          @return zero if lock is acquired. otherwise an error-number
      **/
      int trylock(){
        //#ifndef ICL_SYSTEM_WINDOWS
        return pthread_mutex_trylock(&m);
        //#else
  
        //#endif
      }
  
      /// unlocks the mutex
      void unlock(){
  	  //#ifndef ICL_SYSTEM_WINDOWS
        pthread_mutex_unlock(&m);
  	  //#else
  	  
  	  //#endif
      }
      
      /// Locks a mutex on the stack (mutex is unlocked when the stack's section is released
      class ICL_UTILS_EXP Locker : public Uncopyable{
        public:
        /// Locks the given mutex until the section is leaved
        Locker(Mutex *m);
  
        /// Locks the given mutex until the section is leaved
        Locker(Mutex &m);
  
        /// Locks given lockable until destruction
        Locker(const Lockable *l);
  
        /// Locks given lockable until destruction
        Locker(const Lockable &l);
  
        /// unlocks the given mutex (automatically called for objects on the stack)
        ~Locker();
        private:
        /// wrapped mutex
        Mutex *m;
      };
      private:
      /// wrapped thread_mutex_t struct
      pthread_mutex_t m;
      /// wrapped thread_mutexattr struct
      pthread_mutexattr_t a;
    };
    
    
    /** \cond inline implementation of the embedded Locker class */
    inline Mutex::Locker::Locker(Mutex *m):m(m){
      m->lock();
    }
    inline Mutex::Locker::Locker(Mutex &m):m(&m){
      m.lock();
    }
    inline Mutex::Locker::~Locker(){
      m->unlock();
    }
    /** \endcond */
  } // namespace utils
}

