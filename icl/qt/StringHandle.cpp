// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/StringHandle.h>

#include <icl/utils/dispatch/AssignRegistry.h>
#include <icl/utils/StringUtils.h>

#include <string>
#include <QLineEdit>


namespace icl::qt {

  StringHandle::StringHandle(QLineEdit *le, GUIWidget *w)
    : GUIHandle<QLineEdit>(le, w),
      m_cache(std::make_shared<Cache>()) {
    if (le) m_cache->text = le->text().toLatin1().data();
    if (!le) return;
    auto cache = m_cache;
    QObject::connect(le, &QLineEdit::textChanged, le,
                     [cache](const QString &t){
                       auto s = t.toLatin1();
                       std::scoped_lock lock(cache->mutex);
                       cache->text.assign(s.data(), s.size());
                     });
  }

  void StringHandle::operator=(const std::string &text){
    (**this)->setText(text.c_str());
  }

  std::string StringHandle::getCurrentText() const{
    if (!m_cache) return {};
    std::scoped_lock lock(m_cache->mutex);
    return m_cache->text;
  }

  template<typename T>
    requires std::is_arithmetic_v<T>
  void StringHandle::operator=(T v) {
    *this = icl::utils::str(v);
  }
  template<typename T>
    requires std::is_arithmetic_v<T>
  T StringHandle::as() const {
    return icl::utils::parse<T>(getCurrentText());
  }

  // Emit the specializations we enroll into AssignRegistry below.
  template void StringHandle::operator=<int>(int);
  template void StringHandle::operator=<float>(float);
  template void StringHandle::operator=<double>(double);
  template int    StringHandle::as<int>() const;
  template float  StringHandle::as<float>() const;
  template double StringHandle::as<double>() const;

  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::StringHandle;
  __attribute__((constructor))
  static void icl_register_string_handle_assignments() {
    AssignRegistry::enroll_symmetric<StringHandle, int, float, double, std::string>();
    AssignRegistry::enroll_identity<StringHandle>();
  }
}