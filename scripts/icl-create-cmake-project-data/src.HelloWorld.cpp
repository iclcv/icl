/********************************************************************
**                                                                 **
** Copyright (C) 2013 Viktor Richter                               **
**                                                                 **
** File   : src/HelloWorld.cpp                                     **
** Authors: Viktor Richter                                         **
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
********************************************************************/

#include <HelloWorld.h>
#include <iostream>

HelloWorld::HelloWorld(){
  std::cout << "HelloWorld constructor called." << std::endl;
}

void HelloWorld::print(std::string s){
  std::cout << "function 'print' called with '" << s << "'" << std::endl;
}
