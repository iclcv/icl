// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/Img.h>

namespace icl::io {
  /// Writer backend for ".csv" (Comma-Separated Values) \ingroup FILEIO_G
  /** Singleton plugin.  Inherits Configurable; registered with FileWriter
      under the "csv" prefix, so callers do
      `writer.setPropertyValue("csv.extend-file-name", true)`. */
  class ICLIO_API FileSinkPluginCSV : public utils::Configurable {
    public:
    FileSinkPluginCSV();

    /// process-wide singleton accessor
    static FileSinkPluginCSV &instance();

    /// write implementation
    void write(utils::File &file, const core::ImgBase *image);

    private:
    /// when true, encode image params by extending the file name
    /// (paired with FileSourcePluginCSV's decoder)
    bool m_extendFileName = false;
  };
} // namespace icl::io
