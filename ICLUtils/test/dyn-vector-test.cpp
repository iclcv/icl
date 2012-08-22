/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/test/dyn-vector-test.cpp                      **
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

#include <ICLUtils/DynVector.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Macros.h>

using namespace icl;

typedef DynMatrix<float> Mat;
typedef DynColVector<float> CVec;
typedef DynRowVector<float> RVec;

int main(){
  Mat m(5,5);
  for(int x=0;x<5;++x){
    for(int y=0;y<5;++y){
      m(x,y) = x + 10*y;
    }
  }
  
  CVec v2 = m.col(2);
  RVec r2 = m.row(2);
  CVec v22 = r2.transp();
  RVec r22 = v2.transp();
  
  SHOW((v2,v2));

  SHOW(v2%v2);

  SHOW(v2%r2);
  
  //  SHOW(v2);
  // SHOW(r2);
  // SHOW(v22);
  // SHOW(r22);

}
