/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/Application.h                            **
** Module : ICLQt                                                  **
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

#ifndef ICL_APPLICATION_H
#define ICL_APPLICATION_H

#include <string>
#include <vector>
#include <ICLUtils/Exception.h>
#include <QApplication>

namespace icl{
  /** \cond */
  class ExecThread;
  /** \endcond */
  
  
  /// QApplication extension for ICL based applications
  /** After the 100th time of writing 
      \code
      void init(){...}
      
      void run(){...}
      
      int main(int n, char **ppc){
        pa_init("...");
        ExecThread x(run);
        QApplication app(n,ppc);
        init();
        x.run();
        return app.exec();
      }
      \endcode
      I wrote a utility class that does all the stuff above for me:
      \code
      void init(){...}
      
      void run(){...}
      
      int main(int n, char **ppc){
        return ICLApplication(n,ppc,"...",init,run).exec();
      }
      
      \endcode
      Of course sometimes we need more than on initialization functions or even 
      more than one extra-thread:

      \code
      void init1(){...}
      void init2(){...}
      
      void run1(){...}
      void run2(){...}
      void run3(){...}
      
      int main(int n, char **ppc){
         ICLApplication app(n,ppc,"...");
         app.addInit(init1);
         app.addInit(init2);
         app.addThread(run1);
         app.addThread(run2);
         app.addThread(run3);
         return app.exec();
      }
      \endcode
      I guess there's nothing more to explain, isn't it?
  */
  class ICLApplication{
    
    public:
    QApplication *app;
    
    /// just type definition for convenience a void valued function with no args)
    typedef void (*callback)(void);
    
    /// Such an exception is returned if a 2nd instance of ICLApplication is created
    struct SecondSingeltonException : public ICLException{
      /// Basic constructor
      SecondSingeltonException(const std::string &reason):ICLException(reason){}
    };

    /// Constructor
    /** @param argc C++-main function arg-count
        @param argv C++-main function argument list (as obtained by int main(argc,argv))
        @param paInitString if not equal to "", pa_init is called with this string
        @param init initialization function pointer. Which is just called once when
                    exec() is called
        @param run threaded function which is called in a loop before the QApplication is
                   started using QApplication::exec();

        */
    ICLApplication(int argc, char **argv, const std::string &paInitString="", 
                   callback init=0, callback run=0,
                   callback run2=0, callback run3=0,
                   callback run4=0, callback run5=0)throw (SecondSingeltonException);

    /// Destructor
    ~ICLApplication();
    
    /// adds a new threaded function
    /** Threaded function are executed in a loop. Execution is started immediately before the
        QApplication is executed within ICLApplication::exec() */
    void addThread(callback cb);

    /// adds a new initialization function
    /** Initialization functions are called at the beginning of an ICLApplication::exec() call*/
    void addInit(callback cb);
    
    /// adds a new finalization function
    /** Finalization functions are called, when the singelton ICLApplication instance is deleted */
    void addFinalization(callback cb);

    /// executes this ICLApplication
    /** callbacks are executed in the following order:
        * init functions, entry by entry in same order as their addition to this instance
        * threads functions, are started in a extra thead, entry by entry in same order as their addition to this instance
        * QApplication is executed and it's return code is f passed using 'return QApplication::exec();'
    */
    int exec();
    
    private:
    /// singelton instance
    static ICLApplication *s_app;
    
    /// list of threads
    static std::vector<ExecThread*> s_threads;
    
    /// list of initialization functions
    static std::vector<callback> s_inits;
    
    
    /// list of callback functions
    static std::vector<callback> s_callbacks;

    /// list of finalization functions
    static std::vector<callback> s_finalizes;
  };

  /// this is just a shortcut typedef
  typedef ICLApplication ICLApp; 
}

#endif
