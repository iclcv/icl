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

  namespace {
    /// Parse `text` as float — strtof semantics (returns 0.0f on
    /// empty / non-numeric input).
    float parseFloat(const QString &text) {
      auto bytes = text.toLatin1();
      return std::strtof(bytes.data(), nullptr);
    }
  }

  FloatHandle::FloatHandle(QLineEdit *le, GUIWidget *w)
    : GUIHandle<QLineEdit>(le, w),
      m_cache(std::make_shared<std::atomic<float>>(le ? parseFloat(le->text()) : 0.0f)) {
    if (!le) return;
    auto cache = m_cache;
    QObject::connect(le, &QLineEdit::textChanged, le,
                     [cache](const QString &t){
                       cache->store(parseFloat(t), std::memory_order_relaxed);
                     });
  }

  void FloatHandle::operator=(float f){
    (**this)->setText(QString::number(f));
  }
  void FloatHandle::operator=(const std::string &s){
    *this = icl::utils::parse<float>(s);
  }
  float FloatHandle::getValue() const{
    return m_cache ? m_cache->load(std::memory_order_relaxed) : 0.0f;
  }

  template<typename T>
    requires std::is_same_v<T, std::string>
  T FloatHandle::as() const {
    return icl::utils::str(getValue());
  }
  template std::string FloatHandle::as<std::string>() const;

  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::FloatHandle;
  __attribute__((constructor))
  static void icl_register_float_handle_assignments() {
    AssignRegistry::enroll_symmetric<FloatHandle, int, float, double, std::string>();
    AssignRegistry::enroll_identity<FloatHandle>();
  }
}