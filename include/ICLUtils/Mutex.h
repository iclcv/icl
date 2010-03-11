/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
** University of Bielefeld                                                **
** Contact: nivision@techfak.uni-bielefeld.de                             **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

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
