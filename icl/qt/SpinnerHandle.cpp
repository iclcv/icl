// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/SpinnerHandle.h>

#include <icl/utils/AssignRegistry.h>
#include <icl/utils/StringUtils.h>

#include <QSpinBox>

namespace icl::qt {

  SpinnerHandle::SpinnerHandle(QSpinBox *sb, GUIWidget *w)
    : GUIHandle<QSpinBox>(sb, w),
      m_cache(std::make_shared<std::atomic<int>>(sb ? sb->value() : 0)) {
    if (!sb) return;
    // Capture the cache by value (shared_ptr copy) so the atomic
    // outlives the handle: the cache survives as long as either any
    // handle copy OR the Qt connection is alive.  The context object
    // is the widget, so the connection is dropped automatically when
    // `sb` is destroyed — matching the lifetime of everything else
    // routed through the handle.
    auto cache = m_cache;
    QObject::connect(sb, QOverload<int>::of(&QSpinBox::valueChanged),
                     sb, [cache](int v){
                       cache->store(v, std::memory_order_relaxed);
                     });
  }

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
    // Lock-free read; `m_cache` is null only on default-constructed
    // handles, where the "value" is undefined anyway.
    return m_cache ? m_cache->load(std::memory_order_relaxed) : 0;
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
  using icl::utils::AssignRegistry;
  using icl::qt::SpinnerHandle;
  __attribute__((constructor))
  static void icl_register_spinner_handle_assignments() {
    AssignRegistry::enroll_symmetric<SpinnerHandle, int, float, double, std::string>();
    AssignRegistry::enroll_identity<SpinnerHandle>();
  }
}