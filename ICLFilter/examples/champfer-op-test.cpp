/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/examples/champfer-op-test.cpp                **
** Module : ICLFilter                                              **
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

#include <ICLQuick/Quick.h>
#include <ICLFilter/ChamferOp.h>
#include <ICLUtils/StackTimer.h>

void test_func_8(const ImgBase *src, ImgBase **dst){
  BENCHMARK_THIS_FUNCTION;
  static ChamferOp co(3,4,8,false);
  co.apply(src,dst);
}

void test_func_2(const ImgBase *src, ImgBase **dst){
  BENCHMARK_THIS_FUNCTION;
  static ChamferOp co(3,4,2,false);
  co.apply(src,dst);
}
void test_func_4(const ImgBase *src, ImgBase **dst){
  BENCHMARK_THIS_FUNCTION;
  static ChamferOp co(3,4,4,false);
  co.apply(src,dst);
}
void test_func_1(const ImgBase *src, ImgBase **dst){
  BENCHMARK_THIS_FUNCTION;
  static ChamferOp co(3,4,1,false);
  co.apply(src,dst);
}


int main(int n, char **ppc){
  
  ImgQ image = create("parrot");
  image = gray(image);
  image = scale(image,640,480);
  image = filter(image,"laplace");
  image = thresh(image,250);
  //image.setROI(Rect(100,100,320,240));
  
  ImgQ resAccu;
  ImgBase *res=0;
  Img8u im = cvt8u(image);
  
  for(int i=0;i<50;i++){
    test_func_1(&im,&res);
    test_func_2(&im,&res);
    test_func_4(&im,&res);
    test_func_8(&im,&res);
  }
  printf("------- \n");
  ImgQ r = cvt(res);
  r = norm(r);
  // roi(image) = roi(r);
  //image.setFullROI();
  show(r);


  return 0;
}
