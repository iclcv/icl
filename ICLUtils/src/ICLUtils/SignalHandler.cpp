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
#ifdef ICL_SYSTEM_WINDOWS
namespace icl{
  namespace utils{
    void SignalHandler::install(const std::string &, Function<void,const std::string&>,
                                const std::string &){}
    
    void SignalHandler::uninstall(const std::string &){}
#else

#include <ICLUtils/SignalHandler.h>
#include <ICLUtils/StrTok.h>
#include <vector>
#include <map>
#include <set>
#include <signal.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Lockable.h>
#include <ICLUtils/StringUtils.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

#ifdef ICL_SYSTEM_APPLE
#define SIGPOLL SIGIO
#endif

namespace icl{
  namespace utils{
    struct SignalHandlerContext : public Lockable{
      SignalHandlerContext() : Lockable(true){
#define A_(S) add(str("SIG") + #S,SIG##S) 
        A_(ABRT); A_(ALRM); A_(BUS);  A_(CHLD); A_(CONT); A_(FPE);  A_(HUP);  A_(ILL);    A_(INT);
        A_(KILL); A_(PIPE); A_(QUIT); A_(SEGV); A_(STOP); A_(TERM); A_(TSTP); A_(TTIN);   A_(TTOU);
        A_(USR1); A_(USR2); A_(POLL); A_(PROF); A_(SYS);  A_(TRAP); A_(URG);  A_(VTALRM); A_(XCPU); A_(XFSZ);
#undef A_
      }
      int t(const std::string &s) const{
        std::map<std::string,int>::const_iterator it = keys.find(s);
        if(it != keys.end()){
          return it->second;
        }
        else{
          ERROR_LOG("undefined signal \"" << s << "\"");
          return -1;
        }
      }
      const std::string &t(int signal) const{
        std::map<int,std::string>::const_iterator it = names.find(signal);
        if(it != names.end()){
          return (*it).second;
        }else{
          ERROR_LOG("undefined signal \"" << signal << "\n");
          static const std::string udef = "[undefined signal]";
          return udef;
        }
      }
      
      void handle(int signal){
        Mutex::Locker lock(this);
        std::string name = t(signal);
        for(HMap::iterator it = handlers.begin(); it != handlers.end(); ++it){
          if(it->second.count(signal)){
            it->second.f(name);
          }
        }        
      }
      
      struct Handler : public std::set<int>{
        Function<void,const std::string&> f;
      };

    private:
      void add(const std::string &s, int signal){
        keys[s]=signal;
        names[signal]=s;
      }
      std::map<int,std::string> names;
      std::map<std::string,int> keys;
      
    public:
      std::map<int,int> numHandlers; // for given signal key!
      typedef std::map<std::string, Handler> HMap;
      HMap handlers;
    };
    
    static SignalHandlerContext &ctx(){
      static SignalHandlerContext instance;
      return instance;
    }
    
  
    
    static void register_low_level_handler(void (*handler)(int, siginfo_t*, void*), int signal){
      Mutex::Locker lock(ctx());
      
      struct sigaction action;
      memset(&action, 0, sizeof(action));
      if (handler) {
        action.sa_sigaction = handler;
        action.sa_flags = SA_SIGINFO;
      } else {
        action.sa_handler = SIG_DFL;
      }
      sigaction(signal, &action, NULL);
    }

    static void low_level_handler(int signum, siginfo_t* info, void *ptr) {
      static bool bQuitting = false;
      if (bQuitting) exit (EXIT_FAILURE); // signal during exit
      bQuitting = true;
      
      //DEBUG_LOG("handling signal " << ctx().t(signum));
      ctx().handle(signum);
      //DEBUG_LOG("done with signal " << ctx().t(signum));
      
      // bQuitting = false; ??
    }

    void SignalHandler::install(const std::string &id, Function<void,const std::string&> f,
                                const std::string &signals){

      SignalHandlerContext &c = ctx();      
      Mutex::Locker lock(c);

      if(c.handlers.find(id) != c.handlers.end()){
        throw ICLException("SingnalHandler with id " + id 
                           + " was registered twice");
      }
         
      
      std::vector<std::string> names = tok(signals, ",");
      
      SignalHandlerContext::Handler &h = c.handlers[id];
      h.f = f;
      
      for(size_t i=0;i<names.size();++i){
        int signal =  c.t(names[i]);
        if(c.numHandlers.find(signal) == c.numHandlers.end()){
          register_low_level_handler(low_level_handler, signal);
          c.numHandlers[signal] = 1;
        }else{
          c.numHandlers[signal]++;
        }
        h.insert(signal);
      }

    }
    
    void SignalHandler::uninstall(const std::string &id){
      SignalHandlerContext &c = ctx();      
      Mutex::Locker lock(c);
      
      if(c.handlers.find(id) != c.handlers.end()){
        throw ICLException("SingnalHandler with id " + id 
                           +" cannot be uninstalled since it was never installed");
      }
      SignalHandlerContext::Handler &h = c.handlers[id];
      
      std::vector<int> rm;
      for(std::set<int>::iterator it = h.begin(); it != h.end();++it){
        int &n = c.numHandlers[*it];
        if(!--n){
          rm.push_back(*it);
        }
      }

      for(size_t i=0;i<rm.size();++i){
        register_low_level_handler(0, rm[i]); // no handlers left
        c.numHandlers.erase(rm[i]);           // ensure that next call will re-install a low-level handler
      }
      
      // remove the Handler instance
      c.handlers.erase(id);
    }
    
    
#if 0
    void signal_handler_function(int signal){
      Mutex::Locker l(SignalHandlerMutex);
      shMap::iterator begin = SHM.lower_bound(signal);
      shMap::iterator end = SHM.upper_bound(signal);
      std::string str = SSM.getString(signal);
      for(; begin != end; ++begin){
        //        while(SHM.count(signal)){
        //shMap::iterator it = SHM.find(signal);
        // SignalHandler *sh = (*it).second;
        begin->second->handleSignals(str);
        //          sh->handleSignals(SSM.getString(signal));
        // sh->removeAllHandles();
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

#endif
  } // namespace utils
  
}

#endif
