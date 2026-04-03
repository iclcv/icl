// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Types.h>
#include <ICLIO/FileWriterPlugin.h>
#include <vector>
#include <mutex>

namespace icl::io {
  /// Writer plugin to write images as ".ppm", ".pgm", ".pnm" and ".icl" \ingroup FILEIO_G
  class ICLIO_API FileWriterPluginPNM : public FileWriterPlugin{
    public:
    /// write implementation
    virtual void write(utils::File &file, const core::ImgBase *image);

    private:
    /// internal mutex to protect the buffer
    std::recursive_mutex m_oBufferMutex;

    /// internal data conversion buffer
    std::vector<icl8u> m_vecBuffer;
  };
  } // namespace icl::io