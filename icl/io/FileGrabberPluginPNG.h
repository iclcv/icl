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
  /// Plugin to read ".png" images \ingroup FILEIO_G
  class ICLIO_API FileGrabberPluginPNG {
    std::vector<unsigned char> data;
    std::vector<unsigned char*> rows;
    std::recursive_mutex mutex;

    public:
    /// grab implementation
    void grab(utils::File &file, core::ImgBase **dest);
  };
  } // namespace icl::io
