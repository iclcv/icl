#include "iclThread.h"
//#include <stdio.h>
#include "iclMacros.h"
//#include <sched.h>
#include <pthread.h>
#include <unistd.h>


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
  
  void Thread::msleep(unsigned int msecs){ 
    usleep(msecs*1000); 
  }
  void Thread::sleep(float secs){
    usleep((long)secs*1000000); 
  }
  
  void *icl_thread_handler(void *t){
    ((Thread*)t)->run();
    pthread_exit(0);
    return 0;
  }

}
