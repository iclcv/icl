#ifndef ICL_THREADED_UPDATABLE_SLIDER_H
#define ICL_THREADED_UPDATABLE_SLIDER_H

#include <QSlider>
#include <QApplication>
#include <ICLQt/SliderUpdateEvent.h>
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
  class ThreadedUpdatableSlider : public QSlider{
    public:
    
    /// Base constructor
    ThreadedUpdatableSlider(QWidget *parent = 0): QSlider(parent){}

    ThreadedUpdatableSlider(Qt::Orientation o, QWidget *parent = 0): QSlider(o, parent){}
    
    /// call this function to update a widget's UI from an external thread
    void setValueFromOtherThread(int value){
      QApplication::postEvent(this,new SliderUpdateEvent(value),Qt::HighEventPriority);
    }
    
    /// automatically called by Qt's event processing mechanism
    virtual bool event ( QEvent * event ){
      ICLASSERT_RETURN_VAL(event,false);
      if(event->type() == SliderUpdateEvent::EVENT_ID){
        setValue(reinterpret_cast<SliderUpdateEvent*>(event)->value);
        return true;
      }else{
        return QSlider::event(event);
      }
    } 
  };

}

#endif
