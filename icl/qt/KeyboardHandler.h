// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <set>

namespace icl::qt {

  /** \cond */
  class ICLWidget;
  /** \endcond */

  /// A keyboard event delivered to an installed KeyboardHandler.
  struct KeyEvent {
    int key = 0;          ///< Qt key code (e.g. Qt::Key_W)
    bool pressed = false; ///< true = key down, false = key up
    int modifiers = 0;    ///< ored Qt::KeyboardModifiers (Shift/Ctrl/Alt/...)
  };

  /// Result of KeyboardHandler::process — drives the handler-chain dispatch.
  /** Mirrors MouseResult: a widget dispatches a key event to its installed
      handlers in registration order and stops at the first that returns
      Processed; Forward lets the next handler see it. */
  enum class KeyResult {
    Processed,   //!< event consumed — stop the chain here
    Forward      //!< not handled / observe-only — pass to the next handler
  };

  /// Keyboard input handler installed on an ICLWidget (the keyboard analogue of
  /// MouseHandler — ICL had no app-level keyboard input before this).
  /** Install with `widget->install(&handler)`. The widget forwards key
      press/release events (auto-repeat filtered) to installed handlers. The base
      tracks a **held-key set**, so the common case — polling continuous input
      each frame — needs no subclass:
      \code
      KeyboardHandler keys;
      gui["draw"].install(&keys);
      ...                                  // per frame:
      if (keys.held(Qt::Key_W)) car->setEngineForce(+F);
      if (keys.held(Qt::Key_A)) car->setSteering(+a);
      \endcode
      For event-driven handling, override `process()` (call the base, or maintain
      the held set yourself, if you still want `held()` to work). */
  class ICLQt_API KeyboardHandler {
  public:
    virtual ~KeyboardHandler() = default;

    /// Handle one key event. The base updates the held set and returns Forward
    /// (observe-only). Reimplement for custom, chain-aware handling.
    virtual KeyResult process(const KeyEvent &e);

    /// Is \a key (a Qt::Key_* code) currently held down?
    bool held(int key) const { return m_held.find(key) != m_held.end(); }

    /// Clear the held set (e.g. on focus loss, to avoid stuck keys).
    void clearHeld() { m_held.clear(); }

  protected:
    std::set<int> m_held;
  };

} // namespace icl::qt
