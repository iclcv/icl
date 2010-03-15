/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/LabelHandle.cpp                              **
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

#include <ICLQt/LabelHandle.h>
#include <QPainter>
#include <QLabel>

namespace icl{
  void LabelHandle::operator=(const std::string &text){
    lab()->setText(text.c_str());
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator=(const QString &text){
    lab()->setText(text);
    lab()->updateFromOtherThread();
  } 
  void LabelHandle::operator=(const char *text){
    lab()->setText(QString(text));
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator=(int num){
    lab()->setNum(num);
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator=(double num){
    lab()->setNum(float(num));
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator+=(const std::string &text){
    lab()->setText(lab()->text() + text.c_str());
    lab()->updateFromOtherThread();
  }
}
