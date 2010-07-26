/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/FSliderHandle.cpp                            **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQt/FSliderHandle.h>
#include <QtGui/QSlider>

namespace icl{
  FSliderHandle::FSliderHandle():
    GUIHandle<QSlider>(),m_fMin(0),m_fMax(0),m_fM(0),m_fB(0),m_iSliderRange(0){
  }
  FSliderHandle::FSliderHandle(QSlider *sl,float *minV, float *maxV, float *M, float *B,int range, GUIWidget *w):
    GUIHandle<QSlider>(sl,w),m_fMin(minV),m_fMax(maxV),m_fM(M),m_fB(B),m_iSliderRange(range){
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
}
