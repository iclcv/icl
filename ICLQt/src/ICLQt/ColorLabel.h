// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Color.h>
#include <ICLQt/ThreadedUpdatableWidget.h>
#include <QtCore/QMutex>

namespace icl::qt {
    /// Utility class to avoid Qt warning when accesing QLabels from differnt Threads
    /** QLabels can not be used from different Threads. So if a QLabel is created in
        in the main thread, it might not be set up to show another text/number from
        the working thread.
        As a workaround, the "label" component of the ICL GUI API uses not the
        original QLabel but this thread-save reimplementation called
        CompabilityLabel.
    */
    class ICLQt_API ColorLabel : public ThreadedUpdatableWidget{
      public:
      /// Create a new label with given text and given parent widget
      ColorLabel(core::Color4D &color, bool useAlpha, QWidget *parent=0);

      /// reimplemented drawin function (draw the current text centered)
      virtual void paintEvent(QPaintEvent *evt);

      /// sets new color rgb
      void setColor(const core::Color &color);

      /// sets new color rgba
      void setColor(const core::Color4D &color);

      /// returns current color
      core::Color getRGB() const;

      /// returns current rgba color
      core::Color4D getRGBA() const;

      /// returns current red value
      int getRed() const;

      /// returns current green value
      int getGreen() const;

      /// returns current blue value
      int getBlue() const;

      /// return current alpha value
      int getAlhpa() const;

      /// returns wheter internal color uses alpha value
      bool hasAlpha() const;

      private:

      /// shallowly wrapped color
      core::Color4D &m_color;

      /// indicator wheter alpha is used
      bool m_hasAlpha;

      /// Thread-safety mutex
      mutable QMutex m_oMutex;
    };
  } // namespace icl::qt