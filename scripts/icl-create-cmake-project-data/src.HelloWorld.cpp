// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#include <HelloWorld.h>
#include <iostream>

HelloWorld::HelloWorld(){
  std::cout << "HelloWorld constructor called." << std::endl;
}

void HelloWorld::print(std::string s){
  std::cout << "function 'print' called with '" << s << "'" << std::endl;
}
