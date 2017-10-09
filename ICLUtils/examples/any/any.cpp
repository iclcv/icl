/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/examples/any/any.cpp                          **
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
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Any.h>
#include <ICLUtils/Random.h>
#include <ICLUtils/Time.h>

using namespace icl::utils;

void bench(){
  std::vector<float> v(1000);
  std::fill(v.begin(),v.end(),GRand(0,1));

  Time t = Time::now();
  for(int i=0;i<1000;++i){
    Any a = v;
    std::vector<float> b = a;
  }
  t.showAge("any");


  t = Time::now();
  for(int i=0;i<1000;++i){
    std::string a = cat(v,",");
    std::vector<float> b = parseVecStr<float>(a);
  }
  t.showAge("cat / parse");
}

int main(int n, char **ppc){
  float data[] = { 1,2,3,4,5,6,7,8,9,10 };
  std::vector<float> v(data,data+10);

  Any a = v;

  std::vector<float> v2 = a;

  bench();

  std::cout << "[" << cat(v2,",") << "]" << std::endl;
}
