#include "iclThreadUtils.h"
#include <vector>

namespace icl{

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
