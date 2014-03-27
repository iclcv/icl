/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ThreadedUpdatableTextView.cpp          **
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

#include <ICLQt/ThreadedUpdatableTextView.h>
#include <QApplication>
#include <ICLUtils/Macros.h>
#include <ICLQt/Application.h>

namespace icl{
  namespace qt{
    
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
      switch((int)event->type()){
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
  } // namespace qt
}

