// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <QtWidgets/QLineEdit>

namespace icl{
  namespace qt{

    /// Class for handling "int" textfield components \ingroup HANDLES
    class ICLQt_API IntHandle : public GUIHandle<QLineEdit>{
      public:

      /// Create an empty handle
      IntHandle(){}

      /// Create a new Int handle
      IntHandle(QLineEdit *le, GUIWidget *w):GUIHandle<QLineEdit>(le,w){}

      /// makes the associated textfield show the given value
      void operator=(int i);

      /// returns the current text as int
      int getValue() const;
    };
  } // namespace qt
}
