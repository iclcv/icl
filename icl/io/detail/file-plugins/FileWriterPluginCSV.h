// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/core/Img.h>

namespace icl::io {
  /// Writer backend for ".csv" (Comma-Separated Values) \ingroup FILEIO_G
  class ICLIO_API FileWriterPluginCSV {
    public:

    /// write implementation
    void write(utils::File &file, const core::ImgBase *image);

    /// static feature adaption function
    /** if the flag is set to true, the writer will encode image
        properties by extending the given filename
        @see FileGrabberPluginCSV
    **/
    static void setExtendFileName(bool value);

    private:
    /// static flag
    static bool s_bExtendFileName;
  };
  } // namespace icl::io
