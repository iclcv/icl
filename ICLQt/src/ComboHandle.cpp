/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/ComboHandle.cpp                              **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLQt/ComboHandle.h>
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
