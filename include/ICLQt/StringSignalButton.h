#ifndef ICL_STRING_SIGNAL_BUTTON_H
#define ICL_STRING_SIGNAL_BUTTON_H

#include <QPushButton>

namespace icl{
  /// internally used button that emits a signal with its text \ingroup UNCOMMON
  class StringSignalButton : public QPushButton{
    Q_OBJECT
    public:
    /// Create a new StringSignalButton with given text and parent widget
    StringSignalButton(const QString &text,QWidget *parent);

    signals:
    /// the clicked signal (with the buttons text)
    void clicked(const QString &text);
    
    private slots:
    /// internally used slot (connected to the parent buttons clicked() signal)
    void receiveClick(bool checked);
    
  };
}


#endif
