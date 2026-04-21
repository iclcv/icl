// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/ButtonHandle.h>

#include <icl/utils/AssignRegistry.h>

#include <QPushButton>
#include <algorithm>

namespace icl::qt {
  ButtonHandle::ButtonHandle(){}

  ButtonHandle::ButtonHandle(QPushButton *b, GUIWidget *w):
    GUIHandle<QPushButton>(b,w),
    m_triggered(std::make_shared<std::atomic<bool>>(false)){
  }

  bool ButtonHandle::wasTriggered(bool reset) {
    if (reset) {
      // Atomic read-modify-write: return the old value and clear
      // in one step so two concurrent readers don't both see "true".
      return m_triggered->exchange(false, std::memory_order_relaxed);
    }
    return m_triggered->load(std::memory_order_relaxed);
  }

  void ButtonHandle::reset() {
    m_triggered->store(false, std::memory_order_relaxed);
  }
  const std::string &ButtonHandle::getID() const {
    return m_sID;
  }


  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::ButtonHandle;
  __attribute__((constructor))
  static void icl_register_button_handle_assignments() {
    AssignRegistry::enroll_provider<ButtonHandle, bool, int, float, double>();
  }
}