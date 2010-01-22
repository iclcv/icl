#include <ICLQt/Application.h>
#include <ICLUtils/ThreadUtils.h>
#include <ICLUtils/ProgArg.h>

namespace icl{

  typedef ICLApplication::callback callback;
 
  ICLApplication *ICLApplication::s_app(0);
  std::vector<ExecThread*> ICLApplication::s_threads;
  std::vector<callback> ICLApplication::s_inits;
  std::vector<callback> ICLApplication::s_callbacks;
  
  ICLApplication::ICLApplication(int n, char **ppc, 
                                 const std::string &paInitString,
                                 callback init, callback run,
                                 callback run2, callback run3,
                                 callback run4, callback run5)
    throw (SecondSingeltonException){
    if(s_app) throw SecondSingeltonException("only one instance is allowed!");
    if(paInitString != ""){
      pa_init(n,ppc,paInitString);
    }
    app = new QApplication(n,ppc);
    s_app = this;
    if(init) addInit(init);
    
    if(run) s_callbacks.push_back(run);
    if(run2) s_callbacks.push_back(run2);
    if(run3) s_callbacks.push_back(run3);
    if(run4) s_callbacks.push_back(run4);
    if(run5) s_callbacks.push_back(run5);
  }
  
  ICLApplication::~ICLApplication(){
    for(unsigned int i=0;i<s_threads.size();++i){
      delete s_threads[i];
    }
    s_app = 0;
    s_threads.clear();
    s_inits.clear();
    s_callbacks.clear();
    delete app;
    

  }
  
  void ICLApplication::addThread(callback cb){
    ICLASSERT_RETURN(cb);
    s_callbacks.push_back(cb);
  }
  void ICLApplication::addInit(callback cb){
    ICLASSERT_RETURN(cb);
    s_inits.push_back(cb);
  }


  
  int ICLApplication::exec(){
    for(unsigned int i=0;i<s_inits.size();++i){
      s_inits[i]();
    }
    for(unsigned int i=0;i<s_callbacks.size();++i){
      s_threads.push_back(new ExecThread(s_callbacks[i]));
    }
    for(unsigned int i=0;i<s_threads.size();++i){
      s_threads[i]->run();
    }
    return app->exec();
  }
  
  

}
