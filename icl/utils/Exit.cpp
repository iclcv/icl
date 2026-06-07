// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/Exit.h>
#include <cstdlib>

namespace icl::utils {
  namespace { std::function<void(int)> g_exitHandler; }

  void setExitHandler(std::function<void(int)> handler){
    g_exitHandler = std::move(handler);
  }

  void exit(int code){
    if(g_exitHandler) g_exitHandler(code);  // expected not to return
    std::exit(code);                         // default / handler fell through
  }
} // namespace icl::utils
