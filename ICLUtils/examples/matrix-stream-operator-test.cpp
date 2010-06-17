/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/examples/matrix-stream-operator-test.cpp      **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLUtils/FixedMatrix.h>
#include <iostream>
#include <sstream>

typedef unsigned char real;
int main(){
  icl::FixedMatrix<real,2,3> m( 2, 4,
                            6, 7,
                            0, -1);
  
  std:: cout << m << std::endl;
  
  std::ostringstream os;
  os << m << m << m;
  
  std::istringstream is(os.str());
  icl::FixedMatrix<real,2,3> ms[3]={
    icl::FixedMatrix<real,2,3>(0.0),
    icl::FixedMatrix<real,2,3>(1.0),
    icl::FixedMatrix<real,2,3>(2.0)
  };
  is >> ms[0];
  is >> ms[1];
  is >> ms[2];
  
  std::cout << "ms[0]:\n" << ms[0] << std::endl;
  std::cout << "ms[1]:\n" << ms[1] << std::endl;
  std::cout << "ms[2]:\n" << ms[2] << std::endl;
  
}
