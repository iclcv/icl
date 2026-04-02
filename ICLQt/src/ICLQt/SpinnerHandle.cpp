// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/SpinnerHandle.h>
#include <QSpinBox>

namespace icl::qt {
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


  } // namespace icl::qt