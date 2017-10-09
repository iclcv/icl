/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/SliderHandle.cpp                       **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLQt/SliderHandle.h>
#include <ICLQt/ThreadedUpdatableSlider.h>

namespace icl{
  namespace qt{

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


  } // namespace qt
}
