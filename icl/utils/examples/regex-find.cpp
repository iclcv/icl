// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/StringUtils.h>
#include <icl/utils/Macros.h>

using namespace icl::utils;

int main(int n, char **ppc){
  if(n<3){
    ERROR_LOG(str("usage ") + str(*ppc) + " [regex] [TEXT]");
  }
  std::ostringstream os;
  std::copy(ppc+2,ppc+n,std::ostream_iterator<std::string>(os," "));

  MatchResult r = match(os.str(),ppc[1],10);
  if(r){
    std::cout << "** MATCH **" << std::endl << std::endl;

    std::cout << "** here are up to 10 submatches" << std::endl;
    for(unsigned int i=0;i<r.submatches.size();++i){
      std::cout << "["<< i <<"]: #" << r.submatches[i] << "#" << std::endl;
    }
  }else{
    std::cout << "** __NO__ MATCH **" << std::endl << std::endl;
  }

}
