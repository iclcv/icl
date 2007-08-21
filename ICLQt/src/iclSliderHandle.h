#ifndef ICL_SLIDER_HANDLE_H
#define ICL_SLIDER_HANDLE_H

#include <iclGUIHandle.h>
/**\cond*/
class QSlider;
/**\endcond*/

namespace icl{
  
  /// Handle class for slider componets \ingroup HANDLES
  class SliderHandle : public GUIHandle<QSlider>{
    public:
    /// create a slider handle
    SliderHandle(QSlider *sl=0):GUIHandle<QSlider>(sl){}
    
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
