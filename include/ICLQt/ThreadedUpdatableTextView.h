/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/ThreadedUpdatableTextView.h              **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_THREADED_UPDATABLE_TEXT_VIEW_H
#define ICL_THREADED_UPDATABLE_TEXT_VIEW_H

#include <QTextEdit>
#include <QEvent>
#include <string>

namespace icl{

  class ThreadedUpdatableTextView : public QTextEdit{
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
}

#endif
