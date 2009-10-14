#include <iclThreadedUpdatableTextView.h>
#include <QApplication>
#include <iclMacros.h>
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

