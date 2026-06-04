// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/QuickIO.h>

using namespace icl::core;

namespace icl::qt {

  void print(const Image &image) {
    image.print("image");
  }

} // namespace icl::qt
