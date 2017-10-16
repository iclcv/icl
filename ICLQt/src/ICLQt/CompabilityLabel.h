/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/CompabilityLabel.h                     **
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

