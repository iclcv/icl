/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ComboHandle.cpp                        **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLQt/ComboHandle.h>
#include <QComboBox>

namespace icl{
  namespace qt{
  
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
  
    
  } // namespace qt
}
