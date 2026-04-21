// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <QtCore/QString>
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
    StringHandle(){}

    /// Create a new Int handle
    StringHandle(QLineEdit *le,std::string *str, GUIWidget *w):
	  GUIHandle<QLineEdit>(le,w),m_str(str){}

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
  };
  } // namespace icl::qt