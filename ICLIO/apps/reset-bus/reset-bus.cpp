// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>

int main(int n, char **ppc){
  pa_init(n,ppc,"-device-type|-t(type) -verbose");
  GenericGrabber::resetBus(pa("-t"), pa("-verbose"));
}
