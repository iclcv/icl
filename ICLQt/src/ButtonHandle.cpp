/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/ButtonHandle.cpp                             **
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

#include <ICLQt/ButtonHandle.h>
#include <algorithm>

namespace icl{
  ButtonHandle::ButtonHandle(){}
  
  ButtonHandle::ButtonHandle(QPushButton *b, GUIWidget *w):
    GUIHandle<QPushButton>(b,w),m_bTriggered(false){
  }
  /*
  void ButtonHandle::trigger() { 
    m_bTriggered = true; 
    for(unsigned int i=0;i<m_vecCallbackFuncs.size();i++){
      m_vecCallbackFuncs[i]();
    }
    for(unsigned int i=0;i<m_vecCallbackObjs.size();i++){
      (*m_vecCallbackObjs[i])();
    }
  }
  */
  bool ButtonHandle::wasTriggered(bool reset) {
    if(m_bTriggered){
      if(reset) m_bTriggered = false;
      return true;
    }
    return false;
  }

  void ButtonHandle::reset() { 
    m_bTriggered = false; 
  }
  const std::string &ButtonHandle::getID() const { 
    return m_sID; 
  }
  /*
  void  ButtonHandle::registerCallback(ButtonHandle::callback c, bool remove){
    if(remove){
		std::vector<callback>::iterator it =  find(m_vecCallbackFuncs.begin(),m_vecCallbackFuncs.end(),c);
      if(it !=  m_vecCallbackFuncs.end()){
        m_vecCallbackFuncs.erase(it);
      }
    }else{
      m_vecCallbackFuncs.push_back(c);
    }
    
  }
  

  void  ButtonHandle::registerCallback(ButtonHandle::Callback *c, bool remove){
    if(remove){
      std::vector<Callback*>::iterator it =  find(m_vecCallbackObjs.begin(),m_vecCallbackObjs.end(),c);
      if(it !=  m_vecCallbackObjs.end()){
        m_vecCallbackObjs.erase(it);
      }
    }else{
      m_vecCallbackObjs.push_back(c);
    }
  
  }
  */

}
