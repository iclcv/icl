#include <ICLQt/FloatHandle.h>
#include <string>
#include <QLineEdit>


namespace icl{
  
  void FloatHandle::operator=(float f){
    (**this)->setText(QString::number(f));
  }
  float FloatHandle::getValue() const{
    return (float)atof((**this)->text().toLatin1().data());
  }
}


