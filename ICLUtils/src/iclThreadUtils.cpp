#include "iclThreadUtils.h"
#include <vector>

namespace icl{

  struct ExecThreadImpl : public Thread{
    std::vector<ExecThread::callback> cbs;
    bool looped;
    
    inline void one_cycle(){
      for(int i=0;i<cbs.size();++i){
        cbs[i]();
      }
      /// allow thread join from outside!
      usleep(0);
    }
    
    virtual void run(){
      if(looped){
        while(true){
          one_cycle();
        }
      }else{
        one_cycle();
      }
    }
  };
  
  ExecThread::ExecThread(ExecThread::callback a, 
                         ExecThread::callback b, 
                         ExecThread::callback c, 
                         ExecThread::callback d, 
                         ExecThread::callback e,
                         ExecThread::callback f) throw (ICLException){
    
    if(!a) throw ICLException("ExecThread called with NULL function!");
    
    impl = new ExecThreadImpl;
    impl->cbs.push_back(a);
    
    if(b) impl->cbs.push_back(b);
    if(c) impl->cbs.push_back(c);
    if(d) impl->cbs.push_back(d);
    if(e) impl->cbs.push_back(e);
    if(f) impl->cbs.push_back(f);
  }
  
 
  ExecThread::~ExecThread(){
    ICL_DELETE(impl);
  }
  
  void ExecThread::run(bool looped){
    impl->looped = looped;
    impl->start();
  }


  // older misc

  namespace{
    static std::vector<Thread*> g_registered_threads_vec;
  }
  
  void register_thread(Thread *t){
    g_registered_threads_vec.push_back(t);
  }
  
  void stop_all_threads(){
    for(unsigned int i=0;i<g_registered_threads_vec.size();++i){
      g_registered_threads_vec[i]->stop();
    }
  }


}

