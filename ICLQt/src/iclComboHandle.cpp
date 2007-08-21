#include "iclComboHandle.h"
#include <QComboBox>

namespace icl{

  void ComboHandle::add(const std::string &item){
    cb()->addItem(item.c_str());
  }
  void ComboHandle::remove(const std::string &item){
    int idx = getIndex(item);
    if(idx >= 0 && idx < getItemCount()){
      cb()->removeItem(idx);
    }
  }
  void ComboHandle::remove(int idx){
    cb()->removeItem(idx);
  }
  void ComboHandle::clear(){
    while(getItemCount()){
      remove(0);
    }
  }
  std::string ComboHandle::getItem(int idx) const{
    return cb()->itemText(idx).toLatin1().data();
  }
  int ComboHandle::getIndex(const std::string &item) const{
    return cb()->findText(item.c_str());
  }
  
  int ComboHandle::getSelectedIndex() const{
    return cb()->currentIndex();
  }
  std::string ComboHandle::getSelectedItem() const{
    return cb()->currentText().toLatin1().data();
  }
  
  int ComboHandle::getItemCount() const{
    return cb()->count();
  }
  
  void ComboHandle::setSelectedIndex(int idx){
    cb()->setCurrentIndex(idx);
  }
  void ComboHandle::setSelctedItem(const std::string &item){
    int idx = getIndex(item);
    if(idx >=0 && idx< getItemCount()){
      cb()->setCurrentIndex(idx);
    }
  }

  
}
