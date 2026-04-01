// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <QTextEdit>
#include <QtCore/QEvent>
#include <string>

namespace icl{
  namespace qt{

    class ICLQt_API ThreadedUpdatableTextView : public QTextEdit{
      static const QEvent::Type ADD_TEXT=static_cast<QEvent::Type>(QEvent::User+1);
      static const QEvent::Type CLEAR_TEXT=static_cast<QEvent::Type>(QEvent::User+2);

      struct AddTextEvent : public QEvent{
        std::string text;
        AddTextEvent(const std::string &text):
        QEvent(ADD_TEXT),text(text){}
      };
      struct ClearTextEvent : public QEvent{
        ClearTextEvent():
        QEvent(CLEAR_TEXT){}
      };
      public:
      ThreadedUpdatableTextView(QWidget *parent=0):
      QTextEdit(parent){}

      void appendTextFromOtherThread(const std::string &text);
      void clearTextFromOtherThread();

      /// automatically called by Qt's event processing mechanism
      virtual bool event ( QEvent * event );
    };
  } // namespace qt
}
