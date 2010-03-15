/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLUtils/SignalHandler.h                       **
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
*********************************************************************/

//#include <signal.h>
#include <string>
#include <vector>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Macros.h>

namespace icl{
  
  /// C++ Signal-Handler interface class \ingroup UTILS
  /** Just create an own signal handler class,  implement its handleSignal()
      function and create a static object of that signal handler.
      
      example:
      \code
      class MySignalHandler : public SignalHandler{
        public:
        MySignalHandler():SignalHandler("SIGINT,SIGSEGV"){}
        virtual void handleSignals(const string &signal){
           if(signal == "SIGINT") printf("application interrupted! \n");
           else printf("Oops something went wrong ...! \n");
           killCurrentProcess();
        }
      };
      \endcode
  */
  class SignalHandler{
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
    SignalHandler(const std::string &signals="SIGINT,SIGHUP,SIGTERM,SIGSEGV");
    
    /// Destructor
    /** When the destructor is called the system default signal handlers are
        substituted instead of the handleSignals function */
    virtual ~SignalHandler();
    
    /// virtual signal handling function
    virtual void handleSignals(const std::string &signalAsString)=0;

    /// calls the original action which was associated to the corresponding signal
    /** This seems to be not practible, as the old actions are not defined by
        callable functions in the old action sigaction struct. */
    void oldAction(const std::string &signal);

    /// kills the current process
    /** internally this function calls kill(getpid(),1) which is mutch
        stronger than calling exit(0)
        (see the signal.h manpage)
    */
    static void killCurrentProcess();
    
    private:
    /// internal storage of associated signals
    std::vector<int > m_vecAssocitatedSignals;
  };

}
