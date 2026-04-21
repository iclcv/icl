// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/IntHandle.h>

#include <icl/utils/AssignRegistry.h>
#include <icl/utils/StringUtils.h>

#include <string>
#include <charconv>
#include <QLineEdit>


namespace icl::qt {

  namespace {
    /// Parse `text` as int — same semantics as the old read-time path:
    /// leading digits are parsed; empty / non-numeric input yields 0.
    int parseInt(const QString &text) {
      auto bytes = text.toLatin1();
      int val = 0;
      std::from_chars(bytes.data(), bytes.data() + bytes.size(), val);
      return val;
    }
  }

  IntHandle::IntHandle(QLineEdit *le, GUIWidget *w)
    : GUIHandle<QLineEdit>(le, w),
      m_cache(std::make_shared<std::atomic<int>>(le ? parseInt(le->text()) : 0)) {
    if (!le) return;
    auto cache = m_cache;
    QObject::connect(le, &QLineEdit::textChanged, le,
                     [cache](const QString &t){
                       cache->store(parseInt(t), std::memory_order_relaxed);
                     });
  }

  void IntHandle::operator=(int i){
    (**this)->setText(QString::number(i));
  }
  void IntHandle::operator=(const std::string &s){
    *this = icl::utils::parse<int>(s);
  }
  int IntHandle::getValue() const{
    return m_cache ? m_cache->load(std::memory_order_relaxed) : 0;
  }

  template<typename T>
    requires std::is_same_v<T, std::string>
  T IntHandle::as() const {
    return icl::utils::str(getValue());
  }
  template std::string IntHandle::as<std::string>() const;

  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::IntHandle;
  __attribute__((constructor))
  static void icl_register_int_handle_assignments() {
    AssignRegistry::enroll_symmetric<IntHandle, int, float, double, std::string>();
  }
}