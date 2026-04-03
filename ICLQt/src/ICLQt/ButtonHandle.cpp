// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/ButtonHandle.h>
#include <QPushButton>
#include <algorithm>

namespace icl::qt {
  ButtonHandle::ButtonHandle(){}

  ButtonHandle::ButtonHandle(QPushButton *b, GUIWidget *w):
    GUIHandle<QPushButton>(b,w),m_triggered(new bool(false)){
  }

  bool ButtonHandle::wasTriggered(bool reset) {
    if(*m_triggered){
      if(reset) *m_triggered = false;
      return true;
    }
    return false;
  }

  void ButtonHandle::reset() {
    *m_triggered = false;
  }
  const std::string &ButtonHandle::getID() const {
    return m_sID;
  }


  } // namespace icl::qt