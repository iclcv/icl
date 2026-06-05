// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/SaveLoad.h>
#include <icl/io/file/FileGrabber.h>
#include <icl/io/file/FileWriter.h>

namespace icl::io {

  void save(const core::Image &image, const std::string &filename) {
    if(image.isNull()) return;
    FileWriter(filename).write(image.ptr());
  }

  core::Image load(const std::string &filename) {
    try {
      return FileGrabber(filename).grab();
    } catch(const utils::ICLException &ex) {
      ERROR_LOG("exception: " << ex.what());
      return core::Image();
    }
  }

} // namespace icl::io
