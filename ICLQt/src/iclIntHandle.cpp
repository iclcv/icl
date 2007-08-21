#include <iclIntHandle.h>
#include <string>
#include <QLineEdit>


namespace icl{
  
  void IntHandle::operator=(int i){
    (**this)->setText(QString::number(i));
  }
  int IntHandle::getValue() const{
    return atoi((**this)->text().toLatin1().data());
  }
}


