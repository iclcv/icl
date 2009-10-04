#include <iclCheckBoxHandle.h>
#include <QCheckBox>


namespace icl{
  

  CheckBoxHandle::CheckBoxHandle():m_stateRef(0){}
  
  CheckBoxHandle::CheckBoxHandle(QCheckBox *cb, GUIWidget *w, bool *stateRef):
    GUIHandle<QCheckBox>(cb,w),m_stateRef(stateRef){
  }
  
  
  void CheckBoxHandle::check(bool execCallbacks){
    (***this).setCheckState(Qt::Checked);
    if(execCallbacks) cb();
  }
  
  void CheckBoxHandle::uncheck(bool execCallbacks){
    (***this).setCheckState(Qt::Unchecked);
    if(execCallbacks) cb();
  }
  
  bool CheckBoxHandle::isChecked() const{
    return *m_stateRef;
  }
}


