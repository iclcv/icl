/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/FSliderHandle.cpp                      **
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

#include <ICLQt/FSliderHandle.h>
#include <QSlider>

namespace icl{
  namespace qt{
    FSliderHandle::FSliderHandle():
      GUIHandle<ThreadedUpdatableSlider>(),lcd(0),m_fMin(0),m_fMax(0),m_fM(0),m_fB(0),m_iSliderRange(0){
    }
    FSliderHandle::FSliderHandle(ThreadedUpdatableSlider *sl,float *minV, float *maxV, float *M, float *B,int range, GUIWidget *w, QLCDNumber *lcd):
      GUIHandle<ThreadedUpdatableSlider>(sl,w),lcd(lcd),m_fMin(minV),m_fMax(maxV),m_fM(M),m_fB(B),m_iSliderRange(range){
      updateMB();
    }
    void FSliderHandle::setMin(float min){
      *m_fMin = min;
      updateMB();
    }
    void FSliderHandle::setMax(float max){
      *m_fMax = max;
      updateMB();
    }

    void FSliderHandle::setValue(float val){
      (**this)->setValue(f2i(val));
    }

    void FSliderHandle::updateMB(){
      *m_fM = (*m_fMax-*m_fMin)/m_iSliderRange;
      *m_fB = *m_fMin;
    }

    float FSliderHandle::getMin() const{
      return *m_fMin;
    }
    float FSliderHandle::getMax() const{
      return *m_fMax;
    }
    float FSliderHandle::getValue() const{
      return i2f((**this)->value());
    }
  } // namespace qt
}
