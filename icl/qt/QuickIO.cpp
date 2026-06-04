// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/QuickIO.h>
#include <icl/io/TestImages.h>

using namespace icl::core;
using namespace icl::io;

namespace icl::qt {

  void show(const Image &image) {
    if(image.isNull()) return;
    // TODO: use context-local show command settings
    TestImages::show(image.ptr());
  }

  void print(const Image &image) {
    image.print("image");
  }

} // namespace icl::qt
