// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Quick.h>

int main(int nargs, char **ppc){

  ImgQ image = scale(create("parrot"),0.5);

  show(image);

  return 0;
}
