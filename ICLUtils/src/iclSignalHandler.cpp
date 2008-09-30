#include "iclSignalHandler.h"
#include <iclStrTok.h>
#include <vector>
#include <map>
#include <signal.h>
#include <iclMacros.h>
#include <iclMutex.h>

#ifdef SYSTEM_APPLE
#define SIGPOLL SIGIO
#endif

using namespace std;

namespace icl{
  namespace {
    static Mutex SignalHandlerMutex;
    
    typedef map<string,int> stringSignalMap;
    typedef map<int,string> signalStringMap;
    typedef map<int,SignalHandler*> shMap;
    typedef map<int,struct sigaction*> saMap;
#define ADD_SIGNAL(S) add(#S,S) 
    
    class StringSignalMap{
    public:
      StringSignalMap(){
        ADD_SIGNAL(SIGABRT); ADD_SIGNAL(SIGALRM); ADD_SIGNAL(SIGBUS);
        ADD_SIGNAL(SIGCHLD); ADD_SIGNAL(SIGCONT); ADD_SIGNAL(SIGFPE);
        ADD_SIGNAL(SIGHUP);  ADD_SIGNAL(SIGILL);  ADD_SIGNAL(SIGINT);
        ADD_SIGNAL(SIGKILL); ADD_SIGNAL(SIGPIPE); ADD_SIGNAL(SIGQUIT);
        ADD_SIGNAL(SIGSEGV); ADD_SIGNAL(SIGSTOP); ADD_SIGNAL(SIGTERM);
        ADD_SIGNAL(SIGTSTP); ADD_SIGNAL(SIGTTIN); ADD_SIGNAL(SIGTTOU);
        ADD_SIGNAL(SIGUSR1); ADD_SIGNAL(SIGUSR2); ADD_SIGNAL(SIGPOLL);
        ADD_SIGNAL(SIGPROF); ADD_SIGNAL(SIGSYS);  ADD_SIGNAL(SIGTRAP);
        ADD_SIGNAL(SIGURG);  ADD_SIGNAL(SIGVTALRM); ADD_SIGNAL(SIGXCPU);
        ADD_SIGNAL(SIGXFSZ);
      }
      int getSignal(const string &s) const{
        stringSignalMap::const_iterator it = strsgn.find(s);
        if(it != strsgn.end()){
          return (*it).second;
        }
        else{
          ERROR_LOG("undefined signal \"" << s << "\"");
          return -1;
        }
      }
      const string &getString(int signal) const{
        signalStringMap::const_iterator it = sgnstr.find(signal);
        if(it != sgnstr.end()){
          return (*it).second;
        }else{
          ERROR_LOG("undefined signal \"" << signal << "\n");
          static string sUndefinedSignal = "undefined signal";
          return sUndefinedSignal;
        }
      }
      
    private:
      void add(const std::string &s, int signal){
        strsgn[s]=signal;
        sgnstr[signal]=s;
      }
      stringSignalMap strsgn;
      signalStringMap sgnstr;
    };
    
    static StringSignalMap SSM;
    static shMap SHM;
    static saMap SAM;
    
    void signal_handler_function(int signal){
      SignalHandlerMutex.lock();
      shMap::iterator it = SHM.find(signal);
      if(it != SHM.end()){
        SignalHandler *sh = (*it).second;
        sh->handleSignals(SSM.getString(signal));
      }else{
        ERROR_LOG("singnal_handler_function was called for \"" << SSM.getString(signal) << "\"");
      }
      SignalHandlerMutex.unlock();
    }
  }
  
  
  
  SignalHandler::SignalHandler(const std::string &signals){
    StrTok t(signals,",");
    const vector<string> &toks = t.allTokens();
    for(unsigned int i=0;i<toks.size();i++){
      int signal = SSM.getSignal(toks[i]);
    
      if(SHM.find(signal) != SHM.end()){
        ERROR_LOG("this signal is already handled by another signal handler: \""<< toks[i]<<"\"");
        continue;
      }
      
      struct sigaction new_action;
      struct sigaction *old_action = new struct sigaction;

      new_action.sa_handler = signal_handler_function;
      sigemptyset (&new_action.sa_mask);
      new_action.sa_flags = 0;
      
      sigaction (signal, NULL, old_action);

      if (old_action->sa_handler != SIG_IGN){
        sigaction (signal, &new_action, NULL);
        
        SHM[signal]=this;    
        SAM[signal]=old_action;
        m_vecAssocitatedSignals.push_back(signal);
      }else{
        ERROR_LOG("this signal can not be handle because it was ignored before: \"" << toks[i] << "\"");
        delete old_action;
        
      }
      /// sigaction code::
    }
  }
  SignalHandler::~SignalHandler(){
    SignalHandlerMutex.lock();
    for(unsigned int i=0;i<m_vecAssocitatedSignals.size();i++){
      int s = m_vecAssocitatedSignals[i];
      SHM.erase(SHM.find(s));
      
      struct sigaction *old_action = SAM[s];
      sigaction (s, old_action, NULL);
      delete old_action;
      SAM.erase(SAM.find(s));
    }
    SignalHandlerMutex.unlock();
  }

  void SignalHandler::oldAction(const std::string &signal){
    int s  = SSM.getSignal(signal);
    struct sigaction* old_sigaction = SAM[s];    
    ICLASSERT_RETURN( old_sigaction );

    if( old_sigaction->sa_handler != SIG_DFL &&
        old_sigaction->sa_handler != SIG_ERR &&
        old_sigaction->sa_handler != SIG_HOLD &&
        old_sigaction->sa_handler != SIG_IGN ){
      old_sigaction->sa_handler(s);
    }
  }

  void SignalHandler::killCurrentProcess() {
    kill(getpid(),1);
  }
}
