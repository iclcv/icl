/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ButtonHandle.cpp                       **
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

#include <ICLQt/ButtonHandle.h>
#include <QPushButton>
#include <algorithm>

namespace icl{
  namespace qt{
    ButtonHandle::ButtonHandle(){}
    
    ButtonHandle::ButtonHandle(QPushButton *b, GUIWidget *w):
      GUIHandle<QPushButton>(b,w),m_triggered(new bool(false)){
    }
  
    bool ButtonHandle::wasTriggered(bool reset) {
      if(*m_triggered){
        if(reset) *m_triggered = false;
        return true;
      }
      return false;
    }
  
    void ButtonHandle::reset() { 
      *m_triggered = false; 
    }
    const std::string &ButtonHandle::getID() const { 
      return m_sID; 
    }
  
  
  } // namespace qt
}
