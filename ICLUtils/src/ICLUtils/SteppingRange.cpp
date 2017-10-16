/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/SteppingRange.cpp                **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLUtils/SteppingRange.h>
#include <ICLUtils/StringUtils.h>

namespace icl{
  namespace utils{

    template<class T>
    std::ostream &operator<<(std::ostream &s, const SteppingRange <T> &range){
      s << '[';
      icl_to_stream(s,range.minVal);
      s << ',';
      icl_to_stream(s,range.maxVal);
      s << "]:";
      icl_to_stream(s,range.stepping);
      return s;
    }

    template<class T>
    std::istream &operator>>(std::istream &s, SteppingRange <T> &range){
      char c;
      s >> c;
      icl_from_stream(s,range.minVal);
      s >> c;
      icl_from_stream(s,range.maxVal);
      s >> c;
      range.stepping = 0;
      if(s)
        s >> c;

      if(s)
        icl_from_stream(s,range.stepping);
      return s;
    }



  #define ICL_INSTANTIATE_DEPTH(D)                                        \
    template ICLUtils_API std::ostream &operator<<(std::ostream&,const SteppingRange<icl##D>&); \
    template ICLUtils_API std::istream &operator>>(std::istream&, SteppingRange<icl##D>&);
    ICL_INSTANTIATE_ALL_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH


  } // namespace utils
}
