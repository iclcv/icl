#include "iclSpinnerHandle.h"
#include <QSpinBox>

namespace icl{
  
  void SpinnerHandle::setMin(int min){
    sb()->setMinimum(min);
  }
  
  void SpinnerHandle::setMax(int max){
    sb()->setMaximum(max);
  }
  
  void SpinnerHandle::setValue(int val){
    sb()->setValue(val);
  }
  
  int SpinnerHandle::getMin() const{
    return sb()->minimum();
  }
  
  int SpinnerHandle::getMax() const{
    return sb()->maximum();
  }
  
  int SpinnerHandle::getValue() const{
    return sb()->value();
  }
  
  
}
