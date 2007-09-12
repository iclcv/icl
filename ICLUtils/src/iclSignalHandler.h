//#include <signal.h>
#include <string>
#include <vector>
#include <iclMutex.h>
#include <iclMacros.h>

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
