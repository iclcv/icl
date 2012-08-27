/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/TwoLevelRegionStructure.cpp             **
** Module : ICLMarkers                                             **
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

#include <ICLMarkers/TwoLevelRegionStructure.h>
#include <ICLCV/ImageRegion.h>

namespace icl{
  namespace markers{
    TwoLevelRegionStructure::TwoLevelRegionStructure(const std::string &code) throw (ICLException) : color(0), code(code){
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
