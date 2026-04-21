// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/SliderHandle.h>
#include <icl/qt/ThreadedUpdatableSlider.h>

namespace icl::qt {
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


  } // namespace icl::qt