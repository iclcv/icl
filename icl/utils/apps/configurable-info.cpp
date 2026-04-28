// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/config/Configurable.h>
#include <icl/utils/ProgArg.h>

#include <icl/markers/FiducialDetector.h>


using namespace icl::markers;
using namespace icl::utils;

// include some static instances to ensure, that the linker
// does not purge the linked libraries because no symbols of these
// are used here
static FiducialDetector fid; // includes lots of additional stuff such as ICLFilter, ICLCV and ICLMarkers

int main(int n, char **ppc){
  pa_init(n,ppc,"-list|-ls|-l "
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
    std::string name = pa("-o",0).as<std::string>();
    std::string filename = pa("-o",1).as<std::string>();
    Configurable::create_configurable(name)->saveProperties(name);
  }

  if(pa("-i")){
    std::string name = pa("-i").as<std::string>();
    Configurable *p = Configurable::create_configurable(name);

    std::cout << "--- Properties for Configurable " << name << " ---" << std::endl;
    std::vector<std::string> props = p->getPropertyList();
    for(unsigned int j=0;j<props.size();++j){
      std::string pj = props[j];
      auto h = p->prop(pj);
      std::cout << "   \"" << pj << "\"" << std::endl;
      std::cout << "      type        : " << h.type() << std::endl;
      // Some kinds (command / monostate) aren't meaningfully stringifiable
      // under typed storage — catch and print empty rather than aborting
      // the whole listing on the first such property.
      std::cout << "      def. value  : ";
      try { std::cout << h.value.str(); } catch(...) {}
      std::cout << std::endl;
      std::cout << "      info        : " << h.info() << std::endl;
      std::cout << "      tooltip     : ";
      std::vector<std::string> lines = tok(h.tooltip, "\n");
      for(unsigned int k=0;k<lines.size();++k){
        std::cout << (k?"                    ":"") << lines[k] << std::endl;
      }
      std::cout << std::endl;
    }
  }
}
