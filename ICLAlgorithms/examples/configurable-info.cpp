/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/examples/configurable-info.cpp           **
** Module : ICLAlgorithms                                          **
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

#include <ICLUtils/Configurable.h>
#include <ICLUtils/ProgArg.h>

#include <ICLAlgorithms/LMA.h>
#include <ICLMarkers/FiducialDetector.h>

using namespace icl;

// include some static instances to ensure, that the linker
// does not purge the linked libraries because no symbols of these
// are used here
static LMA lma; // adds all the basic stuff
static FiducialDetector fid; // includes lots of additional stuff such as ICLFilter, ICLBlob and ICLMarkers

int main(int n, char **ppc){
  painit(n,ppc,"-list|-ls|-l "
         "-create-default-property-file|-o|-c(ConfigurableName,FileName) "
         "-show-properties|-s|-i(ConfigurableName)");
  
  std::vector<std::string> all = Configurable::get_registered_configurables();
  
  if(pa("-l")){
    std::cout << "--- Supported Configurables --- " << std::endl;
    for(unsigned int i=0;i<all.size();++i){
      std::cout << all[i] << std::endl;
    }
  }
  
  if(pa("-o")){
    std::string name = pa("-o",0);
    std::string filename = pa("-o",1);
    Configurable::create_configurable(name)->saveProperties(name);
  }
  
  if(pa("-i")){
    std::string name = pa("-i");
    Configurable *p = Configurable::create_configurable(name);
    
    std::cout << "--- Properties for Configurable " << name << " ---" << std::endl;
    std::vector<std::string> props = p->getPropertyList();
    for(unsigned int j=0;j<props.size();++j){
      std::string pj = props[j];
      std::cout << "   \"" << pj << "\"" << std::endl;
      std::cout << "      type        : " << p->getPropertyType(pj) << std::endl;
      std::cout << "      def. value  : " << p->getPropertyValue(pj) << std::endl; 
      std::cout << "      info        : " << p->getPropertyInfo(pj) << std::endl;
      std::cout << "      volatileness: " << p->getPropertyVolatileness(pj) << std::endl;
      std::cout << "      tooltip     : ";
      std::vector<std::string> lines = tok(p->getPropertyToolTip(pj), "\n");
      for(unsigned int k=0;k<lines.size();++k){
        std::cout << (k?"                    ":"") << lines[k] << std::endl;
      }
      std::cout << std::endl;
    }
  }
}
