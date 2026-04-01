// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLIO/DCDevice.h>
#include <ICLUtils/ProgArg.h>

int main(int n, char **ppc){
  icl::utils::pa_init(n,ppc,"-quiet|-q");
  icl::io::DCDevice::dc1394_reset_bus(!icl::utils::pa("-q"));
}
