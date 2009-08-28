#ifndef ICL_SLIDER_HANDLE_H
#define ICL_SLIDER_HANDLE_H

#include <iclGUIHandle.h>


namespace icl{
  
  /** \cond */
  class ThreadedUpdatableSlider;
  /** \endcond */
 
  /// Handle class for slider componets \ingroup HANDLES
  class SliderHandle : public GUIHandle<ThreadedUpdatableSlider>{
    public:
    /// Creates and empty slider handle
    SliderHandle(){}

    /// create a slider handle
    SliderHandle(ThreadedUpdatableSlider *sl, GUIWidget *w):GUIHandle<ThreadedUpdatableSlider>(sl,w){}
    
    /// set the min value
    void setMin(int min);

    /// set the max value
    void setMax(int max);

    /// set the range of the slider
    void setRange(int min, int max);
    
    /// set the current value of the slider
    void setValue(int val);

    /// sets all parameters of a slider
    void setAll(int min ,int max, int val){ setRange(min,max); setValue(val); }

    /// returns the current min. of the slider
    int getMin() const;

    /// returns the current max. of the slider
    int getMax() const;

    /// returns the current value of the slider
    int getValue() const;
    
    /// assigns a new value to the slider (equal to setValue)
    void operator=(int val) { setValue(val); }
  };
  
}

#endif
