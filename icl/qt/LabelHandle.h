// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <QtCore/QString>
#include <icl/qt/GUIHandle.h>
#include <icl/qt/CompabilityLabel.h>
#include <QWidget>
#include <string>


namespace icl::qt {
  /// Class for GUI-Label handling \ingroup HANDLES
  /** The gui label is created inside the GUI class, it can be used
      to make GUI-"label" components show anther text
      @see GUI */
  class ICLQt_API LabelHandle : public GUIHandle<CompabilityLabel>{
    public:

    /// Create an empty handle
    LabelHandle(){}

    /// Create a new LabelHandle
    LabelHandle(CompabilityLabel *l, GUIWidget *w):GUIHandle<CompabilityLabel>(l,w){}

    ///  assign a std::string (makes the underlying label show that string)
    void operator=(const std::string &text);

    ///  assign a QString (makes the underlying label show that string)
    void operator=(const QString &text);

    ///  assign a const char* (makes the underlying label show that string)
    void operator=(const char *text);

    ///  assign an int (makes the underlying label show that integer)
    void operator=(int num);

    ///  assign a double (makes the underlying label show that double)
    void operator=(double num);

    /// appends text to the current text
    void operator+=(const std::string &text);

    private:
    /// utitlity function
    CompabilityLabel *lab() { return **this; }

    /// utitlity function
    const CompabilityLabel *lab() const { return **this; }
  };

  } // namespace icl::qt