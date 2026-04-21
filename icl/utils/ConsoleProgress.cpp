// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/ConsoleProgress.h>
#include <cstdio>
using namespace icl;

namespace{

  char BACK_LINE[] = { char(27), char(91), char(65), char(0)};// "\E[A\0"

  int BAR_LEN = 30;

  std::string PROGRESS_STR;
}
namespace icl::utils {
  void progress_init(const std::string &text){

    printf("\n");
    printf("%s%s\n",BACK_LINE,text.c_str());
    PROGRESS_STR = text;
  }


  void progress_finish(){

    printf("%s%s:100%s[##############################]\n",BACK_LINE,PROGRESS_STR.c_str(),"%");

  }


  void progress(int curr, int max, const std::string &t){

    printf("%s%s:",BACK_LINE,PROGRESS_STR.c_str());
    float frac = static_cast<float>(curr)/static_cast<float>(max);
    printf("%3d%s",static_cast<int>(frac*100),"%");

    int N1 = static_cast<int>(frac*BAR_LEN);
    int N2 = BAR_LEN - N1;

    printf("[");
    for(int i=0;i<N1;i++){
      printf("#");
    }
    for(int i=0;i<N2;i++){
      printf("=");
    }
    if(t.length()){
      printf("] %s\n",t.c_str());
    }else{
      printf("]\n");
    }

  }
  } // namespace icl::utils