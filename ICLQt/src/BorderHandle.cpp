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

#include <ICLQt/BorderHandle.h>

#include <QGroupBox>

namespace icl{
  std::string BorderHandle::getTitle() const{
    const BorderHandle &bh = *this;//const_cast<BorderHandle*>(this);
    const QGroupBox *gb = *bh;
    if(gb){
      return gb->title().toLatin1().data();
    }
    return "undefined!";
  }
  void BorderHandle::operator=(const std::string &title){
    if(**this) (**this)->setTitle(title.c_str());
    (**this)->update();
  }
  
  
}
