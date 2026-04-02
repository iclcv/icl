// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/FileGrabberPlugin.h>
#include <vector>
#include <mutex>

namespace icl::io {
    /// Plugin class to read "png" images \ingroup FILEIO_G
    class ICLIO_API FileGrabberPluginPNG : public FileGrabberPlugin {
      std::vector<unsigned char> data;
      std::vector<unsigned char*> rows;

      /// ensures, that data and rows is not used from several threads
      std::recursive_mutex mutex;

      public:
      /// grab implementation
      virtual void grab(utils::File &file, core::ImgBase **dest);
    };
  } // namespace icl::io