// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <string>
#include <vector>
#include <QtWidgets/QCheckBox>

namespace icl{
  namespace qt{

    /// Special Utiltiy class for handling Button clicks in the ICL GUI API \ingroup HANDLES
    class ICLQt_API CheckBoxHandle : public GUIHandle<QCheckBox>{
      public:

      /// creates a n empty button handle
      CheckBoxHandle();

      /// create a new event with a given button id
      CheckBoxHandle(QCheckBox *cb, GUIWidget *w, bool *stateRef);

      /// checks this checkbox
      void check(bool execCallbacks=true);

      // unchecks this checkbox
      void uncheck(bool execCallbacks=true);

      /// defines the check-state
      inline void doCheck(bool on, bool execCallbacks=true){
        if(on) check(execCallbacks);
        else uncheck(execCallbacks);
      }

      // returns whether this the checkbox is currently checked
      bool isChecked() const;

      private:

      /// internal state reference variable
      bool *m_stateRef;

    };
  } // namespace qt
}
