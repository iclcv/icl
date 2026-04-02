// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/FileWriterPlugin.h>
#include <mutex>

namespace icl::io {
    /// A Writer Plugin for writing ".png" images \ingroup FILEIO_G
    class FileWriterPluginPNG : public FileWriterPlugin{
      std::recursive_mutex mutex;
      std::vector<unsigned char> data;
      std::vector<unsigned char*> rows;

      public:
      /// write implementation
      ICLIO_API virtual void write(utils::File &file, const core::ImgBase *image);
    };
  } // namespace icl::io