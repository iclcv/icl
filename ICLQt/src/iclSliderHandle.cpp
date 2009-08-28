#include <iclSliderHandle.h>
#include <iclThreadedUpdatableSlider.h>

namespace icl{
  
  void SliderHandle::setMin(int min){
    (**this)->setMinimum(min);
  }
  void SliderHandle::setMax(int max){
    (**this)->setMaximum(max);
  }
  void SliderHandle::setRange(int min, int max){
    setMin(min);
    setMax(max);
  }
  void SliderHandle::setValue(int val){
    (**this)->setValueFromOtherThread(val);
  }
  int SliderHandle::getMin() const{
    return (**this)->minimum();
  }
  int SliderHandle::getMax() const{
    return (**this)->maximum();
  }
  int SliderHandle::getValue() const{
    return (**this)->value();
  }

  
}
