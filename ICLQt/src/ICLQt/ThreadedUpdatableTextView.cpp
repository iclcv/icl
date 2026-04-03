// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/ThreadedUpdatableTextView.h>
#include <QApplication>
#include <ICLUtils/Macros.h>
#include <ICLQt/Application.h>

namespace icl::qt {
  void ThreadedUpdatableTextView::appendTextFromOtherThread(const std::string &text){
    if(ICLApp::isGUIThreadActive()){
      append(text.c_str());
    }else{
      QApplication::postEvent(this,new AddTextEvent(text),Qt::HighEventPriority);
    }
  }
  void ThreadedUpdatableTextView::clearTextFromOtherThread(){
    if(ICLApp::isGUIThreadActive()){
      clear();
    }else{
      QApplication::postEvent(this,new ClearTextEvent,Qt::HighEventPriority);
    }
  }

  bool ThreadedUpdatableTextView::event ( QEvent * event ){
    ICLASSERT_RETURN_VAL(event,false);
    switch(static_cast<int>(event->type())){
      case ADD_TEXT:
        append(reinterpret_cast<AddTextEvent*>(event)->text.c_str());
        return true;
      case CLEAR_TEXT:
        clear();
        return true;
      default:
        return QTextEdit::event(event);
    }
  }
  } // namespace icl::qt