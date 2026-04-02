// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/MouseHandler.h>

namespace icl::qt {
  void MouseHandler::handleEvent(const MouseEvent &event){
    process(event);
  }
  void MouseHandler::process(const MouseEvent &event){
    if(m_handler)m_handler(event);
  }
  } // namespace icl::qt