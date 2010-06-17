/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/Range.cpp                                 **
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

#include <ICLUtils/Range.h>
#include <ICLUtils/StringUtils.h>
namespace icl{

  template<class T> 
  std::ostream &operator<<(std::ostream &s, const Range <T> &range){
    s << '[';
    icl_to_stream(s,range.minVal);
    s << ',';
    icl_to_stream(s,range.maxVal);
    return s << ']';
  }

  template<class T> 
  std::istream &operator>>(std::istream &s, Range <T> &range){
    char c;
    s >> c;
    icl_from_stream(s,range.minVal);
    s >> c;
    icl_from_stream(s,range.maxVal);
    return s >> c;
  }

  
  
#define ICL_INSTANTIATE_DEPTH(D)                                        \
  template std::ostream &operator<<(std::ostream&,const Range<icl##D>&); \
  template std::istream &operator>>(std::istream&,Range<icl##D>&);
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
  
}
