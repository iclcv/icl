/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ToggleButton.cpp                       **
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

#include <ICLQt/ToggleButton.h>

namespace icl{
  namespace qt{
  
    ToggleButton::ToggleButton(const std::string &untoggledText, 
                               const std::string &toggledText,
                               QWidget *parent,
                               bool *stateRef):
      QPushButton(untoggledText.c_str(),parent),m_stateRef(stateRef){
      
      setCheckable(true);
      
      m_text[0] = untoggledText;
      m_text[1] = toggledText;
      
      connect(this,SIGNAL(toggled(bool)),this,SLOT(toggleStateChanged(bool)));
    }
    
    ToggleButton::~ToggleButton(){
      disconnect(this,SIGNAL(toggled(bool)),this,SLOT(toggleStateChanged(bool)));
    }
      
    void ToggleButton::toggleStateChanged(bool toggled){
      if(toggled) toggled = 1; // I'm not shure if a boolean is always 0x1
      setText(m_text[toggled].c_str());
      if(m_stateRef) *m_stateRef = toggled;
    }  
  } // namespace qt
}
