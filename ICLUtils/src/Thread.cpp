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

#include <ICLUtils/Thread.h>
//#include <stdio.h>
#include <ICLUtils/Macros.h>
//#include <sched.h>
#include <pthread.h>
#ifdef SYSTEM_LINUX
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
  void Thread::unlock(){
    impl->mutex.unlock();
  }
  
  void Thread::usleep(unsigned int usec){
    ::usleep(usec);
  }

  void Thread::msleep(unsigned int msecs){
#ifndef SYSTEM_WINDOWS
    usleep(msecs*1000);
#else
	  //System::Threading::Thread::Sleep(msecs);
    sleep(msecs);
#endif
  }
  void Thread::sleep(float secs){
#ifndef SYSTEM_WINDOWS
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
      pthread_exit(impl->data);
    }
  }
  
  void *icl_thread_handler(void *t){
    ((Thread*)t)->run();
    pthread_exit(0);
    return 0;
  }


}
