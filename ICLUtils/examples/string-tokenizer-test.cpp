/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/examples/string-tokenizer-test.cpp            **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLUtils/StringUtils.h>
#include <ICLUtils/ProgArg.h>

using namespace icl;

int main(int n, char **ppc){
  painit(n,ppc,"[m]-text|-t(text) [m]-delim|-d(delim) -escape-char|-e(char) -single-char-delim|-s");
  
  std::vector<std::string> ts = tok(pa("-t"),pa("-d"), pa("-s"), pa("-e") ? (*pa("-e"))[0] : '\0');
  
  std::cout << "tokens:" << std::endl;
  for(size_t i=0;i<ts.size();++i){
    std::cout << "--" << ts[i] << "--" << std::endl;
  }
  
  std::cout << std::endl;
}
