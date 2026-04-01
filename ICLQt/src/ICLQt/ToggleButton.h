// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <QPushButton>

namespace icl{
  namespace qt{

    class ICLQt_API ToggleButton : public QPushButton{
      Q_OBJECT;
      public:
      ToggleButton(const std::string &untoggledText,
                   const std::string &toggledText,
                   QWidget *parent = 0,
                   bool *stateRef=0);
      ~ToggleButton();

      private Q_SLOTS:
      void toggleStateChanged(bool toggled);

      private:
      std::string m_text[2];
      bool *m_stateRef;
    };
  } // namespace qt
}
