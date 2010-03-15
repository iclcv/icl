/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLUtils/examples/config-file-test.cpp                 **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/ConfigFile.h>

using namespace icl;

int main(int n, char **ppc){
  painit(n,ppc,"[m]-config|-c(filename)");

  ConfigFile f(*pa("-c"));
  std::cout << "config file content:" << std::endl;
  f.listContents();


  f.clear();
  f.setPrefix("config.");

  f["section-1.subsection-1.val1"] = 5;
  f["section-2.subsection-1.val1"] = 5.4;
  f["section-1.subsection-2.val1"] = 544.f;
  f["section-2.subsection-1.val2"] = 'c';
  f["section-3.subsection-1.val3"] = str("22test");
  
  f.listContents();
  
  std::cout << f << std::endl;
}
