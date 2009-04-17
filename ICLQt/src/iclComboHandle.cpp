#include "iclComboHandle.h"
#include <QComboBox>

namespace icl{

  void ComboHandle::add(const std::string &item){
    cbx()->addItem(item.c_str());
  }
  void ComboHandle::remove(const std::string &item){
    int idx = getIndex(item);
    if(idx >= 0 && idx < getItemCount()){
      cbx()->removeItem(idx);
    }
  }
  void ComboHandle::remove(int idx){
    cbx()->removeItem(idx);
  }
  void ComboHandle::clear(){
    while(getItemCount()){
      remove(0);
    }
  }
  std::string ComboHandle::getItem(int idx) const{
    return cbx()->itemText(idx).toLatin1().data();
  }
  int ComboHandle::getIndex(const std::string &item) const{
    return cbx()->findText(item.c_str());
  }
  
  int ComboHandle::getSelectedIndex() const{
    return cbx()->currentIndex();
  }
  std::string ComboHandle::getSelectedItem() const{
    return cbx()->currentText().toLatin1().data();
  }
  
  int ComboHandle::getItemCount() const{
    return cbx()->count();
  }
  
  void ComboHandle::setSelectedIndex(int idx){
    cbx()->setCurrentIndex(idx);
  }
  void ComboHandle::setSelectedItem(const std::string &item){
    int idx = getIndex(item);
    if(idx >=0 && idx< getItemCount()){
      cbx()->setCurrentIndex(idx);
    }
  }

  
}
