#include <iclStringHandle.h>
#include <string>
#include <QLineEdit>


namespace icl{
  
  void StringHandle::operator=(const std::string &text){
    (**this)->setText(text.c_str());
  }
  std::string StringHandle::getValue() const{
    return (**this)->text().toLatin1().data();
  }
}


