// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/DCGrabber.h>
using namespace icl::io;


int main(int n, char **ppc){
  std::vector<DCDevice> devs = DCGrabber::getDCDeviceList();
  printf("found %d cameras \n",(unsigned int)devs.size());


  for(unsigned int i=0;i<devs.size();i++){
    DCDevice &d = devs[i];
    char acBuf[100];
    sprintf(acBuf,"%2d",i);
    d.show(acBuf);
  }

  return 0;
}
