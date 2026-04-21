// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/core/Img.h>

#include <mutex>
#include <vector>

namespace icl::io {
  /// Writer backend for ".png" images \ingroup FILEIO_G
  /** Registered as a function-plugin via `REGISTER_FILE_WRITER_PLUGIN`
      (see FileWriter.h). Instance state is held via a per-registration
      function-local static inside the registration lambda. */
  class FileWriterPluginPNG {
    std::recursive_mutex mutex;
    std::vector<unsigned char> data;
    std::vector<unsigned char*> rows;

    public:
    /// write implementation
    ICLIO_API void write(utils::File &file, const core::ImgBase *image);
  };
  } // namespace icl::io
