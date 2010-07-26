/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/CheckBoxHandle.cpp                           **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQt/CheckBoxHandle.h>
#include <QtGui/QCheckBox>


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


