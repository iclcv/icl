#include <iclApplication.h>
#include <iclThreadUtils.h>
#include <iclProgArg.h>

namespace icl{
  typedef ICLApplication::callback callback;
  
  ICLApplication::ICLApplication(int n, char **ppc, 
                                 const std::string &paInitString,
                                 callback init, callback run)
    throw (SecondSingeltonException):QApplication(n,ppc){
    if(s_app) throw SecondSingeltonException("only one instance is allowed!");
    if(paInitString != ""){
      pa_init(n,ppc,paInitString);
    }
    s_app = this;
    if(init) addInit(init);
    if(run) addThread(run);
  }
  
  ICLApplication::~ICLApplication(){
    for(unsigned int i=0;i<s_threads.size();++i){
      delete s_threads[i];
    }
    s_app = 0;
    s_threads.clear();
    s_inits.clear();
  }
  
  void ICLApplication::addThread(callback cb){
    ICLASSERT_RETURN(cb);
    s_threads.push_back(new ExecThread(cb));
  }
  void ICLApplication::addInit(callback cb){
    ICLASSERT_RETURN(cb);
    s_inits.push_back(cb);
  }


  
  int ICLApplication::exec(){
    for(unsigned int i=0;i<s_inits.size();++i){
      s_inits[i]();
    }
    for(unsigned int i=0;i<s_threads.size();++i){
      s_threads[i]->run();
    }
    return QApplication::exec();
  }
  
  
  ICLApplication *ICLApplication::s_app(0);
  std::vector<ExecThread*> ICLApplication::s_threads;
  std::vector<callback> ICLApplication::s_inits;

}
