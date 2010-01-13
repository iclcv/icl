#include <ICLQt/StringSignalButton.h>

namespace icl{

  StringSignalButton::StringSignalButton(const QString &text,QWidget *parent):
    QPushButton(text,parent)
  {
    connect(this,SIGNAL(clicked(bool)), this, SLOT(receiveClick(bool)));
  }
  
  void StringSignalButton::receiveClick(bool enabled){
    (void)enabled;
    emit clicked(text());
  }
  
}
