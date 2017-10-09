/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/examples/progarg/progarg.cpp                  **
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

#include <ICLUtils/ProgArg.h>
#include <ICLUtils/Size.h>
#include <string>
#include <cstdio>

using namespace icl::utils;

int main(int n, char **ppc){

  pa_explain("-s","defines image size")
            ("-f","defines image foramt (RGB,Gray or HLS)")
            ("-c","defines channel count to use")
            ("-fast","enables a 'fast'-mode");

  pa_init(n,ppc,"-size|-s(Size=VGA) -format|-f(format=RGB) -channels|-c(int) -fast",true);

  // program name
  SHOW(pa_get_progname());

  // number of all arguments
  SHOW(pa_get_count(false));

  SHOW(!!pa("-c"));

  SHOW(!!pa("-f"));
  // number of dangling arguments

  Size s = pa("-s");
  std::cout << "argument '-size' was " << (pa("-s")?"":"not ")
            << "given: " << s << std::endl;

  if(pa("-format")){
    SHOW(pa("-format"));
  }
  if(pa("-fast")){
    std::cout << "argument -fast was given:" << std::endl;
  }
  SHOW(pa("-fast"));


  for(unsigned int i=0;i<pa_get_count();i++){
    std::cout << "dangling argument " << i << " is:'"
              << pa(i) << "'" << std::endl;
  }
}
