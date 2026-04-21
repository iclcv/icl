// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/FSliderHandle.h>

#include <icl/utils/AssignRegistry.h>
#include <icl/utils/StringUtils.h>

#include <QSlider>

namespace icl::qt {
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
    // Lock-free read through the atomic cache in ThreadedUpdatableSlider.
    // Mapping from integer slider coordinates to float value is pure
    // arithmetic on plain-old-data members; no Qt-widget access.
    return i2f((**this)->atomicValue());
  }

  void FSliderHandle::operator=(const std::string &s){
    setValue(icl::utils::parse<float>(s));
  }

  template<typename T>
    requires std::is_same_v<T, std::string>
  T FSliderHandle::as() const {
    return icl::utils::str(getValue());
  }
  template std::string FSliderHandle::as<std::string>() const;

  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::FSliderHandle;
  __attribute__((constructor))
  static void icl_register_fslider_handle_assignments() {
    AssignRegistry::enroll_symmetric<FSliderHandle, int, float, double, std::string>();
  }
}