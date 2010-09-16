/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/Mutex.h                               **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_MUTEX_H
#define ICL_MUTEX_H
#include <pthread.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{

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
  class Mutex : public Uncopyable{ 
    public:
    /// Create a mutex
    Mutex(){
	  #ifndef ICL_SYSTEM_WINDOWS
		pthread_mutex_init(&m,0);
	  #else
	  
	  #endif
    }
    /// Destroys the mutex
    ~Mutex(){
	  #ifndef ICL_SYSTEM_WINDOWS
        pthread_mutex_destroy(&m);
	  #else
	  
	  #endif
    }
    /// locks the mutex
    void lock(){
	  #ifndef ICL_SYSTEM_WINDOWS
      pthread_mutex_lock(&m);
	  #else
	  
	  #endif
    }

    /// unlocks the mutex
    void unlock(){
	  #ifndef ICL_SYSTEM_WINDOWS
      pthread_mutex_unlock(&m);
	  #else
	  
	  #endif
    }
    
    /// Locks a mutex on the stack (mutex is unlocked when the stack's section is released
    class Locker : public Uncopyable{
      public:
      /// Locks the given mutex until the section is leaved
      Locker(Mutex *m);

      /// Locks the given mutex until the section is leaved
      Locker(Mutex &m);

      /// Locks given lockable unti destruction
      Locker(Lockable *l);

      /// Locks given lockable unti destruction
      Locker(Lockable &l);

      /// unlocks the given mutex (automatically called for objects on the stack)
      ~Locker();
      private:
      /// wrapped mutex
      Mutex *m;
    };
    private:
    /// wrapped thread_mutex_t struct
    pthread_mutex_t m;
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
}

#endif
