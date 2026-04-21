// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/FloatHandle.h>

#include <icl/utils/AssignRegistry.h>
#include <icl/utils/StringUtils.h>

#include <string>
#include <cstdlib>
#include <QLineEdit>


namespace icl::qt {
  void FloatHandle::operator=(float f){
    (**this)->setText(QString::number(f));
  }
  void FloatHandle::operator=(const std::string &s){
    *this = icl::utils::parse<float>(s);
  }
  float FloatHandle::getValue() const{
    auto bytes = (**this)->text().toLatin1();
    return std::strtof(bytes.data(), nullptr);
  }

  template<typename T>
    requires std::is_same_v<T, std::string>
  T FloatHandle::as() const {
    return icl::utils::str(getValue());
  }
  template std::string FloatHandle::as<std::string>() const;

  }  // namespace icl::qt

namespace {
  __attribute__((constructor))
  static void icl_register_float_handle_assignments() {
    auto &r = icl::utils::AssignRegistry::instance();
    using icl::qt::FloatHandle;

    r.enroll<FloatHandle, int>();
    r.enroll<FloatHandle, float>();
    r.enroll<FloatHandle, double>();
    r.enroll<FloatHandle, std::string>();

    r.enroll<int,         FloatHandle>();
    r.enroll<float,       FloatHandle>();
    r.enroll<double,      FloatHandle>();
    r.enroll<std::string, FloatHandle>();
  }
}