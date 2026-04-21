// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/ProgArg.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/ConfigFile.h>

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
