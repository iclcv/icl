// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/Macros.h>
#include <QWidget>
#include <QApplication>
#include <QtCore/QEvent>
#include <QtCore/QThread>

namespace icl{
  namespace qt{

    /// Compability class
    /** This class provides a compability function for asyncronous updating
        of a QWidget.
        As QWidget::update() has shown to be not as threadsafe as expected,
        the new updateFromOtherThread function fixes this problem.

        updateFromOtherThread used QApplication::postEvent, to post a UserType
        QEvent to this object, which is caught in the overloaded event() function
    */
    class ThreadedUpdatableWidget : public QWidget{
      public:

      /// Base constructor
      ThreadedUpdatableWidget(QWidget *parent = 0): QWidget(parent){}

      /// call this function to update a widget's UI from an external thread
      /** new, if this is called from the GUI thread, setValue is called directly
          without using Qt's signal mechanism*/
      void updateFromOtherThread(){
        if(QThread::currentThread() == QCoreApplication::instance()->thread()){
          update();
        }else{
          QApplication::postEvent(this,new QEvent(QEvent::User),Qt::HighEventPriority);
        }

      }

      /// automatically called by Qt's event processing mechanism
      virtual bool event ( QEvent * event ){
        ICLASSERT_RETURN_VAL(event,false);
        if(event->type() == QEvent::User){
          update();
          return true;
        }else{
          return QWidget::event(event);
        }
      }
    };

  } // namespace qt
}
