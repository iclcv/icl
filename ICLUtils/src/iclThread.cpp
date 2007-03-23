#include "iclThread.h"
#include <stdio.h>

namespace icl{
  
  namespace{
    void *thread_handler(void *thread){
      Thread* t = (Thread*)thread;
      t->run();
      t->stop();
      return 0;
    }
  }
  
  Thread::Thread(){
    pthread_mutex_init(&m_oMutex,0);
  }
  Thread::~Thread(){
    stop();
  }
  
  void Thread::start(){
    pthread_create(&m_oPT, 0, thread_handler, (void*)this);  
  }
  
  void Thread::stop(){
    lock();
    void *data;
    pthread_join(m_oPT,&data);
    unlock();
  }
  
  
  void Thread::run(){
    printf("the virtual Thread Function run() has been called! \n");
    printf("please reimplement this function to make the thread perform \n");
    printf("some special operations! \n");
  }

}
