#ifndef ICL_STRING_SIGNAL_BUTTON_H
#define ICL_STRING_SIGNAL_BUTTON_H

#include <QPushButton>

namespace icl{
  class StringSignalButton : public QPushButton{
    Q_OBJECT
    public:
    StringSignalButton(const QString &text,QWidget *parent);

    signals:
    
    void clicked(const QString &text);
    
    private slots:
    
    void receiveClick(bool checked);
    
  };
}


#endif
