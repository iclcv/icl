// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/core/Img.h>

#include <mutex>

namespace icl::io {
  /// Plugin for writing ".csv" files (Comma-Separated Values) \ingroup FILEIO_G
  /** image parameters can be found by three different means:
      -# <b>Encoded into the file name.</b>
      -# <b>As comment block</b>
      -# <b>Interpret a csv file as matrix data</b>
      */
  class ICLIO_API FileGrabberPluginCSV {
    public:
    /// Create a new Plugin
    FileGrabberPluginCSV();

    /// Destructor
    ~FileGrabberPluginCSV();

    /// grab implementation
    void grab(utils::File &file, core::ImgBase **dest);

    private:

    /// internally used reading buffer
    core::Img64f *m_poReadBuffer;

    /// internally used mutex to protect the reading buffer
    std::recursive_mutex *m_poReadBufferMutex;
  };
  } // namespace icl::io
