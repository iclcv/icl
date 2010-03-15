/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/StringHandle.cpp                             **
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

#include <ICLQt/StringHandle.h>
#include <string>
#include <QLineEdit>


namespace icl{
  
  void StringHandle::operator=(const std::string &text){
    (**this)->setText(text.c_str());
    getGUIWidget()->getGUI()->lockData();
    *m_str = text;
    getGUIWidget()->getGUI()->unlockData();

  }
  std::string StringHandle::getCurrentText() const{
    return (**this)->text().toLatin1().data();  
  }
  std::string StringHandle::getValue() const{
    std::string s;
    const_cast<StringHandle*>(this)->getGUIWidget()->getGUI()->lockData();
    s = *m_str;
    const_cast<StringHandle*>(this)->getGUIWidget()->getGUI()->unlockData();
    return s;
  }
}


