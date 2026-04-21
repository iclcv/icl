// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <QtWidgets/QLineEdit>

#include <atomic>
#include <memory>
#include <string>
#include <type_traits>

namespace icl::qt {
  /// Class for handling "int" textfield components \ingroup HANDLES
  class ICLQt_API IntHandle : public GUIHandle<QLineEdit>{
    public:

    /// Create an empty handle
    IntHandle() = default;

    /// Create a new IntHandle wrapping `le`.  Seeds the lock-free
    /// cache by parsing the QLineEdit's current text and installs a
    /// Qt connection that re-parses on every `textChanged(QString)` —
    /// runs on the GUI thread.  See `SpinnerHandle` for the lifetime
    /// argument behind the shared_ptr + widget-context design.
    IntHandle(QLineEdit *le, GUIWidget *w);

    /// makes the associated textfield show the given value
    void operator=(int i);

    /// parses `s` as an int and sets the textfield.  Throws on malformed input.
    void operator=(const std::string &s);

    /// returns the current text as int
    int getValue() const;

    /// Explicit readback.  Arithmetic specialization static-casts
    /// the current value; string specialization formats it.
    template<typename T>
      requires std::is_arithmetic_v<T>
    T as() const { return static_cast<T>(getValue()); }

    template<typename T>
      requires std::is_same_v<T, std::string>
    T as() const;

    private:
    /// Lock-free snapshot of the parsed text.  Updated on every
    /// `QLineEdit::textChanged(QString)` — same cadence as the
    /// pre-cache read-time parse.  Null on a default-constructed
    /// handle; getValue() returns 0 in that case.
    std::shared_ptr<std::atomic<int>> m_cache;
  };
  } // namespace icl::qt