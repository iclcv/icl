// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/Range.h>
#include <icl/utils/StringUtils.h>
namespace icl::utils {
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
  template ICLUtils_API std::ostream &operator<<(std::ostream&, const Range<icl##D>&); \
  template ICLUtils_API std::istream &operator>>(std::istream&, Range<icl##D>&);
    ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH

  } // namespace icl::utils