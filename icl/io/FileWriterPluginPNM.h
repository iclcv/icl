// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/core/Img.h>

#include <mutex>
#include <vector>

namespace icl::io {
  /// Writer backend for ".ppm", ".pgm", ".pnm" and ".icl" \ingroup FILEIO_G
  class ICLIO_API FileWriterPluginPNM {
    public:
    /// write implementation
    void write(utils::File &file, const core::ImgBase *image);

    private:
    /// internal mutex to protect the buffer
    std::recursive_mutex m_oBufferMutex;

    /// internal data conversion buffer
    std::vector<icl8u> m_vecBuffer;
  };
  } // namespace icl::io
