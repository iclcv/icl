/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/StateHandle.cpp                              **
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

#include <ICLQt/StateHandle.h>
#include <ICLQt/ThreadedUpdatableTextView.h>

namespace icl{

  
  void StateHandle::append(const std::string &text){
    this->text()->appendTextFromOtherThread(text);
  }
  
  void StateHandle::clear(){
    text()->clearTextFromOtherThread();
  }
  
  void StateHandle::setMaxLen(int maxLen){
    this->maxLen = maxLen;
    removeOldLines();
  }
  
  int StateHandle::getMaxLen() const{
    return maxLen;
  }
  
  void StateHandle::removeOldLines(){
    // todo!
  }
 
}                               

