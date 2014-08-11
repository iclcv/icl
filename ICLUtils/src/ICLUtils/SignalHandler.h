/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/SignalHandler.h                  **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Macros.h>
#include <string>
#include <vector>
#include <ICLUtils/Function.h>

namespace icl{
  namespace utils{
    
    /// C++ Signal-Handler interface class \ingroup UTILS
    /** Just create an own signal handler class,  implement its handleSignal()
        function and create a static object of that signal handler.
        
        example:
        \code
        class MySignalHandler : public icl::utils::SignalHandler{
          public:
          MySignalHandler():SignalHandler("SIGINT,SIGSEGV"){}
          virtual void handleSignals(const string &signal){
             if(signal == "SIGINT") printf("application interrupted! \n");
             else printf("Oops something went wrong ...! \n");
          }
        };
        \endcode

        The handleSignal() function must not exit the program. This will be
        done auomatically.
    */
    class ICLUtils_API SignalHandler{
      public:
      /// Create a new Signal handler with a list of signals
      /** The default parameters can be used to catch some common signals
          that may occur, when the program is uncommonly killed.
          
          @param signals comma-separated list of string representations of 
          the following signals:
          - <b>SIGABRT</b> ( process abort signal)
          - <b>SIGALRM</b> ( Alarm clock)
          - <b>SIGBUS</b>  ( Access to an undefined portion of a memory object)
          - <b>SIGCHLD</b> ( Child process terminated, stopped or continued)
          - <b>SIGCONT</b> ( Continue executing, if stopped)
          - <b>SIGFPE</b>  ( Erroneous arithmetic operation)
          - <b>SIGHUP</b>  ( Hangup )
          - <b>SIGILL</b>  ( Illegal instruction )
          - <b>SIGINT</b>  ( Terminal interrupt signal )
          - <b>SIGKILL</b> ( <b>Kill</b> (cannot be caught or ignored)
          - <b>SIGPIPE</b> ( Write on a pipe with no one to read it)
          - <b>SIGQUIT</b> ( Terminal quit signal )
          - <b>SIGSEGV</b> ( Invalid memory reference )
          - <b>SIGSTOP</b> ( <b>Stop executing</b> (cannot be caught or ignored) )
          - <b>SIGTERM</b> ( Termination signal )
          - <b>SIGTSTP</b> ( Terminal stop signal)
          - <b>SIGTTIN</b> ( Background process attempting read)
          - <b>SIGTTOU</b> ( Background process attempting write)
          - <b>SIGUSR1</b> ( User-defined signal 1)
          - <b>SIGUSR2</b> ( User-defined signal 2)
          - <b>SIGPOLL</b> ( Pollable event)
          - <b>SIGPROF</b> ( Profiling timer expired)
          - <b>SIGSYS</b>  ( Bad system call)
          - <b>SIGTRAP</b> ( Trace/breakpoint trap )
          - <b>SIGURG</b>  ( High bandwidth data is available at a socket)
          - <b>SIGVTALRM</b> ( Virtual timer expired)
          - <b>SIGXCPU</b> ( CPU time limit exceeded) 
          - <b>SIGXFSZ</b> ( File size limit exceeded)
      */
      private:
      SignalHandler();

      // todo: later the constructor should be made private!
      //public:
      /// this cannot be instantiated manually! Use SignalHandler::install instead
      //SignalHandler(const std::string &signalsList="SIGINT,SIGHUP,SIGTERM,SIGSEGV");
      

      public:
      //friend class NamedCallbackHandler;
      
      /// installs a handler to the given signals!
      /** several handlers can be installed to the same signals.
          If a handler is installed twice under the same ID, the
          handler installation is skipped! */
      static void install(const std::string &id,
                          Function<void,const std::string&> handler,
                          const std::string &signalList="SIGINT,SIGTERM,SIGSEGV");
      
      static void uninstall(const std::string &id);
      
      
      /// Destructor
      /** When the destructor is called the system default signal handlers are
          substituted instead of the handleSignals function */
      // virtual ~SignalHandler();
      
      /// virtual signal handling function
      /** The SignalHandler implementation will track all instantiated
          subclassed and, on signal, call the handleSignals functions
          and in the end kill the process with the Hangup-signal.
      **/
      // virtual void handleSignals(const std::string &signalAsString)=0;
  
      /// removes the signal handle for this instance
      /** If this is the last instance registered to a certain signal, the
          sygnal handle will be released and the default signal handlers are
          registered.
      **/
      // void removeHandle(std::string signalName);

      /// removes all handles for this instance
      /** Basically calls removeHandle with all registeded handle names
      **/
      //void removeAllHandles();

      /// calls the original action which was associated to the corresponding signal
      /** This seems to be not practible, as the old actions are not defined by
          callable functions in the old action sigaction struct. */
      //void oldAction(const std::string &signal);
        
      //private:
      /// internal storage of associated signals
      //std::vector<int> m_vecAssocitatedSignals;
    };
  
  } // namespace utils
} // namespace icl
