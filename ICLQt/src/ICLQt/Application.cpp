// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Application.h>
#include <ICLQt/DefaultStyle.h>
#include <QtCore/QLocale>
#include <QtCore/QThread>
#include <QFile>
#include <QTextStream>
#include <QPalette>
#include <QStyleFactory>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/SignalHandler.h>
#include <mutex>
#include <cstdlib>

using namespace icl::utils;

namespace icl{
  namespace qt{
    struct ExecThread : public Thread{
      typedef void (*callback)(void);
      callback cb;
      ExecThread(callback cb):cb(cb){
        if(!cb) throw ICLException("ExecThread called with NULL function!");
      }
      void run() override {
        while(running()){
          if(!trylock()){
            cb();
            unlock();
          }
          usleep(1);
        }
      }
    };

    typedef ICLApplication::callback callback;

    ICLApplication *ICLApplication::s_app(0);
    std::vector<ExecThread*> ICLApplication::s_threads;
    std::vector<callback> ICLApplication::s_inits;
    std::vector<callback> ICLApplication::s_callbacks;
    std::vector<callback> ICLApplication::s_finalizes;
		std::vector<callback> ICLApplication::s_prepare_shutdowns;

#if 0
    static void qapplication_quit_wrapper(int){
      DEBUG_LOG("calling qapplication::quit!");
      QApplication::quit();
    }
#endif
    static void handle_icl_app_signal(const std::string &signal){
      static bool first = true;
      if(signal == "SIGINT" && first){
        first = false;
        std::cout << "Caught CTRL+C signal!\n"
                  << "Trying to force 'normal' shutdown ...\n"
                  << "(press CTRL+C again to force immediate exit)" << std::endl;
        QApplication::quit();
      }else if(first){
        first = false;
        std::cout << "Caught signal " + signal + "!\n"
                  << "Trying to force 'normal' shutdown ...\n"
                  << "(next signal will force an immediate exit)" << std::endl;
        QApplication::quit();
      }else{
        exit(EXIT_FAILURE);
      }

    }
#if 0
    ;
    ICLApplication::instance()->executeInGUIThread(qapplication_quit_wrapper, (int)0, false, true); //QApplication::quit();
    QApplication::quit();

    DEBUG_LOG("ignoring signal " << signal);
    return;

    if(signal == "SIGHUP"){
      //std::cout << "[SIGHUP]" << std::endl;
    }else if(signal == "SIGSEGV"){
      std::cout << "Segmentation violation detected!\n"
                << "Trying to force 'normal' shutdown ..." << std::endl;
      ICLApplication::instance()->executeInGUIThread(qapplication_quit_wrapper, (int)0, false, true); //QApplication::quit();
    }else{
      static bool first = true;
      if(first){
        first = false;
        std::cout << "Caught " << signal << " signal!\n"
                  << "Trying to force 'normal' shutdown ...\n"
                  << "(Send signal again to force immediate exit)" << std::endl;
        //QApplication::quit();
        ICLApplication::instance()->executeInGUIThread(qapplication_quit_wrapper, (int)0, false, true); //QApplication::quit();
        //QApplication::processEvents();
      }else{
        exit(EXIT_SUCCESS);
      }
    }
  }
#endif


  ICLApplication::ICLApplication(int n, char **ppc,
                                 const std::string &paInitString,
                                 callback init, callback run,
                                 callback run2, callback run3,
                                 callback run4, callback run5){
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
      app = static_cast<QApplication*>(existingApp);
    }else{
      static int static_n = 1;
      static char *static_ppc[] = { ppc[0], nullptr };
      app = new QApplication(static_n, static_ppc);
    }
#endif
    sharedWidget = new QOpenGLWidget();

    // Dark theme with Fusion style by default.
    // ICL_THEME=none disables, ICL_THEME=/path/to.qss loads custom file.
    const char *theme = std::getenv("ICL_THEME");
    if(!theme || std::string(theme) != "none"){
      app->setStyle("Fusion");

      // Set dark palette — this is what Qt actually uses for text/background colors
      QPalette p;
      p.setColor(QPalette::Window,          QColor(43, 43, 43));
      p.setColor(QPalette::WindowText,      QColor(224, 224, 224));
      p.setColor(QPalette::Base,            QColor(60, 60, 60));
      p.setColor(QPalette::AlternateBase,   QColor(51, 51, 51));
      p.setColor(QPalette::ToolTipBase,     QColor(60, 60, 60));
      p.setColor(QPalette::ToolTipText,     QColor(224, 224, 224));
      p.setColor(QPalette::Text,            QColor(224, 224, 224));
      p.setColor(QPalette::Button,          QColor(60, 60, 60));
      p.setColor(QPalette::ButtonText,      QColor(224, 224, 224));
      p.setColor(QPalette::BrightText,      QColor(255, 50, 50));
      p.setColor(QPalette::Link,            QColor(61, 174, 233));
      p.setColor(QPalette::Highlight,       QColor(61, 174, 233));
      p.setColor(QPalette::HighlightedText, Qt::white);
      p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128));
      p.setColor(QPalette::Disabled, QPalette::Text,       QColor(128, 128, 128));
      p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));
      app->setPalette(p);

      // Apply QSS polish on top (or load custom file)
      if(theme && std::string(theme) != "none"){
        QFile f(theme);
        if(f.open(QIODevice::ReadOnly | QIODevice::Text)){
          app->setStyleSheet(QTextStream(&f).readAll());
        }
      } else {
        app->setStyleSheet(qt::defaultStyleSheet());
      }
    }

    QLocale::setDefault(QLocale::C);

#ifdef ICL_SYSTEM_LINUX
    /* From the Qt documentation (originally Qt 4.7, still applicable):
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

    SignalHandler::install("ICL-Application",handle_icl_app_signal,
                           "SIGINT,SIGTERM,SIGSEGV,SIGHUP",100);

    connect(app, SIGNAL(lastWindowClosed()), this, SLOT(lastWindowClosed()));
  }

  ICLApplication::~ICLApplication(){
    s_app = 0;
    app->processEvents();
		for (unsigned int i = 0; i < s_prepare_shutdowns.size(); ++i){
			s_prepare_shutdowns[i]();
		}
		s_prepare_shutdowns.clear();
    for (unsigned int i = 0; i < s_threads.size(); ++i){
      s_threads[i]->stop(); // force right virtual stop implementation to be
      delete s_threads[i];
    }
    s_threads.clear();
    s_inits.clear();
    s_callbacks.clear();
    delete sharedWidget;
    delete app;

    for (unsigned int i = 0; i < s_finalizes.size(); ++i){
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

	void ICLApplication::addPrepareShutDown(callback cb) {
		ICLASSERT_RETURN(cb);
		s_prepare_shutdowns.push_back(cb);
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
      std::recursive_mutex *mutex;
      AsynchronousEventWrapper(ICLApplication::AsynchronousEvent *ae, std::recursive_mutex *mutex = 0):
        QEvent(static_cast<QEvent::Type>(QEvent::registerEventType())),ae(ae),mutex(mutex){}
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

  void ICLApplication::lastWindowClosed(){
    QApplication::quit(); // most likely not needed here!
  }

  void ICLApplication::executeInGUIThread(ICLApplication::AsynchronousEvent *event,
                                          bool blocking, bool forcePostEvent){
    if(isGUIThreadActive() && !forcePostEvent){
      event->execute();
    }else{
      if(blocking) {
        std::recursive_mutex mutex;
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
