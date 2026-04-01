// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/Size32f.h>


namespace icl{
  namespace utils{

    const Size32f Size32f::null(0,0);


    std::ostream &operator<<(std::ostream &s, const Size32f &size){
      return s << size.width << 'x' << size.height;
    }

    std::istream &operator>>(std::istream &s, Size32f &size){
      char c;
      return s >> size.width >> c >> size.height;
    }

  } // namespace utils
}
