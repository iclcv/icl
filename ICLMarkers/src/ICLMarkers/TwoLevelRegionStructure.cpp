// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLMarkers/TwoLevelRegionStructure.h>
#include <ICLCV/ImageRegion.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::cv;

namespace icl{
  namespace markers{
    TwoLevelRegionStructure::TwoLevelRegionStructure(const std::string &code) : color(0), code(code){
      if(code.size() < 3) throw ICLException("TwoLevelRegionStructure(code): error parsing code " + code + "(code too short)");
      int i=0;
      if(code[i] == 'b' || code[i] == 'w'){
        color = 255*(code[i]=='w');
        ++i;
      }
      if(code[i] == '0') ++i; // root region
      const int n = code.length();

      for(;i<n;++i){
        if(code[i] == '1'){
          children.push_back(0);
        }else if(code[i] == '2'){
          ++children.back();
        }else{
          throw ICLException("TwoLevelRegionStructure(code): error parsing code " + code + "(allowed tokes are 1 and 2)");
        }
      }
      /// depends on the code (usually it should already be sorted)
      std::sort(children.begin(),children.end());
    }

    bool TwoLevelRegionStructure::match(const ImageRegion &r) const{
      if(r.getVal() != color) return false;
      const std::vector<ImageRegion> &rs = r.getSubRegions();
      unsigned int n = rs.size();
      if(n != children.size()) return false;
      std::vector<int> counts(n);
      for(unsigned int i=0;i<n;++i){
        counts[i] = rs[i].getSubRegions().size();
      }
      std::sort(counts.begin(),counts.end());
      for(unsigned int i=0;i<n;++i){
        if(children[i] != counts[i]) return false;
      }
      return true;
    }




  } // namespace markers
}
