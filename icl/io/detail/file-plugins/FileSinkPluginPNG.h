// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/Img.h>

#include <mutex>
#include <vector>

namespace icl::io {
  /// Writer backend for ".png" images \ingroup FILEIO_G
  /** Singleton plugin (function-local static, accessible via instance()).
      Inherits Configurable; the singleton is registered with FileWriter as
      a child under the "png" prefix, so callers can do
      `writer.setPropertyValue("png.compression-level", 9)`. */
  class ICLIO_API FileSinkPluginPNG : public utils::Configurable {
    public:
    FileSinkPluginPNG();

    /// process-wide singleton accessor
    static FileSinkPluginPNG &instance();

    /// write implementation
    void write(utils::File &file, const core::ImgBase *image);

    private:
    int m_compressionLevel = 4;          //!< zlib level 0-9, set via property "compression-level"
    std::recursive_mutex mutex;
    std::vector<unsigned char> data;
    std::vector<unsigned char*> rows;
  };
  } // namespace icl::io
