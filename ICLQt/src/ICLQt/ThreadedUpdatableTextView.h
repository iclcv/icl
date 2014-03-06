/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ThreadedUpdatableTextView.h            **
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
#include <QtGui/QTextEdit>
#include <QtCore/QEvent>
#include <string>

namespace icl{
  namespace qt{
  
    class ICLQt_API ThreadedUpdatableTextView : public QTextEdit{
      static const QEvent::Type ADD_TEXT=(QEvent::Type)(QEvent::User+1);
      static const QEvent::Type CLEAR_TEXT=(QEvent::Type)(QEvent::User+2);
  
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

