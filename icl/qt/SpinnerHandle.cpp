// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/SpinnerHandle.h>

#include <icl/utils/AssignRegistry.h>
#include <icl/utils/StringUtils.h>

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

  void SpinnerHandle::operator=(const std::string &s){
    setValue(icl::utils::parse<int>(s));
  }

  template<typename T>
    requires std::is_same_v<T, std::string>
  T SpinnerHandle::as() const {
    return icl::utils::str(getValue());
  }
  template std::string SpinnerHandle::as<std::string>() const;

  }  // namespace icl::qt

namespace {
  __attribute__((constructor))
  static void icl_register_spinner_handle_assignments() {
    auto &r = icl::utils::AssignRegistry::instance();
    using icl::qt::SpinnerHandle;

    r.enroll<SpinnerHandle, int>();
    r.enroll<SpinnerHandle, float>();
    r.enroll<SpinnerHandle, double>();
    r.enroll<SpinnerHandle, std::string>();

    r.enroll<int,         SpinnerHandle>();
    r.enroll<float,       SpinnerHandle>();
    r.enroll<double,      SpinnerHandle>();
    r.enroll<std::string, SpinnerHandle>();
  }
}