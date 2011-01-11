/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/SignalHandler.cpp                         **
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

#include <ICLUtils/SignalHandler.h>
#include <ICLUtils/StrTok.h>
#include <vector>
#include <map>
#include <signal.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Mutex.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef ICL_SYSTEM_APPLE
#define SIGPOLL SIGIO
#endif

using namespace std;

namespace icl{
  namespace {
    static Mutex SignalHandlerMutex;
    
#ifdef ICL_SYSTEM_WINDOWS
	struct sigaction {
    int          sa_flags;
    sigset_t     sa_mask;
    __p_sig_fn_t sa_handler;   /* see mingw/include/signal.h about the type */
};
#define sigemptyset(pset)    (*(pset) = 0)
int sigaction(int signum, const struct sigaction *act, struct sigaction *oact);
int kill(pid_t pid, int signal);
#endif

    typedef map<string,int> stringSignalMap;
    typedef map<int,string> signalStringMap;
    typedef map<int,SignalHandler*> shMap;
    typedef map<int,struct sigaction*> saMap;
#define ADD_SIGNAL(S) add(#S,S) 
    
    class StringSignalMap{
    public:
	#ifndef ICL_SYSTEM_WINDOWS
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
	#else
	  StringSignalMap(){
        ADD_SIGNAL(SIGABRT); ADD_SIGNAL(SIGFPE);
        ADD_SIGNAL(SIGILL);  ADD_SIGNAL(SIGINT);
        ADD_SIGNAL(SIGSEGV); ADD_SIGNAL(SIGTERM);
      }
	#endif
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
      #ifndef ICL_SYSTEM_WINDOWS
      sigaction (signal, NULL, old_action);
	  #else
	  
	  #endif
      if (old_action->sa_handler != SIG_IGN){
	    #ifndef ICL_SYSTEM_WINDOWS
        sigaction (signal, &new_action, NULL);
        #else
		
		#endif
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
	  #ifndef ICL_SYSTEM_WINDOWS
      sigaction (s, old_action, NULL);
	  #else
	  
	  #endif
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
		#ifndef ICL_SYSTEM_WINDOWS
        old_sigaction->sa_handler != SIG_HOLD &&
		#endif
        old_sigaction->sa_handler != SIG_IGN ){
      old_sigaction->sa_handler(s);
    }
  }
		
  void SignalHandler::killCurrentProcess() {
	#ifndef ICL_SYSTEM_WINDOWS
    kill(getpid(),1);
	#else
	
	#endif
  }
}
