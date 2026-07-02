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
#include <type_traits>


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

    ///  assign any other integral value (long, unsigned, size_t, ...) as a number
    /** `int` has its dedicated overload above; this catches the wider and
        unsigned integer types — notably the `size_t` returned by
        `container.size()` — which would otherwise be an ambiguous
        int-vs-double conversion (and hence not assignable at all). */
    template<typename T>
      requires (std::is_integral_v<T> && !std::is_same_v<T, int>
                                      && !std::is_same_v<T, bool>)
    void operator=(T num){
      if constexpr (std::is_signed_v<T>)
        (*this) = QString::number(static_cast<qlonglong>(num));
      else
        (*this) = QString::number(static_cast<qulonglong>(num));
    }

    /// appends text to the current text
    void operator+=(const std::string &text);

    /// Explicit readback — returns the current label text.  Only a
    /// string specialization: labels don't "own" a numeric value that
    /// could be read back (they show arbitrary formatted text).
    template<typename T>
      requires std::is_same_v<T, std::string>
    T as() const;

    private:
    /// utitlity function
    CompabilityLabel *lab() { return **this; }

    /// utitlity function
    const CompabilityLabel *lab() const { return **this; }
  };

  } // namespace icl::qt