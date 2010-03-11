/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLQt/CheckBoxHandle.h>
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


