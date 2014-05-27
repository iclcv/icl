/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/Application.cpp                        **
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

#include <ICLQt/Application.h>
#include <QtCore/QLocale>
#include <QtCore/QThread>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/Mutex.h>

using namespace icl::utils;

namespace icl{
  namespace qt{
    struct ExecThread : public Uncopyable, public Thread{
      typedef void (*callback)(void);
      callback cb;
      ExecThread(callback cb):cb(cb){
        if(!cb) throw ICLException("ExecThread called with NULL function!");
      }
      virtual void run(){
        while(true){
          if(!trylock()){
            cb();
            unlock();
          }
          usleep(0);
        }
      }
    };
  
    typedef ICLApplication::callback callback;
   
    ICLApplication *ICLApplication::s_app(0);
    std::vector<ExecThread*> ICLApplication::s_threads;
    std::vector<callback> ICLApplication::s_inits;
    std::vector<callback> ICLApplication::s_callbacks;
    std::vector<callback> ICLApplication::s_finalizes;
  
    
    ICLApplication::ICLApplication(int n, char **ppc, 
                                   const std::string &paInitString,
                                   callback init, callback run,
                                   callback run2, callback run3,
                                   callback run4, callback run5)
      throw (SecondSingeltonException){
      if(s_app) throw SecondSingeltonException("only one instance is allowed!");
      if(paInitString != ""){
        pa_init(n,ppc,paInitString);
      }
      
      /* QApplication uses argv and argc internally, both are passed via reference to
         constructor and we must make sure those references stay valid for the entire
         lifetime of the QApplication object.
  
         Excerpt from the Qt documentation:
         "Warning: The data referred to by argc and argv must stay valid for the entire
         lifetime of the QApplication object. In addition, argc must be greater than
         zero and argv must contain at least one valid character string."
  
         We declare both as static variables before passing them to QApplication for
         this reason. 
     */
  #if 0
      app = new QApplication(n,ppc);
  #else
      
      // For some reason,  passing argv and argc to the QApplication leads
      // to a seg-fault because of reading a NULL string internally ??
      // Therefore we simply pass this static empty parameter list
      QCoreApplication *existingApp = QCoreApplication::instance();
      if(existingApp && dynamic_cast<QApplication*>(existingApp)){
        app = (QApplication*)existingApp;
      }else{
        static int static_n = 1;
        static char *static_ppc[] = { ppc[0], NULL };
        app = new QApplication(static_n, static_ppc);
      }
  #endif
      sharedWidget = new QGLWidget();
  
      QLocale::setDefault(QLocale::C);
  
  #ifdef ICL_SYSTEM_LINUX
      /* From the Qt 4.7 documentation:
          Locale Settings:
          On Unix/Linux Qt is configured to use the system
          locale settings by default. This can cause a conflict
          when using POSIX functions, for instance, when
          converting between data types such as floats and
          trings, since the notation may differ between locales.
          To get around this problem, call the POSIX function
          setlocale(LC_NUMERIC,"C") right after initializing
          QApplication or QCoreApplication to reset the locale
          that is used for number formatting to "C"-locale.    
      */
      setlocale(LC_NUMERIC,"C");
  #endif
  
      s_app = this;
      if(init) addInit(init);
      
      if(run) s_callbacks.push_back(run);
      if(run2) s_callbacks.push_back(run2);
      if(run3) s_callbacks.push_back(run3);
      if(run4) s_callbacks.push_back(run4);
      if(run5) s_callbacks.push_back(run5);
    }
    
    ICLApplication::~ICLApplication(){
      s_app = 0;
      app->processEvents();
      for(unsigned int i=0;i<s_threads.size();++i){
        delete s_threads[i];
      }
      s_threads.clear();
      s_inits.clear();
      s_callbacks.clear();
      delete sharedWidget;
      delete app;
   
      for(unsigned int i=0;i<s_finalizes.size();++i){
      	s_finalizes[i](); 
      }   
      s_finalizes.clear();
    }
    
    void ICLApplication::addThread(callback cb){
      ICLASSERT_RETURN(cb);
      s_callbacks.push_back(cb);
    }
    void ICLApplication::addInit(callback cb){
      ICLASSERT_RETURN(cb);
      s_inits.push_back(cb);
    }
  
    void ICLApplication::addFinalization(callback cb){
      ICLASSERT_RETURN(cb);
      s_finalizes.push_back(cb);
    }


    int ICLApplication::exec(){
      for(unsigned int i=0;i<s_inits.size();++i){
        s_inits[i]();
      }
      for(unsigned int i=0;i<s_callbacks.size();++i){
        s_threads.push_back(new ExecThread(s_callbacks[i]));
      }
      for(unsigned int i=0;i<s_threads.size();++i){
        s_threads[i]->start();
      }
      return app->exec();
    }
  
    namespace{
      struct AsynchronousEventWrapper : public QEvent{
        ICLApplication::AsynchronousEvent *ae;
        Mutex *mutex;
        AsynchronousEventWrapper(ICLApplication::AsynchronousEvent *ae, Mutex *mutex = 0):
          QEvent((QEvent::Type)QEvent::registerEventType()),ae(ae),mutex(mutex){} 
        ~AsynchronousEventWrapper(){
          delete ae;
        }
      };
    }

    bool ICLApplication::event(QEvent *eIn){
      AsynchronousEventWrapper *e = dynamic_cast<AsynchronousEventWrapper*>(eIn);
      if(!e) return false;
      e->ae->execute();
      if(e->mutex)e->mutex->unlock();
      return true;
    }
    
    
    void ICLApplication::executeInGUIThread(ICLApplication::AsynchronousEvent *event, bool blocking){
      if(isGUIThreadActive()){
        event->execute();
      }else{
        if(blocking) {
          Mutex mutex;
          mutex.lock();
          QApplication::postEvent(this,new AsynchronousEventWrapper(event, &mutex));
          mutex.lock();
          mutex.unlock();
        } else {
          QApplication::postEvent(this,new AsynchronousEventWrapper(event));
        }
      }
    }
  
    ICLApplication *ICLApplication::instance(){
      return s_app;
    }
    bool ICLApplication::isGUIThreadActive(){
      return QThread::currentThread() == QCoreApplication::instance()->thread();
    }
  } // namespace qt
}
