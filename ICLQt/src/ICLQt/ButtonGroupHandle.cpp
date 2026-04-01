// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/ButtonGroupHandle.h>
#include <ICLUtils/Macros.h>
#include <QRadioButton>
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
      ICLASSERT_RETURN(index >= 0 && index < static_cast<int>(vec().size()));
      vec()[index]->setEnabled(true);
    }
    void ButtonGroupHandle::disable(int index){
      ICLASSERT_RETURN(index >= 0 && index < static_cast<int>(vec().size()));
      vec()[index]->setEnabled(false);
    }


  } // namespace qt
}
