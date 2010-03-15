/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/SliderHandle.cpp                             **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLQt/SliderHandle.h>
#include <ICLQt/ThreadedUpdatableSlider.h>

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
