// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/ThreadedUpdatableWidget.h>
#include <QtCore/QString>
#include <QtCore/QMutex>

namespace icl{
  namespace qt{

    /// Utility class to avoid Qt warning when accesing QLabels from differnt Threads
    /** QLabels can not be used from different Threads. So if a QLabel is created in
        in the main thread, it might not be set up to show another text/number from
        the working thread.
        As a workaround, the "label" component of the ICL GUI API uses not the
        original QLabel but this thread-save reimplementation called
        CompabilityLabel.
    */
    class ICLQt_API CompabilityLabel : public ThreadedUpdatableWidget{
      public:
      /// Create a new label with given text and given parent widget
      CompabilityLabel(const QString &text, QWidget *parent=0);

      /// reimplemented drawin function (draw the current text centered)
      virtual void paintEvent(QPaintEvent *evt);

      /// make the label show an integer value
      void setNum(int i);

      /// make the label show a float value
      void setNum(float f);

      /// make the lable show a given string
      void setText(const QString &text);

      /// returns the current text (also thread save)
      QString text() const;

      /// returns current alignment
      Qt::Alignment getAlignment() const;

      /// sets new text alignment
      void setAlignment(Qt::Alignment a);

      private:
      /// current text (protected by the mutex)
      QString m_sText;

      /// text alignment
      Qt::Alignment m_alignment;

      /// Thread-safety mutex
      mutable QMutex m_oMutex;
    };
  } // namespace qt
}
