/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLUtils/src/Rect32f.cpp                               **
** Module : ICLUtils                                               **
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

#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Rect.h>



namespace icl{

  const Rect32f Rect32f::null(0,0,0,0);
  
  
  /// ostream operator (x,y)wxy
  std::ostream &operator<<(std::ostream &s, const Rect32f &r){
    return s << r.ul() << r.getSize();
  }
  
  /// istream operator
  std::istream &operator>>(std::istream &s, Rect32f &r){
    Point32f offs;
    Size32f size;
    s >> offs;
    s >> size;
    r = Rect32f(offs,size);
    return s;
  }

  
}
