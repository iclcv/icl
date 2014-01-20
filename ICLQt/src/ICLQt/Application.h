/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/Application.h                          **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
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
#include <ICLUtils/Exception.h>
#include <ICLUtils/Function.h>
#include <string>
#include <vector>

#ifdef ICL_SYSTEM_WINDOWS
#include "windows.h"
#include <GL/glew.h>
#endif

#include <QtGui/QApplication>

namespace icl{

  namespace qt{

    /** \cond */
    struct ExecThread;
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
    class ICLQt_API ICLApplication : public QObject{
      
      public:
      QApplication *app;
      
      /// just type definition for convenience a void valued function with no args)
      typedef void (*callback)(void);
      
      /// Such an exception is returned if a 2nd instance of ICLApplication is created
      struct SecondSingeltonException : public utils::ICLException{
        /// Basic constructor
        SecondSingeltonException(const std::string &reason):utils::ICLException(reason){}
      };
  
      /// Constructor
      /** @param argc C++-main function arg-count
          @param argv C++-main function argument list (as obtained by int main(argc,argv))
          @param paInitString if not equal to "", pa_init is called with this string
          @param init initialization function pointer. Which is just called once when
                      exec() is called
          @param run first threaded function which is called in a loop before the QApplication is started using QApplication::exec();
          @param run2 second threaded function which is called in a loop before the QApplication is started using QApplication::exec();
          @param run3 third threaded function which is called in a loop before the QApplication is started using QApplication::exec();
          @param run4 fourth threaded function which is called in a loop before the QApplication is started using QApplication::exec();
          @param run5 fifth threaded function which is called in a loop before the QApplication is started using QApplication::exec();
  
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
      
      /// interface for events that must be executed in the GUI Thread
      struct AsynchronousEvent{
        /// pure virtual execution method
        virtual void execute() = 0;
        
        /// virtual destructor
        virtual ~AsynchronousEvent(){}
      };
      
      /// internally posts the event to the GUI Thread
      /** The event's ownwership is passed. It is deleted internally after 
          it is processed (please note, that the deletion will also take place
          within the GUI thread */
      void executeInGUIThread(AsynchronousEvent *event, bool blocking = false);
      
      /// utility class for executing functions with given arguments in the GUI thread
      /** This function is a simple convenience wrapper for executeInGUIThread(AsynchronousEvent*,bool) */
      template<class T>
      void executeInGUIThread(utils::Function<void,T> f, T data, bool blocking = false){
        struct TmpAsynchronousEvent : public AsynchronousEvent{
          T data;
          utils::Function<void,T> f;
          TmpAsynchronousEvent(T data, utils::Function<void,T> f):data(data),f(f){}
          void execute(){
            f(data);
          }
        };
        executeInGUIThread(new TmpAsynchronousEvent(data,f),blocking);
      }
      
      /// utility class for executing functions with given arguments in the GUI thread
      /** This function is a simple convenience wrapper for executeInGUIThread(AsynchronousEvent*,bool) */
      template<class T,class U>
      void executeInGUIThread(utils::Function<void,T,U> f, T t, U u, bool blocking = false){
        struct TmpAsynchronousEvent : public AsynchronousEvent{
          T t;
          U u;
          utils::Function<void,T,U> f;
          TmpAsynchronousEvent(T t, U u, utils::Function<void,T,U> f):t(t),u(u),f(f){}
          void execute(){
            f(t,u);
          }
        };
        executeInGUIThread(new TmpAsynchronousEvent(t,u,f),blocking);
      }
      

      /// overloaded event function
      virtual bool event(QEvent *eIn);
      
      /// returns the singelton ICLApplication instance (or NULL if there is none)
      static ICLApplication *instance();
      
      /// returns whether we are currently in the GUI thread
      static bool isGUIThreadActive();
      
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
  } // namespace qt
}

