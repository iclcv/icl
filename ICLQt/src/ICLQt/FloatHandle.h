// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>

/** \cond */
class QLineEdit;
/** \endcond */

namespace icl{
  namespace qt{

    /// Class for handling "float" textfield components \ingroup HANDLES
    class ICLQt_API FloatHandle : public GUIHandle<QLineEdit> {
      public:

      /// Create an empty handle
      FloatHandle(){}

      /// Create a new FloatHandel
      FloatHandle(QLineEdit *le, GUIWidget *w):GUIHandle<QLineEdit>(le,w){}

      /// make the associated text field show a float
      void operator=(float f);

      /// returns the current text as float
      float getValue() const;
    };
  } // namespace qt
}
