// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <QtCore/QString>

#include <memory>
#include <mutex>
#include <string>
#include <type_traits>

/** \cond */
class QLineEdit;
/** \endcond */

namespace icl::qt {
  /// Class for handling "string" textfield components \ingroup HANDLES
  class ICLQt_API StringHandle : public GUIHandle<QLineEdit>{
    public:
    /// Creates an empty string handle
    StringHandle() = default;

    /// Create a new StringHandle wrapping `le`.  Seeds the
    /// current-text cache from the line edit and installs a
    /// `textChanged(QString)` lambda that updates it on the GUI
    /// thread.  The "committed" value (`getValue()`) still routes
    /// through the `*m_str` DataStore bool until `.out()` retirement.
    StringHandle(QLineEdit *le, std::string *str, GUIWidget *w);

    /// makes the associated textfield show the given text
    void operator=(const std::string &text);

    /// makes the associated textfield show the given text
    void operator=(const QString &text){ (*this)=text.toLatin1().data(); }

    /// makes the associated textfield show the given text
    void operator=(const char *text) {(*this) = std::string(text); }

    /// makes the associated textfield show the given arithmetic value,
    /// formatted via `utils::str()`.
    template<typename T>
      requires std::is_arithmetic_v<T>
    void operator=(T v);

    /// returns the current text (only updated when enter is pressed)
    std::string getValue() const;

    /// returns the currently shown text of the textfield
    std::string getCurrentText() const;

    /// Explicit readback.  Arithmetic specialization parses the current
    /// text as T; string specialization returns the text.
    template<typename T>
      requires std::is_arithmetic_v<T>
    T as() const;

    template<typename T>
      requires std::is_same_v<T, std::string>
    T as() const { return getValue(); }

    private:
    std::string *m_str;

    /// Mutex-guarded snapshot of the live QLineEdit text.  Written
    /// from the GUI thread on every `textChanged(QString)` — read
    /// from any thread via `getCurrentText()`.  std::string is not
    /// lock-free so a plain mutex is the right primitive.
    struct Cache {
      mutable std::mutex mutex;
      std::string text;
    };
    std::shared_ptr<Cache> m_cache;
  };
  } // namespace icl::qt