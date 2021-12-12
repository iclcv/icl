/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ColorLabel.h                           **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Color.h>
#include <ICLQt/ThreadedUpdatableWidget.h>
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
  } // namespace qt
}
