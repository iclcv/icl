// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/core/Img.h>

namespace icl::io {
  /// Plugin to grab ".ppm", ".pgm", ".pnm" and ".icl" images \ingroup FILEIO_G
  class ICLIO_API FileGrabberPluginPNM {
    public:
    /// grab implementation
    void grab(utils::File &file, core::ImgBase **dest);
  };
  } // namespace icl::io
