// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>

#include <string>
#include <type_traits>

/** \cond */
class QLineEdit;
/** \endcond */

namespace icl::qt {
  /// Class for handling "float" textfield components \ingroup HANDLES
  class ICLQt_API FloatHandle : public GUIHandle<QLineEdit> {
    public:

    /// Create an empty handle
    FloatHandle(){}

    /// Create a new FloatHandel
    FloatHandle(QLineEdit *le, GUIWidget *w):GUIHandle<QLineEdit>(le,w){}

    /// make the associated text field show a float
    void operator=(float f);

    /// parses `s` as a float and sets the textfield.  Throws on bad input.
    void operator=(const std::string &s);

    /// returns the current text as float
    float getValue() const;

    /// Explicit readback.  Arithmetic specialization static-casts
    /// the current value; string specialization formats it.
    template<typename T>
      requires std::is_arithmetic_v<T>
    T as() const { return static_cast<T>(getValue()); }

    template<typename T>
      requires std::is_same_v<T, std::string>
    T as() const;
  };
  } // namespace icl::qt