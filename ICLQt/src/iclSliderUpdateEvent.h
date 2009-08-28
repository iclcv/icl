#ifndef ICL_SLIDER_UPDATE_EVENT_H
#define ICL_SLIDER_UPDATE_EVENT_H

#include <QEvent>

namespace icl{ 
  
  /// Utility class for threaded updatable sliders 
  struct SliderUpdateEvent : public QEvent{
    int value;
    static const QEvent::Type EVENT_ID=(QEvent::Type)(QEvent::User+1);
    SliderUpdateEvent(int value):
      QEvent(EVENT_ID),value(value){
    }
  };
  
}

#endif
