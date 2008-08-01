#include <iclStringHandle.h>
#include <string>
#include <QLineEdit>


namespace icl{
  
  void StringHandle::operator=(const std::string &text){
    (**this)->setText(text.c_str());
    getGUIWidget()->getGUI()->lockData();
    *m_str = text;
    getGUIWidget()->getGUI()->unlockData();

  }
  std::string StringHandle::getCurrentText() const{
    return (**this)->text().toLatin1().data();  
  }
  std::string StringHandle::getValue() const{
    std::string s;
    const_cast<StringHandle*>(this)->getGUIWidget()->getGUI()->lockData();
    s = *m_str;
    const_cast<StringHandle*>(this)->getGUIWidget()->getGUI()->unlockData();
    return s;
  }
}


