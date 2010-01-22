#ifndef ICL_THREADED_UPDATABLE_WIDGET_H
#define ICL_THREADED_UPDATABLE_WIDGET_H

#include <QWidget>
#include <QApplication>
#include <QEvent>
#include <ICLUtils/Macros.h>

namespace icl{
  
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
    void updateFromOtherThread(){
      QApplication::postEvent(this,new QEvent(QEvent::User),Qt::HighEventPriority);
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

}

#endif
