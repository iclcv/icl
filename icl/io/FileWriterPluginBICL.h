// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/io/FileWriterPlugin.h>
#include <memory>
#include <mutex>
#include <string>

namespace icl::io {
  class ImageCompressor;  // forward decl: avoid pulling ImageCompressor.h
                          // into static-init paths (FileWriter's plugin
                          // map constructs us at static init time)

  /// Writer plugin to write binary icl image (extension bicl / bicl.gz)
  /** The bicl-core::format does also support saving image meta data.
      The wrapped `ImageCompressor` is built lazily on first write so
      that this plugin can itself be constructed at static-init time
      (FileWriter's `s_mapPlugins[".bicl"] = new FileWriterPluginBICL`)
      without touching the (still-empty) CompressionRegister. */
  class ICLIO_API FileWriterPluginBICL : public FileWriterPlugin{
    public:

    FileWriterPluginBICL(const std::string &compressionType="raw",
                         const std::string &quality="");
    ~FileWriterPluginBICL();

    /// write implementation
    virtual void write(utils::File &file, const core::ImgBase *image);

    private:
    std::unique_ptr<ImageCompressor> compressor;  // lazy
    std::string compressionType;
    std::string compressionQuality;
    std::recursive_mutex mutex;
  };
  } // namespace icl::io