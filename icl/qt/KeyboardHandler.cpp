// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/KeyboardHandler.h>

namespace icl::qt {

  KeyResult KeyboardHandler::process(const KeyEvent &e) {
    if (e.pressed) m_held.insert(e.key);
    else           m_held.erase(e.key);
    return KeyResult::Forward;   // observe-only by default
  }

} // namespace icl::qt
