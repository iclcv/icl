/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ButtonGroupHandle.cpp                  **
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

#include <ICLQt/ButtonGroupHandle.h>
#include <ICLUtils/Macros.h>
#include <QRadioButton>

using namespace std;
namespace icl{
  namespace qt{
    void ButtonGroupHandle::select(int id){
      ICLASSERT_RETURN(valid(id));
      vec()[id]->setChecked(true);
    }
    int ButtonGroupHandle::getSelected() const{
      ICLASSERT_RETURN_VAL(n(),-1);
      for(int i=0;i<n();i++){
        if(vec()[i]->isChecked()){
          return i;
        }
      }
      return -1;
    }
    std::string ButtonGroupHandle::getSelectedText() const{
      ICLASSERT_RETURN_VAL(n(),"");
      int selectedIndex = getSelected();
      ICLASSERT_RETURN_VAL(selectedIndex,"");
      return vec()[selectedIndex]->text().toLatin1().data();
    }
    std::string ButtonGroupHandle::getText(int id) const{
      ICLASSERT_RETURN_VAL(valid(id),"");
      return vec()[id]->text().toLatin1().data();
    }
    void ButtonGroupHandle::setText(int id, const std::string &t) {
      ICLASSERT_RETURN(valid(id));
      vec()[id]->setText(t.c_str());

    }

    void ButtonGroupHandle::disable(){
      for(int i=0;i<n();i++){
        vec()[i]->setEnabled(false);
      }
    }

    void ButtonGroupHandle::enable(){
      for(int i=0;i<n();i++){
        vec()[i]->setEnabled(true);
      }
    }

    void ButtonGroupHandle::enable(int index){
      ICLASSERT_RETURN(index >= 0 && index < (int)vec().size());
      vec()[index]->setEnabled(true);
    }
    void ButtonGroupHandle::disable(int index){
      ICLASSERT_RETURN(index >= 0 && index < (int)vec().size());
      vec()[index]->setEnabled(false);
    }


  } // namespace qt
}
