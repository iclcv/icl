/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/SignalHandler.cpp                **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Viktor Richter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLUtils/SignalHandler.h>
#include <ICLUtils/StrTok.h>
#include <vector>
#include <map>
#include <signal.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Mutex.h>
#include <sys/types.h>
#ifndef ICL_SYSTEM_WINDOWS
  #include <unistd.h>
#endif
#include <cstdio>

#ifdef ICL_SYSTEM_APPLE
#define SIGPOLL SIGIO
#endif

using namespace std;

namespace icl{
  namespace utils{

    namespace {
      static Mutex SignalHandlerMutex(Mutex::mutexTypeRecursive);
      
#ifdef ICL_SYSTEM_WINDOWS // TODOW
      struct sigaction {
        //int          sa_flags;
        //sigset_t     sa_mask;
        //__p_sig_fn_t sa_handler;   /* see mingw/include/signal.h about the type */
        int sa_flags;
        int sa_mask;
        void (*sa_handler)(int);
      };
#define sigemptyset(pset)    (*(pset) = 0)
      int sigaction(int signum, const struct sigaction *act, struct sigaction *oact) {
        /*void(*handler)(int);

        if (act && oact) {
          handler = signal(signum, act->sa_handler);
          if (handler == SIG_ERR) return -1;
          oact->sa_handler = handler;
        } if (act && (oact == NULL)) {
          handler = signal(signum, act->sa_handler);
          if (handler == SIG_ERR) return -1;
        }
        else if ((act == NULL) && oact) {
          if ((handler = signal(signum, SIG_IGN)) == SIG_ERR) return -1;
          oact->sa_handler = handler;
          handler = signal(signum, oact->sa_handler);
          if (handler == SIG_ERR) return -1;
        }
        */
        return 0;
      }
      int kill(pid_t pid, int signal);
#endif
      
      typedef map<string,int> stringSignalMap;
      typedef map<int,string> signalStringMap;
      typedef multimap<int,SignalHandler*> shMap;
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
        Mutex::Locker l(SignalHandlerMutex);
        while(SHM.count(signal)){
          shMap::iterator it = SHM.find(signal);
          SignalHandler *sh = (*it).second;
          sh->handleSignals(SSM.getString(signal));
          sh->removeAllHandles();
        }
#ifndef ICL_SYSTEM_WINDOWS
        kill(getpid(),1);
#else

#endif
      }

    
      
    }

    struct NamedCallbackHandler : public SignalHandler{
      Function<void,const std::string&> cb;
      NamedCallbackHandler(const std::string &signalList,
                           Function<void,const std::string&> cb):SignalHandler(signalList),cb(cb){}
      virtual void handleSignals(const std::string &signalAsString){
        cb(signalAsString);
      }
    };
      
    namespace{
      typedef std::map<std::string,SmartPtr<NamedCallbackHandler> > NamedHandlersMap;
      
      SmartPtr<NamedHandlersMap> g_namedHandlers;
      NamedHandlersMap &named(){
        if(!g_namedHandlers) g_namedHandlers = new NamedHandlersMap;
        return *g_namedHandlers;
      }
    }

    
    void SignalHandler::install(const std::string &id,
                                Function<void,const std::string&> handler,
                                const std::string &signalList){
      Mutex::Locker l(SignalHandlerMutex);
      if(named().find(id) != named().end()) return;
      named()[id] = SmartPtr<NamedCallbackHandler> (new NamedCallbackHandler( signalList, handler) );
    }
    void SignalHandler::uninstall(const std::string &id){
      Mutex::Locker l(SignalHandlerMutex);
      NamedHandlersMap::iterator it = named().find(id);
      if(it == named().end()){
        ERROR_LOG("unable to find named signal handler " << id);
      }else{
        named().erase(it);
      }
    }

    SignalHandler::SignalHandler(const std::string &signalsList){
      Mutex::Locker l(SignalHandlerMutex);
      StrTok t(signalsList,",");
      const vector<string> &toks = t.allTokens();
      for(unsigned int i=0;i<toks.size();i++){
        int signal = SSM.getSignal(toks[i]);

        if(SHM.find(signal) != SHM.end()){
          DEBUG_LOG2("handler already initialized. adding " << this << " as additinoal handler for " << toks[i] << "(" << signal << ")");
          SHM.insert(pair<int,SignalHandler*>(signal,this));
          m_vecAssocitatedSignals.push_back(signal);
        } else {
          DEBUG_LOG2("initializing and adding " << this << " as handler for " << toks[i] << "(" << signal << ")");
          struct sigaction new_action;
          struct sigaction *old_action = new struct sigaction;

          new_action.sa_handler = signal_handler_function;
          sigemptyset (&new_action.sa_mask);
          new_action.sa_flags = 0;
          sigaction (signal, NULL, old_action);

          if (old_action->sa_handler != SIG_IGN){
            sigaction (signal, &new_action, NULL);
            SHM.insert(pair<int,SignalHandler*>(signal,this));
            SAM[signal]=old_action;
            m_vecAssocitatedSignals.push_back(signal);
          } else {
            ERROR_LOG("this signal can not be handle because it was ignored before: \"" << toks[i] << "\"");
            delete old_action;
          }
          /// sigaction code::
        }
      }
    }

    void SignalHandler::removeHandle(std::string signalName){
      int signal = SSM.getSignal(signalName);
      std::vector<int>::iterator it;
      for(std::vector<int>::iterator it = m_vecAssocitatedSignals.begin();
          it != m_vecAssocitatedSignals.end(); ++it){
        if(*it == signal){
          SignalHandlerMutex.lock();
          // remove signal from list
          m_vecAssocitatedSignals.erase(it);
          // deregister from signal
          int handlerCount = SHM.count(signal);
          DEBUG_LOG2("associated: " << signal << " count: " << SHM.count(signal));
          std::pair<shMap::iterator,shMap::iterator> handlers = SHM.equal_range(signal);
          for(shMap::iterator it = handlers.first; it != handlers.second;){
            if(it->second == this){
              DEBUG_LOG2("found " << this << " in handler map. removing " << signal);
              shMap::iterator save = it;
              ++save;
              SHM.erase(it);
              it = save;
            } else {
              ++it;
            }
          }
          if(handlerCount == 1){
            DEBUG_LOG2("deleting system signal handler " << signal);
            struct sigaction *old_action = SAM[signal];
            sigaction (signal, old_action, NULL);
            delete old_action;
            SAM.erase(SAM.find(signal));
          }
          SignalHandlerMutex.unlock();
          return;
        }
      }
    }

    void SignalHandler::removeAllHandles(){
      while(m_vecAssocitatedSignals.size()){
        removeHandle(SSM.getString(m_vecAssocitatedSignals[0]));
      }
    }

    SignalHandler::~SignalHandler(){
      removeAllHandles();
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
		
    /*void SignalHandler::killCurrentProcess() {
#ifndef ICL_SYSTEM_WINDOWS
      kill(getpid(),1);
#else
	
#endif
    }*/
  } // namespace utils
  
}
