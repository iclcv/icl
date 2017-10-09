/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Rect32f.cpp                      **
** Module : ICLUtils                                               **
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

#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Rect.h>



namespace icl{
  namespace utils{

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


  } // namespace utils
}
