// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/PlotHandle.h>

namespace icl{
  namespace qt{
    void PlotHandle::render(){
      (**this)->updateFromOtherThread();
    }

    /// todo: more convecience methods
  } // namespace qt
}
