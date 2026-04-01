// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

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
