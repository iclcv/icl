// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/SliderHandle.h>
#include <icl/qt/ThreadedUpdatableSlider.h>

#include <icl/utils/AssignRegistry.h>
#include <icl/utils/StringUtils.h>

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

  void SliderHandle::operator=(const std::string &s){
    setValue(icl::utils::parse<int>(s));
  }

  template<typename T>
    requires std::is_same_v<T, std::string>
  T SliderHandle::as() const {
    return icl::utils::str(getValue());
  }

  // Explicit instantiation so the string specialization is emitted
  // in this TU and visible to other translation units.
  template std::string SliderHandle::as<std::string>() const;

  }  // namespace icl::qt

// ============================================================
//  Runtime enrollment of SliderHandle assignment pairs.
//  Each entry corresponds to a legacy DataStore FROM_TO rule;
//  the trait now delegates to the C++ operators added above.
// ============================================================
namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::SliderHandle;
  __attribute__((constructor))
  static void icl_register_slider_handle_assignments() {
    AssignRegistry::enroll_symmetric<SliderHandle, int, float, double, std::string>();
  }
}