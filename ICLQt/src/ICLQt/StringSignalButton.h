// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <QPushButton>

namespace icl{
  namespace qt{
    /// internally used button that emits a signal with its text \ingroup UNCOMMON
    class ICLQt_API StringSignalButton : public QPushButton{
      Q_OBJECT
      public:
      /// Create a new StringSignalButton with given text and parent widget
      StringSignalButton(const QString &text,QWidget *parent);

      Q_SIGNALS:
      /// the clicked signal (with the buttons text)
      void clicked(const QString &text);

      private Q_SLOTS:
      /// internally used slot (connected to the parent buttons clicked() signal)
      void receiveClick(bool checked);

    };
  } // namespace qt
}
