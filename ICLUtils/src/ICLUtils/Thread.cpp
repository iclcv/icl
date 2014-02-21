/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Thread.cpp                       **
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

#include <ICLUtils/Thread.h>
//#include <stdio.h>
#include <ICLUtils/Macros.h>
//#include <sched.h>
#include <pthread.h>
#ifndef ICL_SYSTEM_APPLE
#ifdef ICL_SYSTEM_WINDOWS
  #include <Windows.h>
#else
  #include <unistd.h>
#endif
#endif

namespace icl{
  namespace utils{
    
    class ThreadImpl{
    public:
      pthread_t thread;
      Mutex mutex;
      bool on;
      void *data;
    };
    
    void ThreadImplDelOp::delete_func(ThreadImpl* impl){
      ICL_DELETE(impl);
    }
    
    void *icl_thread_handler(void *t);
  
    
    Thread::Thread():ShallowCopyable<ThreadImpl,ThreadImplDelOp>(new ThreadImpl){
      impl->on = false;
      impl->data = 0;
    }
    
    Thread::~Thread(){
      stop();
    }
    
    void Thread::start(){
      Mutex::Locker l(impl->mutex);
      if(impl->on){
        ERROR_LOG("unable to start thread (it's still running)");
      }else{
        impl->on = true;
  	  pthread_create(&impl->thread,0,icl_thread_handler, (void*)this); 
  	}
    }
    void Thread::stop(){
      Mutex::Locker l(impl->mutex);
      if(impl->on){
        pthread_cancel(impl->thread);
        pthread_join(impl->thread,&impl->data);
  	  impl->on = false;
        finalize();
      }
    }
    void Thread::wait(){
      Mutex::Locker l(impl->mutex);
      if(impl->on){
  	  pthread_join(impl->thread,&impl->data);
  	  impl->on = false;
        finalize();
      }
    }
    void Thread::lock(){
      impl->mutex.lock();
    }
    int Thread::trylock(){
      return impl->mutex.trylock();
    }
    void Thread::unlock(){
      impl->mutex.unlock();
    }
    
    void Thread::usleep(unsigned int usec){
    #ifdef ICL_SYSTEM_WINDOWS
      Sleep(usec / 1000);
    #else
      ::usleep(usec);
  	#endif
    }
  
    void Thread::msleep(unsigned int msecs){
  #ifdef ICL_SYSTEM_WINDOWS
      Sleep(msecs);
  #else
      usleep(msecs*1000);
  #endif
    }
    void Thread::sleep(float secs){
  #ifdef ICL_SYSTEM_WINDOWS
      Sleep(secs*1000);
  #else
      ::usleep((long)secs * 1000000);
  #endif
    }
  
    // Maybe this works
    bool Thread::running() const{
      Mutex::Locker l(const_cast<Mutex&>(impl->mutex));
      return impl->on;
    }
    void Thread::exit(){
      Mutex::Locker l(impl->mutex);
      if(impl->on){
        impl->on = false;
  	  pthread_exit(impl->data);
      }
    }
    
    void *icl_thread_handler(void *t){
      ((Thread*)t)->run();
  	pthread_exit(0);
  	return 0;
    }
  } // namespace utils
}
