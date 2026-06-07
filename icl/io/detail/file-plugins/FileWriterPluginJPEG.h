// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/Types.h>

namespace icl::io {
  /// Writer backend for ".jpeg" and ".jpg" images \ingroup FILEIO_G
  /** Singleton plugin (function-local static, accessible via instance()).
      Inherits Configurable; the singleton is registered with FileWriter as
      a child under the "jpeg" prefix, so callers can do
      `writer.setPropertyValue("jpeg.quality", 85)`.

      The actual JPEG compression is delegated to JPEGEncoder — this class
      is just the FileWriter-facing facade + the "quality" tunable. */
  class ICLIO_API FileWriterPluginJPEG : public utils::Configurable {
    public:
    FileWriterPluginJPEG();

    /// process-wide singleton accessor
    static FileWriterPluginJPEG &instance();

    /// write implementation
    void write(utils::File &file, const core::ImgBase *image);

    private:
    int m_quality = 90;   //!< JPEG quality, 0-100, set via property "quality"
  };
} // namespace icl::io
