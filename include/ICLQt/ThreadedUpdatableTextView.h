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
