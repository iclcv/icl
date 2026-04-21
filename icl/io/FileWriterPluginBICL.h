// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/io/FileWriterPlugin.h>
#include <icl/io/ImageCompressor.h>
#include <mutex>

namespace icl::io {
  /// Writer plugin to write binary icl image (extension bicl / bicl.gz)
  /** The bicl-core::format does also support saving image meta data */
  class ICLIO_API FileWriterPluginBICL : public FileWriterPlugin{
    public:

    FileWriterPluginBICL(const std::string &compressionType="none",
                         const std::string &quality="none");

    /// write implementation
    virtual void write(utils::File &file, const core::ImgBase *image);

    private:
    ImageCompressor compressor;
    std::recursive_mutex mutex;
  };
  } // namespace icl::io