#include "iclThread.h"
#include <stdio.h>
#include "iclMacros.h"
#include <sched.h>

namespace icl{
  
 
  void *icl_thread_handler(void *thread){
    Thread* t = (Thread*)thread;
    t->m_bRunning=true;
    t->run();
    t->stop();
    return 0;
  }
  
  namespace {
    void get_prio_interval(int &minp, int &maxp, pthread_attr_t *attr){
      int sched_policy;
      if (pthread_attr_getschedpolicy(attr, &sched_policy) != 0) {
        // failed to get the scheduling policy, don't bother
        // setting the priority
        WARNING_LOG("Thread::start: Cannot determine default scheduler policy");
        minp = maxp = 0;
        return;
      }
      minp = sched_get_priority_min(sched_policy);
      maxp = sched_get_priority_max(sched_policy);
      if (minp == -1 || maxp == -1) {
        // failed to get the scheduling parameters, don't
        // bother setting the priority
        WARNING_LOG("Thread::start: Cannot determine scheduler priority range");
      }
    }
  }
  
  Thread::Thread(Thread::priority p): m_bRunning(false){
    pthread_cond_init(&m_oWaitCond,0);
    pthread_mutex_init(&m_oWaitMutex,0);
    pthread_attr_init(&m_oAttr); 
    pthread_attr_setdetachstate(&m_oAttr, PTHREAD_CREATE_DETACHED); // ???

    if(p==inherit){
      pthread_attr_setinheritsched(&m_oAttr, PTHREAD_INHERIT_SCHED);
    }else{
      int prio(0),minp(0),maxp(0);
      get_prio_interval(minp,maxp,&m_oAttr);
      switch(p){
        case idle: prio = minp; break;
        case critical: prio = maxp; break;
        default: 
          prio = (((maxp - minp) / critical) * p) + minp;
          prio = std::max(minp,std::min(maxp,prio));
          break;
      }
      sched_param sp;
      sp.sched_priority = prio;
      pthread_attr_setinheritsched(&m_oAttr, PTHREAD_EXPLICIT_SCHED);
      pthread_attr_setschedparam(&m_oAttr, &sp);
    }
  }
  Thread::~Thread(){
    stop();
    pthread_cond_destroy(&m_oWaitCond);
    pthread_mutex_destroy(&m_oWaitMutex);
  }
  
  void Thread::start(){
    pthread_create(&m_oPT, &m_oAttr, icl_thread_handler, (void*)this); 
  }
  
  void Thread::stop(){
    lock();
    if(m_bRunning){
      void *data;
      pthread_cancel(m_oPT);
      pthread_join(m_oPT,&data);
      m_bRunning = false;
      pthread_cond_broadcast(&m_oWaitCond);
      unlock();
      finalize();
    }else{
      unlock();
    }
  }
    
  void Thread::run(){
    printf("the virtual Thread Function run() has been called! \n");
    printf("please reimplement this function to make the thread perform \n");
    printf("some special operations! \n");
  }

}
