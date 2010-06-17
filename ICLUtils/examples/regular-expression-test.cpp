/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/examples/regular-expression-test.cpp          **
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
#include <ICLUtils/Macros.h>

using namespace icl;

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
