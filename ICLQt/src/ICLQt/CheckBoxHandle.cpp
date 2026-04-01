// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/CheckBoxHandle.h>
#include <QCheckBox>


namespace icl{
  namespace qt{


    CheckBoxHandle::CheckBoxHandle():m_stateRef(0){}

    CheckBoxHandle::CheckBoxHandle(QCheckBox *cb, GUIWidget *w, bool *stateRef):
      GUIHandle<QCheckBox>(cb,w),m_stateRef(stateRef){
    }


    void CheckBoxHandle::check(bool execCallbacks){
      *m_stateRef = true;
      (***this).setCheckState(Qt::Checked);
      if(execCallbacks) cb();
    }

    void CheckBoxHandle::uncheck(bool execCallbacks){
      *m_stateRef = false;
      (***this).setCheckState(Qt::Unchecked);
      if(execCallbacks) cb();
    }

    bool CheckBoxHandle::isChecked() const{
      return *m_stateRef;
    }
  } // namespace qt
}
