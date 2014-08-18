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
#include <ICLUtils/SignalHandler.h>

namespace icl{
  namespace utils{
    void SignalHandler::install(const std::string &, Function<void, const std::string&>,
                                const std::string &, int){}

    void SignalHandler::uninstall(const std::string &){}
  }
}
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
    
      struct Handler : public std::set<int>{
        Function<void,const std::string&> f;
        int order;

      };
      
      static bool cmp_handler_ptr(Handler *a, Handler *b){
        return a->order < b->order;
      }
      
      void handle(int signal){
        Mutex::Locker lock(this);
        std::string name = t(signal);
        std::vector<Handler*> ordered;
        
        for(HMap::iterator it = handlers.begin(); it != handlers.end(); ++it){
          if(it->second.count(signal)){
            ordered.push_back(&it->second);
          }
        }        
        
        std::sort(ordered.begin(), ordered.end(), cmp_handler_ptr);
        
        for(size_t i=0;i<ordered.size();++i){
          ordered[i]->f(name);
        }
      }
      
    

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
                                const std::string &signals, int order){

      SignalHandlerContext &c = ctx();      
      Mutex::Locker lock(c);

      if(c.handlers.find(id) != c.handlers.end()){
        throw ICLException("SingnalHandler with id " + id 
                           + " was registered twice");
      }
         
      
      std::vector<std::string> names = tok(signals, ",");
      
      SignalHandlerContext::Handler &h = c.handlers[id];
      h.f = f;
      h.order = order;
      
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
    
    
  } // namespace utils
  
}

#endif
