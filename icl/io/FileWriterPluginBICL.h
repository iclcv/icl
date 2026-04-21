// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/core/Img.h>

#include <memory>
#include <mutex>
#include <string>

namespace icl::io {
  class ImageCompressor;  // forward decl

  /// Writer backend for binary icl image (extension bicl / bicl.gz) \ingroup FILEIO_G
  /** The bicl-format supports saving image meta data. The wrapped
      `ImageCompressor` is built lazily on first write so that this
      plugin can be constructed at static-init time without touching
      the (still-empty) CompressionRegister. */
  class ICLIO_API FileWriterPluginBICL {
    public:

    FileWriterPluginBICL(const std::string &compressionType="raw",
                         const std::string &quality="");
    ~FileWriterPluginBICL();

    /// write implementation
    void write(utils::File &file, const core::ImgBase *image);

    private:
    std::unique_ptr<ImageCompressor> compressor;  // lazy
    std::string compressionType;
    std::string compressionQuality;
    std::recursive_mutex mutex;
  };
  } // namespace icl::io
