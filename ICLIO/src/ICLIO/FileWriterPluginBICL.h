// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/FileWriterPlugin.h>
#include <ICLIO/ImageCompressor.h>
#include <mutex>

namespace icl{
  namespace io{

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
  } // namespace io
}
