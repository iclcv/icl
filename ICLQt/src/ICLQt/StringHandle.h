// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <QtCore/QString>
#include <string>

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

    /// returns the current text (only updated when enter is pressed)
    std::string getValue() const;

    /// returns the currently shown text of the textfield
    std::string getCurrentText() const;

    private:
    std::string *m_str;
  };
  } // namespace icl::qt