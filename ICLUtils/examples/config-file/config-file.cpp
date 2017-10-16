/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/examples/config-file/config-file.cpp          **
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
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/ConfigFile.h>

using namespace icl::utils;

int main(int n, char **ppc){
  pa_init(n,ppc,"-config|-c(filename)");

  if(pa("-c")){
    ConfigFile f(*pa("-c"));
    std::cout << "parsed and serialized file:" << std::endl;
    std::cout << f << std::endl;
    std::cout << "----------------------------------"<<
    std::endl <<"config file content:" << std::endl;
    f.listContents();
  }else{
    ConfigFile f;
    f.setPrefix("config.");

    f["section-1.subsection-1.val1"] = 5;
    f["section-2.subsection-1.val1"] = 5.4;
    f["section-1.subsection-2.val1"] = 544.f;
    f["section-2.subsection-1.val2"] = 'c';
    f["section-3.subsection-1.val3"] = str("22test");

    std::cout << "parsed and serialized file:" << std::endl;
    std::cout << f << std::endl;
    std::cout << "----------------------------------"<<
    std::endl <<"config file content:" << std::endl;
    f.listContents();
  }
  std::cout << std::endl;
}
