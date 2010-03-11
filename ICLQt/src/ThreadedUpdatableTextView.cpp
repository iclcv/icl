/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLQt/ThreadedUpdatableTextView.h>
#include <QApplication>
#include <ICLUtils/Macros.h>
namespace icl{
  
  void ThreadedUpdatableTextView::appendTextFromOtherThread(const std::string &text){
    QApplication::postEvent(this,new AddTextEvent(text),Qt::HighEventPriority);
  }
  void ThreadedUpdatableTextView::clearTextFromOtherThread(){
    QApplication::postEvent(this,new ClearTextEvent,Qt::HighEventPriority);
  }
  
  bool ThreadedUpdatableTextView::event ( QEvent * event ){
    ICLASSERT_RETURN_VAL(event,false);
    switch(event->type()){
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
}

