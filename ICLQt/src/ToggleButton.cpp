/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/ToggleButton.cpp                             **
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

#include <ICLQt/ToggleButton.h>

namespace icl{

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
}
