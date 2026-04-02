// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/PlotHandle.h>

namespace icl::qt {
    void PlotHandle::render(){
      (**this)->updateFromOtherThread();
    }

    /// todo: more convecience methods
  } // namespace icl::qt