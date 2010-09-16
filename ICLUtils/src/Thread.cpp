/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/Thread.cpp                                **
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

#include <ICLUtils/Thread.h>
//#include <stdio.h>
#include <ICLUtils/Macros.h>
//#include <sched.h>
#include <pthread.h>
#ifndef ICL_SYSTEM_APPLE
#include <unistd.h>
#endif

namespace icl{
  
  struct ThreadImpl{
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
	  #ifndef ICL_SYSTEM_WINDOWS
      pthread_create(&impl->thread,0,icl_thread_handler, (void*)this); 
	  #else
	  
	  #endif
    }
  }
  void Thread::stop(){
    Mutex::Locker l(impl->mutex);
    if(impl->on){
	  #ifndef ICL_SYSTEM_WINDOWS
      pthread_cancel(impl->thread);
      pthread_join(impl->thread,&impl->data);
	  #else
	  
	  #endif
      impl->on = false;
      finalize();
    }
  }
  void Thread::wait(){
    Mutex::Locker l(impl->mutex);
    if(impl->on){
	  #ifndef ICL_SYSTEM_WINDOWS
      pthread_join(impl->thread,&impl->data);
	  #else
	  
	  #endif
      impl->on = false;
      finalize();
    }
  }
  void Thread::lock(){
    impl->mutex.lock();
  }
  void Thread::unlock(){
    impl->mutex.unlock();
  }
  
  void Thread::usleep(unsigned int usec){
	#ifndef ICL_SYSTEM_WINDOWS
    ::usleep(usec);
	#else
	//TODO is this really ok?
	sleep(usec);
	#endif
  }

  void Thread::msleep(unsigned int msecs){
#ifndef ICL_SYSTEM_WINDOWS
    usleep(msecs*1000);
#else
	  //System::Threading::Thread::Sleep(msecs);
    sleep(msecs);
#endif
  }
  void Thread::sleep(float secs){
#ifndef ICL_SYSTEM_WINDOWS
    ::usleep((long)secs*1000000);
#else
    sleep(secs*1000);
//	  System::Threading::Thread::Sleep(secs*1000);
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
	  #ifndef ICL_SYSTEM_WINDOWS
      pthread_exit(impl->data);
	  #else
	  
	  #endif
    }
  }
  
  void *icl_thread_handler(void *t){
    ((Thread*)t)->run();
	#ifndef ICL_SYSTEM_WINDOWS
    pthread_exit(0);
	#else
	
	#endif
    return 0;
  }
}
