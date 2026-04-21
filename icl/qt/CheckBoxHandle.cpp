// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/CheckBoxHandle.h>

#include <icl/utils/AssignRegistry.h>
#include <icl/utils/StringUtils.h>

#include <QCheckBox>


namespace icl::qt {
  CheckBoxHandle::CheckBoxHandle() = default;

  CheckBoxHandle::CheckBoxHandle(QCheckBox *cb, GUIWidget *w):
    GUIHandle<QCheckBox>(cb,w),
    m_cache(std::make_shared<std::atomic<bool>>(cb ? cb->isChecked() : false)) {
    if (!cb) return;
    auto cache = m_cache;
    QObject::connect(cb, &QCheckBox::stateChanged, cb,
                     [cache](int state){
                       cache->store(state == Qt::Checked,
                                    std::memory_order_relaxed);
                     });
  }


  void CheckBoxHandle::check(bool execCallbacks){
    (***this).setCheckState(Qt::Checked);
    if(execCallbacks) cb();
  }

  void CheckBoxHandle::uncheck(bool execCallbacks){
    (***this).setCheckState(Qt::Unchecked);
    if(execCallbacks) cb();
  }

  bool CheckBoxHandle::isChecked() const{
    return m_cache ? m_cache->load(std::memory_order_relaxed) : false;
  }

  void CheckBoxHandle::operator=(const std::string &s){
    doCheck(icl::utils::parse<bool>(s));
  }

  template<typename T>
    requires std::is_same_v<T, std::string>
  T CheckBoxHandle::as() const {
    return icl::utils::str(isChecked());
  }
  template std::string CheckBoxHandle::as<std::string>() const;

  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::CheckBoxHandle;
  __attribute__((constructor))
  static void icl_register_check_box_handle_assignments() {
    AssignRegistry::enroll_symmetric<CheckBoxHandle, bool, int, float, double, std::string>();
  }
}